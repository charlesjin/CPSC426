/*****************************/
/*													 */
/*          Server           */
/*													 */
/*****************************/

#include <unistd.h>
#include <stdlib.h>
#include <string>
#include <ctime>

#include <QVariant>
#include <QDataStream>
#include <QTimer>
#include <QtCore>
#include <QFuture>
#include <QCoreApplication>

#include "netsocket.hh"
#include "shamir.hh"

/*****************************/
/*													 */
/* Netsocket Initializations */
/*													 */
/*****************************/

NetSocket::NetSocket()
{
  // Pick a range of four UDP ports to try to allocate by default,
  // computed based on Unix user ID.
  myPortMin = 32768 + (getuid() % 4096)*4;
  myPortMax = myPortMin + 3;
}

bool NetSocket::initialize()
{
  // Try to bind to each of the range myPortMin..myPortMax in turn.
  for (int p = myPortMin; p <= myPortMax; p++) {
    if (QUdpSocket::bind(p)) {
      myPort = p;

      // seed qrand, initialize members
      qsrand(time(0));
      originID = "charles-" + QString::number((time(0) + qrand() + p) % 524288);
      seqNo = 1;

      qDebug() << "bound to UDP port " << p << " with originID " << originID;

      peerManager = new PeerManager(originID, p);
      peerManager->setParent(this);
      connect(peerManager, SIGNAL(newPeerReady(Peer *)),
          this, SLOT(sendResponse(Peer *)));
      connect(peerManager, SIGNAL(newPeerReady(Peer *)),
          this, SLOT(sendRouteRumorToPeer(Peer *)));

      fileManager = new FileManager();
      fileManager->setParent(this);
      connect(fileManager, SIGNAL(sendBlockRefreshRequest(QMap<QString, QVariant>)),
          this, SLOT(sendDirectMessage(QMap<QString, QVariant>)));
      connect(fileManager, SIGNAL(sendSearchRefreshRequest(QMap<QString, QVariant>)),
          this, SLOT(searchRequestSender(QMap<QString, QVariant>)));

      // start antientropy stuff
      QTimer *aeTimer = new QTimer(this);
      connect(aeTimer, SIGNAL(timeout()), this, SLOT(antiEntropy()));
      aeTimer->start(1500);

      // start route rumors
      QTimer *rTimer = new QTimer(this);
      connect(rTimer, SIGNAL(timeout()), this, SLOT(sendRouteRumor()));
      rTimer->start(60000);

      // parse the command-line args
      QStringList arguments = QCoreApplication::arguments();
      QMap<QString, QVariant> map;

      int i = 1;
      if (arguments.size() > 1 && arguments.at(i) == "-noforward"){
        noForward = true;
        i++;
      } else
        noForward = false;

      for ( ; i < arguments.size(); ++i){
        peerManager->newPeer(arguments.at(i));
      }

      // say hello
      QTimer::singleShot(1000, this, SLOT(sendRouteRumor()));

      return true;
    }
  }

  qDebug() << "Oops, no ports in my default range " << myPortMin
    << "-" << myPortMax << " available";
  return false;
}

/*****************************/
/*													 */
/*      Message Senders      */
/*													 */
/*****************************/

// sends a chat message that you created
void NetSocket::messageSender(QMap<QString, QVariant> map)
{
  map["Origin"] = originID;
  map["SeqNo"] = seqNo++;
  this->pushMessage(map);

  if (noForward)
    return;

  // randomly select a peer and send message
  QByteArray message;
  QDataStream * stream = new QDataStream(&message, QIODevice::WriteOnly);
  (*stream) << map;
  delete stream;

  this->beginRumor(message);

  // temporary hack for saying "hello" to your friendly neighbors
  // seqNo starts at two because it's been incremented already above
  if (seqNo == 2){
    for (int p = myPortMin; p <= myPortMax; p++){
      if (p != myPort){
        this->writeDatagram (message.data(), message.size(), QHostAddress(QHostAddress::LocalHost), p);
      }
    }
  }
}

// sends a want message
void NetSocket::sendResponse(Peer *peer)
{
  QMap<QString, QVariant > map;
  map.insert("Want", want);

  QByteArray message;
  QDataStream * stream = new QDataStream(&message, QIODevice::WriteOnly);
  (*stream) << map;
  delete stream;

  this->writeDatagram (message.data(), message.size(), peer->hostAddress, peer->port);
}

void NetSocket::sendDirectMessage(QMap<QString, QVariant> map)
{
  if (noForward)
    return;

  if (!map.contains("Origin")){
    quint32 hopLimit = 10;
    map["HopLimit"] = hopLimit;
    map["Origin"] = originID;
  } else {
    if (map["HopLimit"].toInt() <= 0){
      return;
    }
    map["HopLimit"] = map["HopLimit"].toUInt() - 1;
  }

  QPair<QHostAddress,quint16> routes = peerManager->getRoutes(map["Dest"].toString());
  if (!routes.first.isNull()){

    QByteArray message;
    QDataStream * stream = new QDataStream(&message, QIODevice::WriteOnly);
    (*stream) << map;
    delete stream;

    this->writeDatagram (message.data(), message.size(), 
        routes.first, routes.second);
  }
}

void NetSocket::fileRequestSender(QMap<QString, QVariant> map)
{
  if (map.contains("blockRequest"))
    this->blockRequestSender(map["blockRequest"].toString());
  else if (map.contains("searchRequest"))
    this->searchRequestInit(map["searchRequest"].toString());
  else if (map.contains("fileName"))
    this->fileRequestFromSearch(map["fileName"].toString());
}

void NetSocket::blockRequestSender(QString downloadStr)
{
  int idx = downloadStr.lastIndexOf(":");
  if (idx == -1){
    return;
  }

  QByteArray hashArr;
  hashArr.append(downloadStr.right(downloadStr.size() - idx - 1));
  QVariant hash = hashArr;
  QVariant peer = downloadStr.left(idx);

  QMap<QString, QVariant> map;
  map.insert("Dest", peer);
  QByteArray hashByte = QByteArray::fromHex(hash.toString().toAscii());
  map.insert("BlockRequest", hashByte);

  this->sendDirectMessage(map);
}

void NetSocket::fileRequestFromSearch(QString fileName)
{
  if (searchResponse.second.contains(fileName))
    this->blockRequestSender(searchResponse.second[fileName]);
}

void NetSocket::searchRequestInit(QString searchStr)
{
  fileManager->newSearchRequest(searchStr);

  QHash<QString, QString> searchResponses;
  QList<QString> hashes;

  searchResponse.first = searchStr;
  searchResponse.second = searchResponses;
  searchHashIndex.erase(searchHashIndex.begin(), searchHashIndex.end());

  QMap<QString, QVariant> map;
  map.insert("Search", searchStr);
  map.insert("Budget", (quint32) 2);

  this->searchRequestSender(map);
}

void NetSocket::searchRequestSender(QMap<QString, QVariant> map)
{
  Peer *peer = NULL;
  this->searchRequestSender(map, peer);
}

void NetSocket::searchRequestSender(QMap<QString, QVariant> map, Peer *sender)
{
  quint32 budget = map["Budget"].toUInt();

  if (!map.contains("Origin"))
    map["Origin"] = originID;
  else
    map["Budget"] = --budget;

  if (budget <= 0)
    return;

  Peer *peer = NULL;
  QList<Peer*> peerPorts = peerManager->peerPorts;
  double noPeers = peerPorts.size();

  if (noPeers <= 0)
    return;

  if (budget <= noPeers){
    map["Budget"] = (quint32) 1;
    for (; budget > 0; budget--){
      peer = peerManager->randomPeer();
      if (peer && peer != sender){
        QByteArray message;
        QDataStream * stream = new QDataStream(&message, QIODevice::WriteOnly);
        (*stream) << map;
        this->writeDatagram (message.data(), message.size(), peer->hostAddress, peer->port);
        delete stream;
      }
    }
  } else {
    quint32 newBudget = ((long) budget)/noPeers + .5;
    map["Budget"] = newBudget;
    int i = 0;
    for (; i < noPeers-1; ++i){
      peer = peerPorts.at(i);
      if (peer && peer != sender){
        QByteArray message;
        QDataStream * stream = new QDataStream(&message, QIODevice::WriteOnly);
        (*stream) << map;
        this->writeDatagram (message.data(), message.size(), peer->hostAddress, peer->port);
        delete stream;
      }
      budget -= newBudget;
    }

    if (noPeers == 1)
      peer = peerPorts.at(0);
    else 
      peer = peerPorts.at(i);

    if (peer && peer != sender){
      map["Budget"] = budget;
      QByteArray message;
      QDataStream * stream = new QDataStream(&message, QIODevice::WriteOnly);
      (*stream) << map;
      this->writeDatagram (message.data(), message.size(), peer->hostAddress, peer->port);
      delete stream;
    }
  }
}

/*****************************/
/*													 */
/*     Message Recievers     */
/*													 */
/*****************************/

void NetSocket::messageReciever()
{
  QByteArray data;
  data.resize(this->pendingDatagramSize());
  QHostAddress sender;
  quint16 senderPort;
  this->readDatagram(data.data(), data.size(), &sender, &senderPort);

  QMap<QString, QVariant> map;
  QDataStream * stream = new QDataStream(&data, QIODevice::ReadOnly);
  (*stream) >> map;
  delete stream;

  Peer *peer = peerManager->checkPeer(sender, senderPort);

  if (map.contains("LastIP") && map.contains("LastPort"))
    peerManager->checkPeer(map["LastIP"].toUInt(), map["LastPort"].toUInt());

  if (map.contains("Dest"))
    this->directMessageReciever(map, peer);
  else if (map.contains("Search"))
    this->searchRequestReciever(map, peer);
  else if (map.contains("Origin") && !map.contains("Want"))
    this->chatReciever(map, peer);
  else if (map.contains("Want"))
    this->statusReciever(map, peer);
  else 
    qDebug() << "--------Bad Message-------" << map << "-----------------";

}

// delegated duties form messageReciever for private messages
void NetSocket::directMessageReciever(QMap<QString, QVariant> map, Peer *peer)
{
  if (noForward)
    return;

  QString dest = map["Dest"].toString();
  QString origin = map["Origin"].toString();

  // this is for me
  if (dest == originID){
    if (map.contains("BlockRequest"))
      this->blockRequestReciever(map);
    else if (map.contains("BlockReply"))
      this->blockReplyReciever(map);
    else if (map.contains("SearchReply"))
      this->searchReplyReciever(map);
    else {
      QVariant message = origin + " (via DM): " + map["ChatText"].toString();
      emit messageRecieved(message);
    }
  } else {
    this->sendDirectMessage(map);
  }


  if (origin != originID)
    this->sendResponse(peer);
}

// delegated duties from directMessageReciever for block requests
void NetSocket::blockRequestReciever(QMap<QString, QVariant> map)
{
  QMap<QString, QVariant> response;
  response = fileManager->fileFinder(map["BlockRequest"]);
  if (!response.isEmpty()){
    response.insert("Dest", map["Origin"]);
    this->sendDirectMessage(response);
  }
}

// delegated duties from directMessageReciever for block replies
void NetSocket::blockReplyReciever(QMap<QString, QVariant> map)
{
  QMap<QString, QVariant> response;
  QByteArray request = fileManager->getNextBlock(map);
  if (!request.isEmpty()){
    response.insert("Dest", map["Origin"]);
    response.insert("BlockRequest", request);
    this->sendDirectMessage(response);
  }
}

// deleted duties from directMessageReciever for search replies
void NetSocket::searchReplyReciever(QMap<QString, QVariant> map)
{
  QList<QVariant> matchNames = map["MatchNames"].toList();
  QList<QVariant> matchIDs = map["MatchIDs"].toList();

  int i = 0;
  for (i=0; i < matchNames.length(); i++){
    QByteArray fileHash = matchIDs[i].toByteArray();
    if (!searchHashIndex.contains(fileHash)){
      searchHashIndex << fileHash;
      QString fileString = map["Origin"].toString() + ":";
      fileString.append(matchIDs[i].toByteArray().toHex());
      QString fileName = matchNames[i].toString();
      searchResponse.second.insert(fileName, fileString);
      emit refreshSearchResults(fileName);
    }
  }
}

// deleted duties from messageReciever for search responses
void NetSocket::searchRequestReciever(QMap<QString, QVariant> map, Peer *peer)
{
  QMap<QString, QVariant> response;
  response = fileManager->fileSearch(map["Search"].toString());
  if (!response.isEmpty()){
    response.insert("Dest", map["Origin"]);
    this->sendDirectMessage(response);
  }
  this->searchRequestSender(map, peer);
}

// delegated duties from messageReciever for chat and route messages
void NetSocket::chatReciever(QMap<QString, QVariant> map, Peer *peer)
{
  QString origin = map["Origin"].toString();
  if (!want.contains(origin))
    want.insert(origin, 1);

  // the one I need next, add it and start rumormongering
  if (want[origin].toUInt() == map["SeqNo"].toUInt()){

    if (map.contains("ChatText")){
      QVariant message = origin + ": " + map["ChatText"].toString();
      emit messageRecieved(message);
    }

    peerManager->updateRoutes(origin, peer);
    this->pushMessage(map);

    map["LastIP"] = peer->hostAddress.toIPv4Address();
    map["LastPort"] = peer->port;

    QByteArray data;
    QDataStream * stream = new QDataStream(&data, QIODevice::WriteOnly);
    (*stream) << map;
    delete stream;

    if (map.contains("ChatText") && !noForward)
      this->beginRumor(data);
    else
      this->allRumor(data);

  } else if (want[origin].toUInt() == map["SeqNo"].toUInt() + 1 && !map.contains("LastIP") ) {
    peerManager->updateRoutes(origin, peer);
  }

  // want
  if (origin != originID)
    this->sendResponse(peer);
}

// delegated duties from messageReciever for want messages
void NetSocket::statusReciever(QMap<QString, QVariant> map, Peer *peer)
{
  QMap<QString, QVariant> responseMap;
  responseMap = this->getMessage(map["Want"].toMap());
  if (responseMap.contains("Origin")){
    QByteArray responseMessage;
    QDataStream * responseStream = new QDataStream(&responseMessage, QIODevice::WriteOnly);
    (*responseStream) << responseMap;
    delete responseStream;
    this->writeDatagram (responseMessage.data(), responseMessage.size(), peer->hostAddress, peer->port);
  } else if (responseMap.contains("Want")){
    this->sendResponse(peer);
  }
}

/*****************************/
/*													 */
/*     Database Interface    */
/*													 */
/*****************************/

void NetSocket::pushMessage(QMap<QString, QVariant> map)
{
  QVariant message = map["ChatText"];
  QString origin = map["Origin"].toString();
  QString seqNo = map["SeqNo"].toString();

  want[origin] = map["SeqNo"].toUInt() + 1;
  messages[origin].insert(seqNo, message);
}

// finds the next message, otherwise returns want
QMap<QString, QVariant> NetSocket::getMessage(QMap<QString, QVariant> peerWant)
{
  bool flag = false; // keeps track of whether you have something I want
  QMap<QString, QVariant> response;

  QMap<QString, QVariant>::iterator i = peerWant.begin();
  QMap<QString, QVariant>::iterator j = want.begin();
  QMap<QString, QVariant>message;

  if (noForward){
    while (true){
      if (i == peerWant.end() || j == want.end()){
        // you have more wants - so therefore you have extra messages
        if (i != peerWant.end()){
          flag = true;
        }
        break;
      }
      if (i.key() == j.key()){
        // we share a peer - check messages
        if (i.value().toInt() > j.value().toInt()) {
          flag = true;
          break;
        }
      } else if (i.key() < j.key()){
        // you have a peer I don't have
        flag = true;
        break;
      }
      i++; j++;
    }
    if (flag){
      response.insert("Want", "true");
    }
    return response;
  }

  while (true){
    if (i == peerWant.end() || j == want.end()){
      // you have more wants - so therefore you have extra messages
      if (i != peerWant.end()){
        flag = true;
      }
      // I have more peers - so therefore I have extra messages
      if (j != want.end()){
        message = messages[j.key()];
      }
      break;
    }
    if (i.key() == j.key()){
      // we share a peer - check messages
      if (i.value().toInt() < j.value().toInt()){
        if (messages.contains(i.key())){
          QMap<QString, QVariant>message = messages[i.key()];
          QVariant chatText = message[i.value().toString()];
          if (!chatText.isNull())
            response.insert("ChatText", chatText);
          response.insert("Origin", i.key());
          response.insert("SeqNo", i.value());
          return response;
        }
      } else if (i.value().toInt() > j.value().toInt()) {
        flag = true;
      }
      i++; j++;
    } else if (i.key() < j.key()){
      // you have a peer I don't have
      flag = true;
      i++;
    } else {
      // I have a peer you don't
      message = messages[j.key()];
      break;
    }
  }

  // case is I had a peer you didn't, so I send you the first message
  if (!message.isEmpty()){
    QVariant chatText = message["1"];
    if (!chatText.isNull())
      response.insert("ChatText", chatText);
    response.insert("Origin", j.key());
    response.insert("SeqNo", "1");
    return response;
  }

  // you have a message I don't, and I have nothing new for you
  if (flag){
    response.insert("Want", "true");
  }

  // need a better way to loop and check to make sure the two match up
  return response;
}

/*****************************/
/*													 */
/*   Rumormongering Manager  */
/*													 */
/*****************************/

// called every x seconds by a timer initialized in NetSocker::initialize()
void NetSocket::antiEntropy()
{
  Peer *randomPeer = peerManager->randomPeer();
  if (randomPeer != NULL)
    sendResponse(randomPeer);
}

void NetSocket::sendRouteRumor()
{
  Peer *peer = peerManager->randomPeer();
  if (peer != NULL){
    this->sendRouteRumorToPeer(peer);
  }
}

void NetSocket::sendRouteRumorToPeer(Peer *peer)
{
  QMap<QString, QVariant> map;
  map["Origin"] = originID;
  map["SeqNo"] = seqNo++;

  QByteArray message;
  QDataStream * stream = new QDataStream(&message, QIODevice::WriteOnly);
  (*stream) << map;
  delete stream;

  this->writeDatagram (message.data(), message.size(), peer->hostAddress, peer->port);
}

void NetSocket::allRumor(QByteArray data)
{
  Peer *peer = NULL;
  QList<Peer*> peerPorts = peerManager->peerPorts;
  for (int i = 0; i < peerPorts.size(); ++i){
    peer = peerPorts.at(i);
    this->writeDatagram (data.data(), data.size(), peer->hostAddress, peer->port);
  }
}

// starts a rumor message
void NetSocket::beginRumor(QByteArray data)
{
  Peer *randomPeer = peerManager->randomPeer();
  if (randomPeer != NULL && !data.isEmpty()){
    this->writeDatagram (data.data(), data.size(), randomPeer->hostAddress, randomPeer->port);
    randomPeer->data = data;
    randomPeer->timer->stop();
    randomPeer->timer->start();
  }
}

// flips a coin for resending a rumor message
void NetSocket::resendRumor()
{
  if (qrand() % 2){
    Peer *peer = qobject_cast<Peer *> (QObject::sender()->parent());
    if (peer){
      QByteArray data = peer->data;
      this->beginRumor(data);
      peer->data = "";
    }
  }
}

/*****************************/
/*													 */
/*      Secret Sharing       */
/*													 */
/*****************************/

void NetSocket::sendSecret(qint32 secret, quint32 secretNo)
{
  // Get the list of peers.
  QList<Peer *> peerPorts = peerManager->peerPorts;

  // Get the number of peers to determine the threshold.
  // Let the threshold k be 75% of the total number of peers.
  qint16 numPeers = peerPorts.count();
  qint16 threshold = (numPeers * 3) / 4;

  // Break up the secret into n shares, where n is the number of peers.
  QList<QPair<qint16, qint64> > secretShares =
    ShamirSecret::generateSecrets(secret, numPeers, threshold);

  // For each peer, send it its secret share.
  for (int i = 0; i < peerPorts.count(); i++) {
    QPair<qint16, qint64> sharePair = secretShares.at(i);
    QVariantList secretShare;
    secretShare.push_back(sharePair.first);
    secretShare.push_back(sharePair.second);

    // Build the message.
    QVariantMap secretMsg;
    secretMsg.insert("SecretShare", secretShare);
    secretMsg.insert("Dest", peerPorts.at(i)->hostName);
    secretMsg.insert("SecretNo", secretNo);

    // Send the secret share to the peer.
    sendDirectMessage(secretMsg);
  }

  qDebug() << "Sent all secret shares to peers.";
}

void NetSocket::requestSecret(QString secretId)
{
  quint32 secretNo = secretId.toUInt();
  // Build the message.
  QVariantMap secretRequestMsg;
  secretRequestMsg.insert("SecretRequest", secretNo);
  
}


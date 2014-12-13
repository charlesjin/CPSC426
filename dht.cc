// dht.cc by Kayo Teramoto
#include <math.h>
#include "dht.hh"

/*****************************/
/*                           */
/*            DHT            */
/*                           */
/*****************************/

DHTManager::DHTManager(QString originID, quint16 port, QHostAddress hostAddress)
{
  self = new Node((int) (qrand() % 128), port, hostAddress);
  self->successor = self;
  self->predecessor = self;
  this->originID = originID;
  sizeDHT = 128;

  checkPredecessorTimer = new QTimer(this);
  connect(timer, SIGNAL(timeout()), this, SLOT(update()));
  timer->start(10000);

  failureTimer = new QTimer(this);
  failureTimer.setSingleShot();
  connect(timer, SIGNAL(timeout()), 
    this, SLOT(predecessorFailed()));

}

int DHTManager::getIndex()
{
  return self->index;
}

void DHTManager::successorRequest(QMap<QString, QVariant> map, Peer *peer)
{
  Node *nn = findSuccessor(map["SuccessorRequest"].toInt(), peer, map["Origin"].toString());
  if (nn != NULL) {
    // Send message back
    QVariantMap map;
    map.insert("SuccessorResponse", nn->index);
    map.insert("SuccessorPort", nn->port);
    map.insert("SuccessorHostAddress", nn->hostAddress.toString());
    map.insert("Predecessor", this->self->index);
    map.insert("PredecessorPort", this->self->port);
    map.insert("PredecessorHostAddress", this->self->hostAddress.toString());
    map.insert("Dest", map["Origin"].toString());
    map.insert("Index", map["SuccessorRequest"].toInt());

    emit sendDHTMessage(map, map["RequestPort"].toUInt(), QHostAddress(map["RequestHostAddress"].toString()));
  }

}

Node *DHTManager::findSuccessor(int index, Peer *peer, QString peerOriginID)
{
  if (index > this->self->index && index <= this->self->successor->index) {
    return this->self->successor;
  } else {
    Node *nn = closestPrecedingNode(index);
    // Ask nn to find the successor of index and return that node
    askForSuccessor(nn, index, peer, peerOriginID);
    return NULL;
  }
}

Node *DHTManager::closestPrecedingNode(int index)
{
  for (int i = finger.size() - 1; i >= 0; i--) {
    if (finger[i].start > this->self->index && finger[i].start < index) {
      return finger[i].node;
    }
  }
  return this->self;
}

void DHTManager::askForSuccessor(Node* nn, int index, Peer *peer, QString peerOriginID)
{
  QVariantMap map;
  map.insert("SuccessorRequest", index);
  map.insert("RequestPort", peer->port);
  map.insert("RequestHostAddress", peer->hostAddress.toString());
  map.insert("Origin", peerOriginID);

  emit sendDHTMessage(map, nn->port, nn->hostAddress);
}

bool DHTManager::isInDHT()
{
  return (finger.size() > 0);
}

// This is the first node in the network
void DHTManager::initializeDHT()
{
  // Initialize the DHT
  for (int i = 0; i < (int) ceil(log2(this->sizeDHT)); i++) {
    FingerEntry entry;
    entry.start = self->index + pow(2, i);
    entry.node = this->self;
    finger << entry;
  }
  this->self->predecessor = this->self;
}

void DHTManager::join(Peer *peer, QMap<QString, QVariant> map)
{
  if (!peer) return;
  this->initFingerTable(map, peer);
  this->updateOthers();
}

// CALLED DURING INIT
void DHTManager::initFingerTable(QMap<QString, QVariant> map, Peer *peer)
{
  Node *predecessor = new Node(map["Predecessor"].toInt(), map["PredecessorPort"].toUInt(), 
    QHostAddress(map["PredecessorHostAddress"].toString()));
  Node *successor = new Node(map["SuccessorResponse"].toInt(), map["SuccessorPort"].toUInt(),
      QHostAddress(map["SuccessorHostAddress"].toString()));
  this->self->predecessor = predecessor;
  this->self->successor = successor;

  int i = 0;
  FingerEntry entry;
  for (i = 0; i < (int) ceil(log2(this->sizeDHT)); i++){
    entry.start = self->index + pow(2, i);
    finger << entry;
  }
  for (i = 1; i < (int) ceil(log2(this->sizeDHT)); i++){
    if (finger[i].start >= self->index && finger[i].start < finger[i-1].start){
      finger[i].node = finger[i-1].node;
    } else {
      this->findSuccessor(finger[i].start, peer, originID);
    }
  }

  // update successor
  QMap<QString, QVariant> updateMap;
  updateMap.insert("updateIndex", this->self->index);
  updateMap.insert("updatePort", this->self->port);
  updateMap.insert("updateHostAddress", this->self->hostAddress.toString());
  updateMap.insert("newPredecessor", this->originID);
  emit sendDHTMessage(updateMap, this->self->successor->port, this->self->successor->hostAddress);
}

void DHTManager::updateOthers()
{
  QMap<QString, QVariant> map;
  map.insert("updateIndex", this->self->index);
  map.insert("updatePort", this->self->port);
  map.insert("updateHostAddress", this->self->hostAddress.toString());

  for (int i = 0; i < finger.size(); i ++){
    int index = this->self->index - pow(2, i);
    map.insert("updateIndex", index);
    this->updatePredecessor(map);
  }
}

void DHTManager::updatePredecessor(QVariantMap map)
{
  int index = map["updateIndex"].toUInt();
  Node *nn = closestPrecedingNode(index);
  if (nn == this->self){
    Node *node = finger[index].node;
    node->index = index;
    node->port = map["updatePort"].toUInt();
    node->hostAddress = QHostAddress(map["updateHostAddress"].toString());
    return;
  }
  emit sendDHTMessage(map, nn->port, nn->hostAddress);
}

void DHTManager::updateFingerTable(QMap<QString, QVariant> map)
{
  int fingerIdx = log2(map["SuccessorRequest"].toInt() - self->index);
  Node *node = new Node(map["SuccessorResponse"].toUInt(), map["SuccessorPort"].toUInt(),
    QHostAddress(map["SuccessorHostAddress"].toString()));
  finger[fingerIdx].node = node;
}

void DHTManager::newPredecessor(QMap<QString, QVariant> map)
{
  Node *node = this->self->predecessor;
  node->index = map["updateIndex"].toUInt();
  node->port = map["updatePort"].toUInt();
  node->hostAddress = QHostAddress(map["updateHostAddress"].toString());
}

// stabilize functions

void DHTManager::stabilizeBegin() 
{
  // ask for successor's predecessor
  QMap<QString, QVariant> map;
  map.insert("StoredPredecessorRequest", this->self->index);
  
  emit sendDHTMessage(map, this->self->successor->port, this->self->successor->hostAddress);
}

void DHTManager::stabilize(QMap<QString, QVariant> map)
{
  int idx = map["StoredPredecessorResponse"].toInt();

  if (idx > this->self->index && idx < this->self->successor->index) {
    this->self->successor->index = idx;
    this->self->successor->port = map["StoredPredecessorPort"].toUInt();
    this->self->successor->hostAddress = QHostAddress(map["StoredPredecessorHostAddress"].toString());
  }

  QMap<QString, QVariant> msg;
  msg.insert("Notify", this->self->index);
  map.insert("SenderPort", this->self->port);
  map.insert("SenderHostAddress", this->self->hostAddress.toString());
  emit sendDHTMessage(map, this->self->successor->port, this->self->successor->hostAddress);
}

void DHTManager::notify (QMap<QString, QVariant> map) 
{
  int idx = map["Notify"].toUInt();

  if (this->self->predecessor == NULL) {

    Node *node = new Node(idx, map["NotifyPort"].toUInt(), 
      QHostAddress(map["NotifyHostAddress"].toString()));
    this->self->predecessor = node;
  } 

  else if (idx > this->self->predecessor->index && idx < this->self->index) {

    this->self->predecessor->index = idx;
    this->self->predecessor->port = map["SenderPort"].toUInt();
    this->self->predecessor->hostAddress = QHostAddress(map["SenderHostAddress"].toString());
  }
}

void DHTManager::sendCurrentPredecessor(Peer *peer) 
{
  QMap<QString, QVariant> map;
  map.insert("StoredPredecessorResponse", this->self->predecessor->index);
  map.insert("StoredPredecessorPort", this->self->predecessor->port);
  map.insert("StoredPredecessorHostAddress", this->self->predecessor->hostAddress.toString())

  emit sendDHTMessage(map, peer->port, peer->hostAddress.toString());
}

void DHTManager::checkPredecessor() 
{
  QMap<QString, QVariant> map;
  map.insert("Heartbeat", 0);
  emit sendDHTMessage(map, this->self->predecessor->port, this->self->predecessor->hostAddress.toString());
  failureTimer->start(5000);
}

void DHTManager::receivedHeartbeat() 
{
  failureTimer->stop();
}

void DHTManager::predecessorFailed() 
{
  this->self->predecessor = NULL;
  qDebug() << "predecessor has failed";
}

/*****************************/
/*                           */
/*            Node           */
/*                           */
/*****************************/

Node::Node(int index, quint16 port, QHostAddress hostAddress)
{
  this->index = index;
  this->port = port;
  this->hostAddress = hostAddress;
  this->predecessor = NULL;
  this->successor = NULL;
}

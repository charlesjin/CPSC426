// dht.cc by Kayo Teramoto
#include <math.h>
#include "dht.hh"

/*****************************/
/*                           */
/*            DHT            */
/*                           */
/*****************************/

// int DHTManager::getIndex()
// {
//   return self->index;
// }

void DHTManager::updatePredecessorReciever(QMap<QString, QVariant> map)
{
  qDebug() << "========UPDATING PREDECESSOR";
  this->self->predecessor->index = map["UpdatePredecessor"].toUInt();;
  this->self->predecessor->port = map["PredecessorPort"].toUInt();
  this->self->predecessor->hostAddress = QHostAddress(map["PredecessorHostAddress"].toString());
  qDebug() << "";
  qDebug() << "";
  qDebug() << "Predecessor updated";
  qDebug() << "PRED IS" << this->self->predecessor->index;
  qDebug() << "SUCC IS" << this->self->successor->index;
  qDebug() << "";
  qDebug() << "";
}

void DHTManager::successorRequest(QMap<QString, QVariant> map, Peer *peer)
{
  qDebug() << "SUCCESSOR REQUEST" << map;
  qDebug() << "";
  Node *nn = findSuccessor(map["SuccessorRequest"].toInt(), peer, map["Origin"].toString(),
      map["FingerEntryNum"].toInt());
  if (nn != NULL) {
    // You are the successor. Update your predecessor and send message back.
    // REALLY NEED TO CHECK THIS
 //   if (nn->index == this->self->index) {
 //     qDebug() << "I am the successor" << this->self->index << this->self->successor->index;
 //     qDebug() << "";
 //     this->self->predecessor = new Node(map["SuccessorRequest"].toInt() - 1,
 //         map["RequestPort"].toUInt(), QHostAddress(map["RequestHostAddress"].toString()));
 //     qDebug() << "";
 //     qDebug() << "";
 //     qDebug() << "";
 //     qDebug() << "PRED IS" << this->self->predecessor->index;
 //     qDebug() << "SUCC IS" << this->self->successor->index;
 //     qDebug() << "";
 //     qDebug() << "";
 //     qDebug() << "";
 //   }

    // Send a message to the successor asking it to update its predecessor
    if (map["FingerEntryNum"].toInt() == 0) {
      QVariantMap upMap;
      upMap.insert("UpdatePredecessor", map["SuccessorRequest"].toInt() - 1);
      upMap.insert("PredecessorPort", map["RequestPort"].toUInt());
      upMap.insert("PredecessorHostAddress", map["RequestHostAddress"].toString());
      qDebug() << "SENDING AN UPDATE PREDECESSOR MESSAGE" << upMap << "IN RESPONSE TO" << map;
    
      emit sendDHTMessage(upMap, nn->port, nn->hostAddress);
    }

    // Respond to original successor request
    QVariantMap newMap;
    newMap.insert("SuccessorResponse", nn->index);
    newMap.insert("SuccessorPort", nn->port);
    newMap.insert("SuccessorHostAddress", nn->hostAddress.toString());
    newMap.insert("Predecessor", this->self->index);
    newMap.insert("PredecessorPort", this->self->port);
    newMap.insert("PredecessorHostAddress", this->self->hostAddress.toString());
    //newMap.insert("Dest", map["Origin"].toString());
    newMap.insert("Index", map["SuccessorRequest"].toInt());
    newMap.insert("FingerEntryNum", map["FingerEntryNum"].toInt());

    // qDebug() << "Sending DHT Message back" << newMap;

    emit sendDHTMessage(newMap, map["RequestPort"].toUInt(), QHostAddress(map["RequestHostAddress"].toString()));
  }
}

Node *DHTManager::findSuccessor(int index, Peer *peer, QString peerOriginID, int fingerEntryNum)
{
  index = index % sizeDHT;
  int me = index;
  int low = this->self->index;
  int high = this->self->successor->index;
  qDebug() << "FIND SUCCESSOR" << me << low << high;
  qDebug() << "";
  if (me == low || inRange(me, low, high)) {
    return this->self->successor;
    //qDebug() << "===========================================";
    //askForSuccessor(this->self->successor, index, peer, peerOriginID, fingerEntryNum);
  } else {
    Node *nn = closestPrecedingNode(index);
    if (nn->index == this->self->index) {
      return this->self;
    }
    // Ask nn to find the successor of index and return that node
    // if (nn->port != peer->port || nn->hostAddress != peer->hostAddress)
      askForSuccessor(nn, index, peer, peerOriginID, fingerEntryNum);
    // else
    //   return this->self;
  
    return NULL;
  }
}

Node *DHTManager::closestPrecedingNode(int index)
{
  index = index % sizeDHT;
  for (int i = finger.size() - 1; i >= 0; i--) {
    int me = finger[i].node->index;
    int low = this->self->index;
    int high = index;
    if (me == high || inRange(me, low, high)){
      qDebug() << "CLOSEST PRECEDING NODE" << finger[i].node->index << me << low << high;
      return finger[i].node;
    }
  }
  qDebug() << "CLOSEST PRECEDING NODE" << this->self->index;
  return this->self;
}

void DHTManager::askForSuccessor(Node* nn, int index, Peer *peer, QString peerOriginID, int fingerEntryNum)
{
  qDebug() << "ASK FOR SUCCESSOR" << nn->port << "" << nn->hostAddress;
  qDebug() << "";
  index = index % sizeDHT;

  QVariantMap map;
  map.insert("SuccessorRequest", index);
  map.insert("RequestPort", peer->port);
  map.insert("RequestHostAddress", peer->hostAddress.toString());
  map.insert("Origin", peerOriginID);
  map.insert("FingerEntryNum", fingerEntryNum);

  if (nn == NULL) {
    qDebug() << "PROBLEM";
    return;
  }
  if (nn->port == 0) {
    qDebug() << "PROBLEM" << nn;
    return;
  }
  qDebug() << "ASK FOR SUCCESSOR" << map;

  emit sendDHTMessage(map, nn->port, nn->hostAddress);
}

// This is the first node in the network
void DHTManager::initializeDHT(int peerIndex, quint16 port, QHostAddress hostAddress)
{
  // Initialize the DHT
  for (int i = 0; i < (int) ceil(log2(this->sizeDHT)); i++) {
    FingerEntry entry;
    entry.start = (self->index + (int) pow(2, i)) % sizeDHT;
    entry.node = new Node(this->self->index, this->self->port, this->self->hostAddress);//0, QHostAddress());
    finger << entry;
  }
  this->self->predecessor = new Node(peerIndex, port, hostAddress);
  this->self->successor = new Node(peerIndex, port, hostAddress);
  fingerTableUpdated();
}

void DHTManager::join(Peer *peer, QMap<QString, QVariant> map)
{
  if (!peer) return;
  this->initFingerTable(map, peer);
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

  int fingerTableSize = (int) ceil(log2(this->sizeDHT));
  for (i = 0; i < fingerTableSize; i++){
    entry.start = (self->index + (int) pow(2, i)) % sizeDHT;
    entry.node = new Node(this->self->index, this->self->port, //QHostAddress());
        this->self->hostAddress);
    finger << entry;
  }

  fingerTableUpdated();

  finger[0].node = new Node(successor->index, successor->port, successor->hostAddress);
  initFingerTableHelper(1, peer);
  initSuccessorList();
}

void DHTManager::initFingerTableHelper(int iterNum, Peer *peer) {
    if (iterNum == log2(this->sizeDHT)) {
      // Done
      this->updateOthers();
      fingerTableUpdated();
      return;
    }
    qDebug() << "170" << finger[iterNum].start << " " << self->index << " " << finger[iterNum-1].node->index;
    qDebug() << "";
    int me = finger[iterNum].start;
    int low = self->index;
    int high = finger[iterNum-1].node->index;
    if (me == high || inRange(me, low, high)) {
      qDebug() << "YES" << iterNum;
      qDebug() << "";
      if (finger[iterNum-1].node->index == 0) {
        qDebug() << "PROBLEM AREA";
      }
      finger[iterNum].node->index = finger[iterNum-1].node->index;
      finger[iterNum].node->port = finger[iterNum-1].node->port;
      finger[iterNum].node->hostAddress = finger[iterNum-1].node->hostAddress;
      initFingerTableHelper(iterNum + 1, peer);
    } else {
      qDebug() << "ELSE" << iterNum;
      Peer *p = new Peer();
      p->port = this->self->port;
      p->hostAddress = this->self->hostAddress;
      Node *nn = new Node(0, peer->port, peer->hostAddress);
      this->askForSuccessor(nn, finger[iterNum].start, p, this->originID, iterNum);
    }
}

void DHTManager::initFingerTableHelper(QMap<QString, QVariant> map)
{
  int entryNum = map["FingerEntryNum"].toInt();
  Peer *peer = new Peer();
  peer->port = finger[0].node->port;
  peer->hostAddress = finger[0].node->hostAddress;
  initFingerTableHelper(entryNum + 1, peer);
}

void DHTManager::updateOthers()
{
  qDebug() << "UPDATE OTHERS";
  qDebug() << "";

  QMap<QString, QVariant> map;
  map.insert("updatePort", this->self->port);
  map.insert("updateHostAddress", this->self->hostAddress.toString());
  map.insert("updateValue", this->self->index);

  for (int i = 0; i < finger.size(); i ++){
    int index = this->self->index - pow(2, i);
    if (index < 0)
      index += sizeDHT;
    index = index % sizeDHT;
    map.insert("updateIndex", index);
    map.insert("updateFingerIndex", i);
    this->updatePredecessor(map);
  }
}

void DHTManager::updatePredecessor(QVariantMap map)
{
  qDebug() << "UPDATE PREDECESSOR " << map;

  int index = map["updateIndex"].toUInt();
  index = index % sizeDHT;
  Node *nn = closestPrecedingNode(index);
  if (nn->index == this->self->index) {
    Node *node = finger[map["updateFingerIndex"].toUInt()].node;
    if (node == NULL) {
      node = new Node(0, 0, QHostAddress());
    }
    int me = map["updateValue"].toUInt();
    int low = finger[map["updateFingerIndex"].toUInt()].start;
    int high = node->index;
    qDebug () << "325 " << me << low << high;

    updateSuccessorList(map);

    if (me == low || inRange(me, low, high)) {
      node->index = map["updateValue"].toUInt();
      node->port = map["updatePort"].toUInt();
      node->hostAddress = QHostAddress(map["updateHostAddress"].toString());
      if (map["updateFingerIndex"].toInt() == 0) {
        this->self->successor->index = map["updateValue"].toUInt();
        this->self->successor->port = map["updatePort"].toUInt();
        this->self->successor->hostAddress = QHostAddress(map["updateHostAddress"].toString());
      qDebug() << "================================";
      qDebug() << "";
      qDebug() << "";
      qDebug() << "Successor updated";
      qDebug() << "PRED IS" << this->self->predecessor->index;
      qDebug() << "SUCC IS" << this->self->successor->index;
      qDebug() << "";
      qDebug() << "";
      qDebug() << "==============";
      }
      fingerTableUpdated();
      //qDebug() << "HERE";
      emit sendDHTMessage(map, this->self->predecessor->port, this->self->predecessor->hostAddress);
    } else {
      qDebug() << "No update necessary" << me << low << high;
      qDebug() << "";
    }
    return;
  }
  // qDebug() << nn->index;
  emit sendDHTMessage(map, nn->port, nn->hostAddress);
}

void DHTManager::updateFingerTable(QMap<QString, QVariant> map)
{
  int diff = map["Index"].toInt() - self->index;
  if (diff < 0)
    diff += sizeDHT;
  diff = diff % sizeDHT;
  if (diff == 0) {
  //  qDebug() << "THE DIFF IS ZERO??" << map;
  //  qDebug() << "";
    return;
  }
  int fingerIdx = log2(diff);

  //qDebug() << "FINGER INDEX" << fingerIdx << "" << diff;
  //qDebug() << "";

  finger[fingerIdx].node->index = map["SuccessorResponse"].toUInt();
  finger[fingerIdx].node->port = map["SuccessorPort"].toUInt();
  finger[fingerIdx].node->hostAddress = QHostAddress(map["SuccessorHostAddress"].toString());

  updateSuccessorListFT(map);

  if (map["FingerEntryNum"].toInt() > 0) {
    initFingerTableHelper(map);
  } else {
    fingerTableUpdated();
  }
}

void DHTManager::fingerTableUpdated()
{
  QList<QPair<int, int> > table;
  for (int i = 0; i < finger.size(); i++) {
    table << qMakePair(finger[i].start, finger[i].node->index);
  }

  qDebug() << "THE TABLE IS" << table;
  qDebug() << "";

  emit fingerTableUpdatedSignal(table);
}


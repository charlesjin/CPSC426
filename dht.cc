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
  self->successor = new Node(self->index, self->port, QHostAddress());
      //self->hostAddress);
  self->predecessor = new Node(self->index, self->port, QHostAddress());
      //self->hostAddress);
  this->originID = originID;
  sizeDHT = 128;
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
    QVariantMap newMap;
    newMap.insert("SuccessorResponse", nn->index);
    newMap.insert("SuccessorPort", nn->port);
    newMap.insert("SuccessorHostAddress", nn->hostAddress.toString());
    newMap.insert("Predecessor", this->self->index);
    newMap.insert("PredecessorPort", this->self->port);
    newMap.insert("PredecessorHostAddress", this->self->hostAddress.toString());
    newMap.insert("Dest", map["Origin"].toString());
    newMap.insert("Index", map["SuccessorRequest"].toInt());

    qDebug() << "Sending DHT Message back";

    emit sendDHTMessage(newMap, map["RequestPort"].toUInt(), QHostAddress(map["RequestHostAddress"].toString()));
  }
  qDebug() << "NN " << nn;
}

Node *DHTManager::findSuccessor(int index, Peer *peer, QString peerOriginID)
{
  index = index % sizeDHT;
  if (index > this->self->index && index <= this->self->successor->index) {
    return this->self->successor;
  } else {
    Node *nn = closestPrecedingNode(index);
    if (nn == this->self) {
      qDebug() << "THIS IS HIT" << this->self->index;
      return this->self;
    }
    // Ask nn to find the successor of index and return that node
    askForSuccessor(nn, index, peer, peerOriginID);
    return NULL;
  }
}

Node *DHTManager::closestPrecedingNode(int index)
{
  index = index % sizeDHT;
  for (int i = finger.size() - 1; i >= 0; i--) {
    if (finger[i].start > this->self->index && finger[i].start < index) {
      return finger[i].node;
    }
  }
  return this->self;
}

void DHTManager::askForSuccessor(Node* nn, int index, Peer *peer, QString peerOriginID)
{
  index = index % sizeDHT;

  QVariantMap map;
  map.insert("SuccessorRequest", index);
  map.insert("RequestPort", peer->port);
  map.insert("RequestHostAddress", peer->hostAddress.toString());
  map.insert("Origin", peerOriginID);

  if (nn == NULL) {
    qDebug() << "PROBLEM";
    return;
  }
  qDebug() << "stuff" << nn->port << " "  << nn->hostAddress;
  if (nn->port == 0) {
    qDebug() << "PROBLEM" << nn;
    return;
  }
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
    entry.start = (self->index + (int) pow(2, i)) % sizeDHT;
    entry.node = new Node(this->self->index, this->self->port, QHostAddress());
        //this->self->hostAddress);
    finger << entry;
  }
  this->self->predecessor = new Node(this->self->index, this->self->port, QHostAddress());
      //this->self->hostAddress);
  fingerTableUpdated();
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
  qDebug() << "ONE";
  Node *predecessor = new Node(map["Predecessor"].toInt(), map["PredecessorPort"].toUInt(), 
    QHostAddress(map["PredecessorHostAddress"].toString()));
  Node *successor = new Node(map["SuccessorResponse"].toInt(), map["SuccessorPort"].toUInt(),
      QHostAddress(map["SuccessorHostAddress"].toString()));
  this->self->predecessor = predecessor;
  this->self->successor = successor;

  qDebug() << "TWO";
  int i = 0;
  FingerEntry entry;
  for (i = 0; i < (int) ceil(log2(this->sizeDHT)); i++){
    entry.start = (self->index + (int) pow(2, i)) % sizeDHT;
    entry.node = new Node(this->self->index, this->self->port, QHostAddress());
        // this->self->hostAddress);
    finger << entry;
  }

  //TESTING
  qDebug() << "TESTING EEEEEEEEEEEEEE";
  fingerTableUpdated();
  //
  //
  finger[0].node = successor;
  for (i = 1; i < finger.size(); i++){
    if (finger[i].start >= self->index && finger[i].start < finger[i-1].start){
      qDebug() << "IF " << i;
      finger[i].node = finger[i-1].node;
    } else {
      qDebug() << "ELSE " << i;
      this->findSuccessor(finger[i].start, peer, originID);
    }
  }
  fingerTableUpdated();
  qDebug() << "THREE";

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
  map.insert("updatePort", this->self->port);
  map.insert("updateHostAddress", this->self->hostAddress.toString());

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
  qDebug() << "THE MAP " << map;

  int index = map["updateIndex"].toUInt();
  index = index % sizeDHT;
  Node *nn = closestPrecedingNode(index);
  if (nn == this->self){
    Node *node = finger[map["updateFingerIndex"].toUInt()].node;
    if (node == NULL) {
      node = new Node(0, 0, QHostAddress());
      qDebug() << "null";
    }
    qDebug() << node;
    qDebug() << node->index;
    node->index = index;
    node->port = map["updatePort"].toUInt();
    qDebug() << map["updateHostAddress"].toString();
    node->hostAddress = QHostAddress(map["updateHostAddress"].toString());
    return;
  }
  emit sendDHTMessage(map, nn->port, nn->hostAddress);
}

void DHTManager::updateFingerTable(QMap<QString, QVariant> map)
{
  int diff = map["SuccessorResponse"].toInt() - self->index;
  if (diff < 0)
    diff += sizeDHT;
  diff = diff % sizeDHT;
  int fingerIdx = log2(diff);
  qDebug() << "FINGERIDX" << fingerIdx;

  finger[fingerIdx].node->index = map["SuccessorResponse"].toUInt();
  finger[fingerIdx].node->port = map["SuccessorPort"].toUInt();
  finger[fingerIdx].node->hostAddress = QHostAddress(map["SuccessorHostAddress"].toString());
  fingerTableUpdated();
}

void DHTManager::fingerTableUpdated()
{
  QList<QPair<int, int> > table;
  for (int i = 0; i < finger.size(); i++) {
    table << qMakePair(finger[i].start, finger[i].node->index);
  }

  qDebug() << "THE TABLE IS THIS" << table;
  qDebug() << "";
  emit fingerTableUpdatedSignal(table);
}

void DHTManager::newPredecessor(QMap<QString, QVariant> map)
{
  Node *node = this->self->predecessor;
  node->index = map["updateIndex"].toUInt();
  node->port = map["updatePort"].toUInt();
  node->hostAddress = QHostAddress(map["updateHostAddress"].toString());
}

////// stabilize functions
//
//void DHTManager::stabilizeBegin() 
//{
//  // ask for successor's predecessor
//  QMap<QString, QVariant> map;
//  map.insert("StoredPredecessorRequest", this->self->index);
//  
//  emit sendDHTMessage(map, this->self->successor->port, this->self->successor->hostAddress);
//}
//
//void DHTManager::stabilize(QMap<QString, QVariant> map)
//{
//  int idx = map["StoredPredecessorResponse"].toInt();
//
//  if (idx > this->self->index && idx < this->self->successor->index) {
//    this->self->successor->index = idx;
//    this->self->successor->port = map["StoredPredecessorPort"].toUInt();
//    this->self->successor->hostAddress = QHostAddress(map["StoredPredecessorHostAddress"].toString());
//  }
//
//  QMap<QString, QVariant> msg;
//  msg.insert("Notify", this->self->index);
//  map.insert("SenderPort", this->self->port);
//  map.insert("SenderHostAddress", this->self->hostAddress.toString());
//  emit sendDHTMessage(map, this->self->successor->port, this->self->successor->hostAddress);
//}
//
//void DHTManager::notify (QMap<QString, QVariant> map) 
//{
//  int idx = map["Notify"].toUInt();
//
//  if (this->self->predecessor == NULL) {
//
//    Node *node = new Node(idx, map["NotifyPort"].toUInt(), 
//      QHostAddress(map["NotifyHostAddress"].toString()));
//  } 
//
//  else if (idx > this->self->predecessor->index && idx < this->self->index) {
//
//    this->self->predecessor->index = idx;
//    this->self->predecessor->port = map["SenderPort"].toUInt();
//    this->self->predecessor->hostAddress = QHostAddress(map["SenderHostAddress"].toString());
//  }
//}
//
//void DHTManage::sendCurrentPredecessor(Peer *peer) 
//{
//  QMap<QString, QVariant> map;
//  map.insert("StoredPredecessorResponse", this->self->predecessor->index);
//  map.insert("StoredPredecessorPort", this->self->predecessor->port);
//  map.insert("StoredPredecessorHostAddress", this->self->predecessor->hostAddress.toString())
//
//  emit sendDHTMessage(map, peer->port, peer->hostAddress.toString())
//}
//
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

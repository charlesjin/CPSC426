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
  self->predecessor = new Node(self->index, self->port, QHostAddress());
  this->originID = originID;
  sizeDHT = 128;
  
  next = 0;

  stabilizeTimer = new QTimer(this);
  connect(stabilizeTimer, SIGNAL(timeout()), 
    this, SLOT(stabilizeBegin()));
  stabilizeTimer->start(7000);

  checkPredecessorTimer = new QTimer(this);
  connect(checkPredecessorTimer, SIGNAL(timeout()), 
    this, SLOT(checkPredecessor()));
  checkPredecessorTimer->start(7000);

  failureTimer = new QTimer(this);
  failureTimer->setSingleShot(true);
  connect(failureTimer, SIGNAL(timeout()), 
    this, SLOT(predecessorFailed()));
    
  fixFingersTimer = new QTimer(this);
  connect(fixFingersTimer, SIGNAL(timeout()),
    this, SLOT(fixFingers()));
  fixFingersTimer->start(7000);
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

    // qDebug() << "Sending DHT Message back" << newMap;

    emit sendDHTMessage(newMap, map["RequestPort"].toUInt(), QHostAddress(map["RequestHostAddress"].toString()));
  }
}

Node *DHTManager::findSuccessor(int index, Peer *peer, QString peerOriginID)
{
  index = index % sizeDHT;
  if (index == this->self->index || inRange(index, this->self->index, this->self->successor->index)) {
    return this->self->successor;
  } else {
    Node *nn = closestPrecedingNode(index);
    if (nn->index == this->self->index || nn->hostAddress.isNull()) {
      return this->self;
    }
    // Ask nn to find the successor of index and return that node
    if (nn->port != peer->port || nn->hostAddress != peer->hostAddress)
      askForSuccessor(nn, index, peer, peerOriginID);
    else
      return this->self;

    qDebug() << " FCUKCUCKCUCKCUCKC " << index;
    return NULL;
  }
}

Node *DHTManager::closestPrecedingNode(int index)
{
  index = index % sizeDHT;
  for (int i = finger.size() - 1; i >= 0; i--) {
    if (inRange(finger[i].node->index, this->self->index, index)){
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

  qDebug() << "ASK FOR SUCCESSOR" << map;

  if (nn == NULL) return;
  if (nn->hostAddress.isNull()) return;
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
    entry.node = new Node(this->self->index, 0, QHostAddress());
    finger << entry;
  }
  this->self->predecessor = new Node(this->self->index, 0, QHostAddress());
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
    entry.node = new Node( (entry.start - 1) % sizeDHT, 0, QHostAddress());
    finger << entry;
  }

  fingerTableUpdated();
  updateFingerTableWithNewNode(map["Predecessor"].toInt(), peer);

  // update successor
  QMap<QString, QVariant> updateMap;
  updateMap.insert("updateIndex", this->self->index);
  updateMap.insert("updatePort", this->self->port);
  updateMap.insert("updateHostAddress", this->self->hostAddress.toString());
  updateMap.insert("newPredecessor", this->originID);
  emit sendDHTMessage(updateMap, this->self->successor->port, this->self->successor->hostAddress);

  finger[0].node = new Node(successor->index, successor->port, successor->hostAddress);
  for (i = 1; i < finger.size(); i++){
    // if (finger[i].start == self->index || inRange(finger[i].start, self->index, finger[i-1].node->index)){
    //   finger[i].node->index = finger[i-1].node->index;
    //   finger[i].node->port = finger[i-1].node->port;
    //   finger[i].node->hostAddress = finger[i-1].node->hostAddress;
    //   qDebug() << "GOT SUCCESSOR" << i << finger[i].node->index;
    // } else {
    //   qDebug() << "FIND SUCCESSOR";
      Node *node = this->findSuccessor(finger[i].start, peer, originID);
      if (node){
        qDebug() << i << node->index;
        finger[i].node->index = node->index;
        finger[i].node->port = node->port;
        finger[i].node->hostAddress = node->hostAddress;
      }
    // }
  }
  fingerTableUpdated();

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
    if (index == this->self->index)
      continue;
    map.insert("updateNodeIndex", this->self->index);
    map.insert("updateIndex", index);
    map.insert("updateFingerIndex", i);
    this->updatePredecessor(map);
  }
}

void DHTManager::updatePredecessor(QVariantMap map)
{
  int index = map["updateIndex"].toUInt();
  index = index % sizeDHT;
  Node *nn = closestPrecedingNode(index);
  if (nn->index == this->self->index){
    Node *node = finger[map["updateFingerIndex"].toUInt()].node;
    if (node == NULL) {
      node = new Node(0, 0, QHostAddress());
    }
    node->index = map["updateNodeIndex"].toUInt();
    node->port = map["updatePort"].toUInt();
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

  if (diff == 0)
    return;

  int fingerIdx = floor(log2(diff));

  finger[fingerIdx].node->index = map["SuccessorResponse"].toUInt();
  finger[fingerIdx].node->port = map["SuccessorPort"].toUInt();
  finger[fingerIdx].node->hostAddress = QHostAddress(map["SuccessorHostAddress"].toString());
  fingerTableUpdated();
}


void DHTManager::newPredecessor(QMap<QString, QVariant> map)
{
  // need to account for this->self->predecessor->hostAddress is null?
  Node *node = this->self->predecessor;
  node->index = map["updateIndex"].toUInt();
  node->port = map["updatePort"].toUInt();
  node->hostAddress = QHostAddress(map["updateHostAddress"].toString());
}

bool DHTManager::betweenInterval(int begin, int end, int x)
{
  if (begin < end)
    return (x > begin && x < end);
  else if (begin > end)
    return (x > begin || x < end);
  else
    return false;
}

void DHTManager::stabilizeBegin() 
{
  if (!isInDHT()) return;
  
  // ask for successor's predecessor
  QMap<QString, QVariant> map;
  map.insert("StoredPredecessorRequest", this->self->index);
  
  emit sendDHTMessage(map, this->self->successor->port, this->self->successor->hostAddress);
}

void DHTManager::updateFingerTableWithNewNode(int peerIndex, Peer *peer)
{

  qDebug() << peerIndex;
  int i = 0;
  for (i = 0; i < finger.size(); i++){
    if (inRange(peerIndex, finger[i].start, finger[i].node->index)){
      finger[i].node->index = peerIndex;
      finger[i].node->hostAddress = peer->hostAddress;
      finger[i].node->port = peer->port;
    }
  }

  fingerTableUpdated();
}

void DHTManager::fingerTableUpdated()
{
  QList<QPair<int, int> > table;
  for (int i = 0; i < finger.size(); i++) {
    table << qMakePair(finger[i].start, finger[i].node->index);
  }
  emit fingerTableUpdatedSignal(table);
}

void DHTManager::notify(QMap<QString, QVariant>map)
{
  int idx = map["Notify"].toUInt();

  if (this->self->predecessor->hostAddress.isNull() || 
      betweenInterval(this->self->predecessor->index, this->self->index, idx)) {
    qDebug() << "updated existing predecessor";
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
  map.insert("StoredPredecessorHostAddress", this->self->predecessor->hostAddress.toString());

  emit sendDHTMessage(map, peer->port, peer->hostAddress);
}

void DHTManager::checkPredecessor() 
{
  if (!isInDHT()) return;
  
  QMap<QString, QVariant> map;
  map.insert("HeartbeatRequest", 0);
  emit sendDHTMessage(map, this->self->predecessor->port, this->self->predecessor->hostAddress);

  qDebug() << this->self->port << " sent heartbeat request to " << this->self->predecessor->port;
  failureTimer->start(3000);
}

void DHTManager::receivedHeartbeat() 
{
  qDebug() << "received heartbeat";
  failureTimer->stop();
}

void DHTManager::predecessorFailed() 
{
  this->self->predecessor->hostAddress = QHostAddress();
  qDebug() << "predecessor has failed";
}

void DHTManager::fixFingers()
{ 
  if (!isInDHT()) return;
  
  next = next + 1;
  if (next > 7)
    next = 0;
  
  // update finger[next] = find_successor(n+2^
  //
  //  call
  //  Node *DHTManager::findSuccessor(int index, Peer *peer, QString peerOriginID)
  // if the response is null, then we're done because it will call updatefingertable
  // otherwise, we need to manually update the finger table entry because it will return a node
  // if node x != NULL, then finger[next].node = x
  
  
  // findSuccessor(finger[i].start, who sent the message, origin;
  // do something like finger[i].node = find_successor(finger[i].start);
  
}

// void DHTManager::newPredecessor(QMap<QString, QVariant> map)
// {
//   Node *node = this->self->predecessor;
//   node->index = map["updateIndex"].toUInt();
//   node->port = map["updatePort"].toUInt();
//   node->hostAddress = QHostAddress(map["updateHostAddress"].toString());
// }

bool DHTManager::inRange(int i, int start, int end)
{
  if (i > start && i < end && start < end)
    return true;

  if (start > end && (i > start || i < end))
    return true;

  return false;
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
void DHTManager::stabilize(QMap<QString, QVariant> map)
{
   int idx = map["StoredPredecessorResponse"].toInt();

  if (!map["StoredPredecessorHostAddress"].isNull() 
       && betweenInterval(this->self->index, this->self->successor->index, idx)) {
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

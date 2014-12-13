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

int DHTManager::getSize()
{
  return sizeDHT;
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
    if (nn->index == this->self->index) {
      qDebug() << "I am the successor" << this->self->index << this->self->successor->index;
      qDebug() << "";
      this->self->predecessor = new Node(map["SuccessorRequest"].toInt() - 1,
          map["RequestPort"].toUInt(), QHostAddress(map["RequestHostAddress"].toString()));
    }

    QVariantMap newMap;
    newMap.insert("SuccessorResponse", nn->index);
    newMap.insert("SuccessorPort", nn->port);
    newMap.insert("SuccessorHostAddress", nn->hostAddress.toString());
    newMap.insert("Predecessor", this->self->index);
    newMap.insert("PredecessorPort", this->self->port);
    newMap.insert("PredecessorHostAddress", this->self->hostAddress.toString());
    newMap.insert("Dest", map["Origin"].toString());
    newMap.insert("Index", map["SuccessorRequest"].toInt());
    newMap.insert("FingerEntryNum", map["FingerEntryNum"].toInt());

    // qDebug() << "Sending DHT Message back" << newMap;

    emit sendDHTMessage(newMap, map["RequestPort"].toUInt(), QHostAddress(map["RequestHostAddress"].toString()));
  }
}

Node *DHTManager::findSuccessor(int index, Peer *peer, QString peerOriginID, int fingerEntryNum)
{
  qDebug() << "FIND SUCCESSOR" << index;
  qDebug() << "";
  index = index % sizeDHT;
  int me = index;
  int low = this->self->index;
  int high = this->self->successor->index;
  if (me == low || inRange(me, low, high)) {
    return this->self->successor;
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
    if (inRange(me, low, high)){
      return finger[i].node;
    }
  }
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

  if (nn == NULL) return;
  if (nn->hostAddress.isNull()) return;
  emit sendDHTMessage(map, nn->port, nn->hostAddress);
}

bool DHTManager::isInDHT()
{
  return (finger.size() > 0);
}

// This is the first node in the network
void DHTManager::initializeDHT(int peerIndex, quint16 port, QHostAddress hostAddress)
{
  // Initialize the DHT
  for (int i = 0; i < (int) ceil(log2(this->sizeDHT)); i++) {
    FingerEntry entry;
    entry.start = (self->index + (int) pow(2, i)) % sizeDHT;
    entry.node = new Node(this->self->index, 0, QHostAddress());
    finger << entry;
  }
  this->self->predecessor = new Node(peerIndex, port, hostAddress);
  this->self->successor = new Node(peerIndex, port, hostAddress);
      //new Node(this->self->index, this->self->port, QHostAddress());
      //this->self->hostAddress);
  // this->self->predecessor = new Node(this->self->index, 0, QHostAddress());
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
    entry.node = new Node(0, peer->port, peer->hostAddress);
    //entry.node = new Node(successor->index, successor->port, successor->hostAddress);
    //entry.node = new Node(this->self->index, this->self->port, QHostAddress());
        // this->self->hostAddress);
    // entry.node = new Node( (entry.start - 1) % sizeDHT, 0, QHostAddress());
    finger << entry;
  }

  fingerTableUpdated();
  updateFingerTableWithNewNode(map["Predecessor"].toInt(), peer);

  // update successor
//  QMap<QString, QVariant> updateMap;
//  updateMap.insert("updateIndex", this->self->index);
//  updateMap.insert("updatePort", this->self->port);
//  updateMap.insert("updateHostAddress", this->self->hostAddress.toString());
//  updateMap.insert("newPredecessor", this->originID);
//  emit sendDHTMessage(updateMap, this->self->successor->port, this->self->successor->hostAddress);

  finger[0].node = new Node(successor->index, successor->port, successor->hostAddress);
  initFingerTableHelper(1, peer);
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
    if (me == low || inRange(me, low, high)) {
      qDebug() << "YES" << iterNum;
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
//      this->findSuccessor(finger[i].start, p, originID);
    }

    // =======
//   for (i = 1; i < finger.size(); i++){
//     // if (finger[i].start == self->index || inRange(finger[i].start, self->index, finger[i-1].node->index)){
//     //   finger[i].node->index = finger[i-1].node->index;
//     //   finger[i].node->port = finger[i-1].node->port;
//     //   finger[i].node->hostAddress = finger[i-1].node->hostAddress;
//     //   qDebug() << "GOT SUCCESSOR" << i << finger[i].node->index;
//     // } else {
//     //   qDebug() << "FIND SUCCESSOR";
//       Node *node = this->findSuccessor(finger[i].start, peer, originID);
//       if (node){
//         qDebug() << i << node->index;
//         finger[i].node->index = node->index;
//         finger[i].node->port = node->port;
//         finger[i].node->hostAddress = node->hostAddress;
//       }
//     // }
//   }
//   fingerTableUpdated();
// >>>>>>> stabilize
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
    if (me == low || inRange(me, low, high)) {
      node->index = map["updateValue"].toUInt();
      node->port = map["updatePort"].toUInt();
      node->hostAddress = QHostAddress(map["updateHostAddress"].toString());
      if (map["updateFingerIndex"].toInt() == 0) {
        qDebug() << "Successor updated";
        this->self->successor->index = map["updateValue"].toUInt();
        this->self->successor->port = map["updatePort"].toUInt();
        this->self->successor->hostAddress = QHostAddress(map["updateHostAddress"].toString());
      }
      fingerTableUpdated();
    } else {
      qDebug() << "No update necessary" << me << low << high;
      qDebug() << "";
    }
    return;
  }
  emit sendDHTMessage(map, nn->port, nn->hostAddress);
}

void DHTManager::updateFingerTable(QMap<QString, QVariant> map)
{
  int diff = map["Index"].toInt() - self->index;
  if (diff < 0)
    diff += sizeDHT;
  diff = diff % sizeDHT;
  if (diff == 0) {
    qDebug() << "THE DIFF IS ZERO??" << map;
    qDebug() << "";
    return;
  }
  int fingerIdx = log2(diff);

  qDebug() << "FINGER INDEX" << fingerIdx << "" << diff;
  qDebug() << "";

  finger[fingerIdx].node->index = map["SuccessorResponse"].toUInt();
  finger[fingerIdx].node->port = map["SuccessorPort"].toUInt();
  finger[fingerIdx].node->hostAddress = QHostAddress(map["SuccessorHostAddress"].toString());
  if (map["FingerEntryNum"].toInt() >= 0) {
    initFingerTableHelper(map);
  } else {
    fingerTableUpdated();
  }
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
  updateOthers();
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

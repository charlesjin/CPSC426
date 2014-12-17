#include <math.h>
#include "dht.hh"

void DHTManager::initTimers() 
{
  next = 0;

  successorList = new QHash<int, Node*>();
  
  stabilizeTimer = new QTimer(this);
  connect(stabilizeTimer, SIGNAL(timeout()), 
    this, SLOT(stabilizeBegin()));
  stabilizeTimer->start(5000); 
  
  stabilizeFailureTimer = new QTimer(this);
  stabilizeFailureTimer->setSingleShot(true);
  connect(stabilizeFailureTimer, SIGNAL(timeout()),
    this, SLOT(stabilizeFailed()));

  checkPredecessorTimer = new QTimer(this);
  connect(checkPredecessorTimer, SIGNAL(timeout()), 
    this, SLOT(checkPredecessor()));
  checkPredecessorTimer->start(5000);

  failureTimer = new QTimer(this);
  failureTimer->setSingleShot(true);
  connect(failureTimer, SIGNAL(timeout()), 
    this, SLOT(predecessorFailed())); 
    
  fixFingersTimer = new QTimer(this);
  connect(fixFingersTimer, SIGNAL(timeout()),
    this, SLOT(fixFingers()));
  fixFingersTimer->start(4000);
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
  
  if (!successorList->isEmpty()) {
  
    QHash<int, Node*>::const_iterator i = successorList->constBegin();
    int newidx = i.key();
    while (i != successorList->constEnd()) {
      if (betweenInterval(this->self->index, newidx, i.key()) 
            || this->self->index == newidx) {
        newidx = i.key();
      }
      ++i;
    }
  
    Node* n = successorList->value(newidx);
  
    qDebug() << "new successor " << n->index;
    this->self->successor->index = n->index;
    this->self->successor->hostAddress = n->hostAddress;
    this->self->successor->port = n->port;
  }

  
  emit sendDHTMessage(map, this->self->successor->port, this->self->successor->hostAddress);
  
  stabilizeFailureTimer->start(3000);
}

void DHTManager::stabilizeFailed() 
{
  qDebug() << "no response from successor";

  int idx = this->self->successor->index;
  successorList->remove(idx);
  
  if (successorList->isEmpty()) {
    this->self->successor = this->self;
    return;
  }
  
  QHash<int, Node*>::const_iterator i = successorList->constBegin();
  int newidx = i.key();
  while (i != successorList->constEnd()) {
    if (betweenInterval(this->self->index, newidx, i.key())
          || this->self->index == newidx) {
      newidx = i.key();
    }
    ++i;
  }
  
  Node* n = successorList->value(newidx);
  
  qDebug() << "new successor " << n->index;
  this->self->successor->index = n->index;
  this->self->successor->hostAddress = n->hostAddress;
  this->self->successor->port = n->port;
  
}

void DHTManager::sendCurrentPredecessor(Peer *peer) 
{
 QMap<QString, QVariant> map;
  map.insert("StoredPredecessorResponse", this->self->predecessor->index);
  map.insert("StoredPredecessorPort", this->self->predecessor->port);
  map.insert("StoredPredecessorHostAddress", 
    this->self->predecessor->hostAddress.toString());

  emit sendDHTMessage(map, peer->port, peer->hostAddress);
}

void DHTManager::notify(QMap<QString, QVariant>map)
{
  int idx = map["Notify"].toUInt();
  qDebug() << "got NOTIFY MESSAGE";
  
  if (this->self->predecessor->hostAddress.isNull() 
        || this->self->predecessor->index == this->self->index
        || betweenInterval(this->self->predecessor->index, this->self->index, idx)) {
    qDebug() << "updated existing predecessor to" << idx;
    this->self->predecessor->index = idx;
    this->self->predecessor->port = map["SenderPort"].toUInt();
    this->self->predecessor->hostAddress = QHostAddress(map["SenderHostAddress"].toString());
  }
}

void DHTManager::checkPredecessor() 
{
  if (!isInDHT() || this->self->predecessor->hostAddress.isNull()) return;
  
  QMap<QString, QVariant> map;
  map.insert("HeartbeatRequest", 0);
  emit sendDHTMessage(map, this->self->predecessor->port, 
    this->self->predecessor->hostAddress);

  qDebug() << this->self->port << " sent heartbeat request to " 
    << this->self->predecessor->port;
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
  qDebug() << "predecessor " << this->self->predecessor->index << " has failed";
}

void DHTManager::fixFingers()
{ 
  if (!isInDHT()) return;
  
  qDebug() << *successorList;
  qDebug() << "current predecessor " << this->self->predecessor->index;
  qDebug() << "current successor is " << this->self->successor->index;
    
  next = next + 1;
  if (next > 6)
    next = 0;

  qDebug() << "running fixFingers on entry " << next;
  
  Peer *p = new Peer();
  p->port = this->self->successor->port;
  p->hostAddress = this->self->successor->hostAddress;
  
  //Node *n = this->findSuccessor(finger[next].start, p, originID, next);
 
  //if (n != NULL){
  //  finger[next].node->index = n->index;
  //  finger[next].node->port = n->port;
  //  finger[next].node->hostAddress = n->hostAddress;
  //}

//  for (next = 0; next < 7; next++) {
    Peer *np = new Peer();
    np->port = this->self->port;
    np->hostAddress = this->self->hostAddress;
    this->askForSuccessor(this->self->predecessor, finger[next].start, np, originID,   next); 
//  }
  
  //fingerTableUpdated();
}

// void DHTManager::newPredecessor(QMap<QString, QVariant> map)
// {
//   Node *node = this->self->predecessor;
//   node->index = map["updateIndex"].toUInt();
//   node->port = map["updatePort"].toUInt();
//   node->hostAddress = QHostAddress(map["updateHostAddress"].toString());
// }

void DHTManager::stabilize(QMap<QString, QVariant> map)
{
  stabilizeFailureTimer->stop();

   int idx = map["StoredPredecessorResponse"].toInt();

  if (!map["StoredPredecessorHostAddress"].isNull() 
       && betweenInterval(this->self->index, this->self->successor->index, idx)) {
    this->self->successor->index = idx;
    this->self->successor->port = map["StoredPredecessorPort"].toUInt();
    this->self->successor->hostAddress = 
      QHostAddress(map["StoredPredecessorHostAddress"].toString());
  }

   QMap<QString, QVariant> msg;
   msg.insert("Notify", this->self->index);
   msg.insert("SenderPort", this->self->port);
   msg.insert("SenderHostAddress", this->self->hostAddress.toString());
   emit sendDHTMessage(msg, this->self->successor->port, 
     this->self->successor->hostAddress);
}

void DHTManager::initSuccessorList() 
{
  Node *e = new Node(this->self->successor->index, 
    this->self->successor->port, this->self->successor->hostAddress);
  successorList->insert(this->self->successor->index, e);
  
  Node *f = new Node(this->self->predecessor->index, 
    this->self->predecessor->port, this->self->predecessor->hostAddress);
  successorList->insert(this->self->predecessor->index, f);
}

void DHTManager::updateSuccessorList(QVariantMap map)
{
  Node* e = new Node(map["updateValue"].toUInt(),
    map["updatePort"].toUInt(), 
    QHostAddress(map["updateHostAddress"].toString()));
  successorList->insert(map["updateValue"].toUInt(), e);
}

void DHTManager::updateSuccessorListFT(QVariantMap map)
{
  Node* e = new Node(map["SuccessorResponse"].toUInt(),
    map["SuccessorPort"].toUInt(), 
    QHostAddress(map["SuccessorHostAddress"].toString()));
  successorList->insert(map["SuccessorResponse"].toUInt(), e); 
}


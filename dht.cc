// dht.cc by Kayo Teramoto
#include <math.h>
#include "dht.hh"

/*****************************/
/*                           */
/*            DHT            */
/*                           */
/*****************************/

DHTManager::DHTManager(quint16 port, QHostAddress hostAddress)
{
  self = new Node(-1, port, hostAddress);
  self->successor = self;
  self->predecessor = self;
  sizeDHT = 128;
}

int DHTManager::getIndex(Node *nn)
{
  return nn->index;
}

Node *DHTManager::findSuccessor(int id)
{
  Node *nn = findPredecessor(id);
  return nn->successor;
}

Node *DHTManager::findPredecessor(int id)
{
  Node *nn = this->self;
  // while (id <= getIndex(nn) || id > getIndex(nn->successor)) {
  //   // nn = nn->closestPrecedingFinger(id);
  // }
  return nn;
}

Node *DHTManager::closestPrecedingFinger(int id)
{
  // for (int i = finger.size() - 1; i >= 0; i--) {
  //   if (getIndex(finger[i].node) > getIndex(this) && getIndex(finger[i].node) < id) {
  //     return finger[i].node;
  //   }
  // }
  // return this;
  return NULL;
}

void DHTManager::join(Node *nn, Peer *peer)
{
  if (nn != NULL) {
    initFingerTable(nn, peer);
    updateOthers();
  } else {
    // This is the first node in the network
    // Initialize the DHT
    for (int i = 0; i < (int) ceil(log2(this->sizeDHT)); i++) {
      FingerEntry entry;
      entry.start = index + pow(2, i);
      entry.node = this->self;
      finger << entry;
    }
    predecessor = this->self;
  }
}

void DHTManager::join(Peer *peer)
{
  if (peer){
    // created a node for the peer
  } else {
    return;
  }
}

// CALLED DURING INIT
// wrapper for DHTManager::initFingerTable(Node *nn, Peer *peer)
void DHTManager::initFingerTable(QMap<QString, QVariant> map, Peer *peer)
{
  Node *predecessor = new Node(map["PredIndex"].toInt(), 0, QHostAddress());
  Node *node = new Node(map["Index"].toInt(), peer->port, peer->hostAddress);
  node->predecessor = predecessor;
  node->successor = NULL;
  this->initFingerTable(node, peer);
}

void DHTManager::initFingerTable(Node *nn, Peer *peer)
{
  int i = 0;
  FingerEntry entry;
  entry.node = NULL;
  entry.start = 0;

  for (i = 0; i < (int) ceil(log2(this->sizeDHT)); i++){
    entry.start = index + pow(2, i);
    finger << entry;
  }

  finger[0].node = nn;
  predecessor = nn->predecessor;
  nn->predecessor = this->self; // TODO: send as message
 
  for (i = 1; i < (int) ceil(log2(this->sizeDHT)); i++){
    if (finger[i].start >= self->index && finger[i].start < finger[i-1].start){
      finger[i].node = finger[i-1].node;
    } else {
      emit findSuccessor(i, peer);
    }
  }
}

// WAITING FOR RESPONSE DURING INIT
// wrapper for DHTManager::initFingerTable(Node *nn, int i)
void DHTManager::initFingerTable(QMap<QString, QVariant> map, Peer *peer, int i)
{
  Node *node = new Node(map["Index"].toInt(), peer->port, peer->hostAddress);
  node->predecessor = NULL;
  node->successor = NULL;
  this->initFingerTable(node, i);
}

void DHTManager::initFingerTable(Node *nn, int i)
{
  finger[i].node = nn;
}

void DHTManager::updateOthers()
{
  // for (int i = 0; i < finger.size(); i ++) {
  //   Node *p = findPredecessor(getIndex(this) - pow(2, i - 1));
  //   p->updateFingerTable(this, i);
  // }
}

void DHTManager::updateFingerTable(Node *nn, int i)
{
  // if (getIndex(nn) >= getIndex(this) &&
  //     getIndex(nn) < getIndex(finger[i].node)) {
  //   finger[i].node = nn;
  //   Node *p = predecessor;
  //   p->updateFingerTable(nn, i);
  // }
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

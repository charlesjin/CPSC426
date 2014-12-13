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
  self = new Node((int) (qrand() % 128), port, hostAddress);
  self->successor = self;
  self->predecessor = self;
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
    QVariantMap map;
    map.insert("SuccessorResponse", nn->index);
    map.insert("SuccessorPort", nn->port);
    map.insert("SuccessorHostAddress", nn->hostAddress.toString());
    map.insert("RequestPort", map["RequestPort"].toUInt());
    map.insert("RequestHostAddress", map["RequestHostAddress"].toString());
    map.insert("Predecessor", this->self->index);
    map.insert("PredecessorPort", this->self->port);
    map.insert("PredecessorHostAddress", this->self->hostAddress.toString());
    map.insert("Dest",map["Origin"].toString());

    emit sendDHTMessage(map, peer->port, peer->hostAddress);
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
      entry.start = self->index + pow(2, i);
      entry.node = this->self;
      finger << entry;
    }
    this->self->predecessor = this->self;
  }
}

void DHTManager::join(Peer *peer)
{
  // Create a node for the peer
  Node *nn = new Node(-1, peer->port, QHostAddress(peer->hostAddress));
}

// CALLED DURING INIT
// wrapper for DHTManager::initFingerTable(Node *nn, Peer *peer)
void DHTManager::initFingerTable(QMap<QString, QVariant> map, Peer *peer)
{
  Node *predecessor = new Node(map["PredIndex"].toInt(), 0, QHostAddress());
  Node *node = new Node(map["SuccIndex"].toInt(), map["SuccPort"].toUInt(),
      QHostAddress(map["SuccHostAddress"].toString()));
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
    entry.start = self->index + pow(2, i);
    finger << entry;
  }

  finger[0].node = nn;
  this->self->predecessor = nn->predecessor;
  nn->predecessor = this->self; // TODO: send as message
 
  for (i = 1; i < (int) ceil(log2(this->sizeDHT)); i++){
    if (finger[i].start >= self->index && finger[i].start < finger[i-1].start){
      finger[i].node = finger[i-1].node;
    } else {
    //  emit findSuccessor(i, peer);
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

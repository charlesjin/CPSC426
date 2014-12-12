// dht.cc by Kayo Teramoto
#include <math.h>

#include "dht.hh"

DHTManager::DHTManager() {
  sizeDHT = 128;
}

int DHTManager::getIndex(Node *nn) {
  return nn->index;
}

Node *DHTManager::findSuccessor(int id) {
  Node *nn = findPredecessor(id);
  return nn->successor;
}

Node *DHTManager::findPredecessor(int id) {
  Node *nn = this;
  while (id <= getIndex(nn) || id > getIndex(nn->successor)) {
    nn = nn->closestPrecedingFinger(id);
  }
  return nn;
}

Node *DHTManager::closestPrecedingFinger(int id) {
  for (int i = finger.size() - 1; i >= 0; i--) {
    if (getIndex(finger[i].node) > getIndex(this) && getIndex(finger[i].node) < id) {
      return finger[i].node;
    }
  }
  return this;
}

void DHTManager::join(Node *nn) {
  if (nn != NULL) {
    initFingerTable(nn);
    updateOthers();
  } else {
    // This is the first node in the network
    // Initialize the DHT
    for (int i = 0; i < finger.size(); i++) {
      finger[i].node = this;
    }
    predecessor = this;
  }
}

void DHTManager::initFingerTable(Node *nn) {
  finger[1].node = nn->findSuccessor(getIndex(finger[1].start));
  predecessor = successor->predecessor;
  successor->predecessor = this;
  for (int i = 0; i < finger.size() - 1; i++) {
    if (getIndex(finger[i + 1].start) >= getIndex(this) &&
        getIndex(finger[i + 1].start) < getIndex(finger[i].node)) {
      finger[i + 1].node = finger[i].node;
    } else {
      finger[i + 1].node =
          nn->findSuccessor(getIndex(finger[i + 1].start));
    }
  }
}

void DHTManager::updateOthers() {
  for (int i = 0; i < finger.size(); i ++) {
    Node *p = findPredecessor(getIndex(this) - pow(2, i - 1));
    p->updateFingerTable(this, i);
  }
}

void DHTManager::updateFingerTable(Node *nn, int i) {
  if (getIndex(nn) >= getIndex(this) &&
      getIndex(nn) < getIndex(finger[i].node)) {
    finger[i].node = nn;
    Node *p = predecessor;
    p->updateFingerTable(nn, i);
  }
}



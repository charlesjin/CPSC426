// main dht.cc
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
  self->successor = new Node(self->index, self->port, QHostAddress::LocalHost);//QHostAddress());
  self->predecessor = new Node(self->index, self->port, QHostAddress::LocalHost);
  this->originID = originID;
  sizeDHT = 128;
  initTimers();
}

int DHTManager::getIndex()
{
  return self->index;
}

bool DHTManager::isInDHT()
{
  return (finger.size() > 0);
}

bool DHTManager::inRange(int i, int start, int end)
{
  if (i > start && i < end && start < end)
    return true;

  if (start > end && (i > start || i < end))
    return true;

  return false;
}

Node::Node(int index, quint16 port, QHostAddress hostAddress)
{
  this->index = index;
  this->port = port;
  this->hostAddress = hostAddress;
  this->predecessor = NULL;
  this->successor = NULL;
}


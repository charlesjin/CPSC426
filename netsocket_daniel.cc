#include <unistd.h>
#include <stdlib.h>
#include <string>
#include <ctime>

#include <QVariant>
#include <QDataStream>
#include <QTimer>
#include <QtCore>
#include <QFuture>
#include <QCoreApplication>

#include "netsocket.hh"
#include "dht.hh"

void NetSocket::stabilizeInitializer(DHTManager* dHTManager) 
{
  connect(this, SIGNAL(sendCurrentPredecessor(Peer*)),
    dHTManager, SLOT(sendCurrentPredecessor(Peer*)));
  connect(this, SIGNAL(stabilize(QMap<QString, QVariant>)),
    dHTManager, SLOT(stabilize(QMap<QString, QVariant>)));
  connect(this, SIGNAL(notify(QMap<QString, QVariant>)),
    dHTManager, SLOT(notify(QMap<QString, QVariant>)));
  connect(this, SIGNAL(receivedHeartbeat()),
    dHTManager, SLOT(receivedHeartbeat()));
}

void NetSocket::storedPredecessorResponseReciever (QMap<QString, QVariant> map)
{
  emit stabilize(map);
}

void NetSocket::notifyReciever(QMap<QString, QVariant> map) 
{
  emit notify(map);
}

void NetSocket::heartbeatRequestReciever(Peer* peer) 
{
  QMap<QString, QVariant> map;
  map.insert("HeartbeatReply", 0);
  sendDHTMessage(map, peer->port, peer->hostAddress);
}

void NetSocket::heartbeatReplyReciever() 
{
  emit receivedHeartbeat();
}

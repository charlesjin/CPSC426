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

void NetSocket::joinDHT(Peer *peer)
{
  // Send direct message to peer asking if it has DHT
  QVariantMap map;
  map.insert("JoinDHTRequest", originID);
  map.insert("Index", dHTManager->getIndex());

  QByteArray message;
  QDataStream * stream = new QDataStream(&message, QIODevice::WriteOnly);
  (*stream) << map;
  delete stream;

  this->writeDatagram (message.data(), message.size(), peer->hostAddress, peer->port);
}

void NetSocket::joinDHTReciever(QMap<QString, QVariant> map, Peer *peer)
{
  QString peerOriginID = map["JoinDHTRequest"].toString();
  int peerIndex = map["Index"].toInt();

  // Base case: you are in the DHT
  // Send back the successor of peerIndex + 1 and the predecessor of that
  // successor
  if (dHTManager->isInDHT()) {
    QVariantMap newMap;
    newMap.insert("SuccessorRequest", peerIndex + 1);
    newMap.insert("Origin", peerOriginID);
    newMap.insert("RequestPort", peer->port);
    newMap.insert("RequestHostAddress", peer->hostAddress.toString());
    newMap.insert("FingerEntryNum", 0);
    emit successorRequest(newMap, peer);
    return;
  }

  // Other case: you are not in the DHT.
  // Compare your originID with the originID of the peer
  if (originID > peerOriginID) {
    // Initialize the DHT
    dHTManager->initializeDHT(peerIndex, peer->port, peer->hostAddress);

    // Send back the successor of peerIndex + 1 and the predecessor of that
    // successor
    QVariantMap newMap;
    newMap.insert("SuccessorRequest", peerIndex + 1);
    newMap.insert("Origin", peerOriginID);
    newMap.insert("RequestPort", peer->port);
    newMap.insert("RequestHostAddress", peer->hostAddress.toString());
    newMap.insert("FingerEntryNum", 0);
    emit successorRequest(newMap, peer);

  } else {
    // Ask the peer to initialize the DHT
    joinDHT(peer);
  }
}

void NetSocket::sendDHTMessage(QVariantMap map, quint16 port, QHostAddress hostAddress)
{
  qDebug() << "Sending DHT Message" << map;
  qDebug() << "To" << port << hostAddress;
  qDebug() << "";

  QByteArray message;
  QDataStream * stream = new QDataStream(&message, QIODevice::WriteOnly);
  (*stream) << map;
  delete stream;

  this->writeDatagram (message.data(), message.size(), hostAddress, port);
}

void NetSocket::successorRequestReciever(QMap<QString, QVariant> map, Peer* peer)
{
  qDebug() << "SReqst Received " << map;
  qDebug() << "";
  emit successorRequest(map, peer);
}

void NetSocket::updatePredecessorReciever(QMap<QString, QVariant> map)
{
  emit updatePredecessorSignal(map);
}

void NetSocket::successorResponseReciever(QMap<QString, QVariant> map, Peer* peer)
{
  qDebug() << "SResp Received " << map;
  qDebug() << "";
  if (!dHTManager->isInDHT())
    dHTManager->join(peer, map);
  else
    emit updateFingerTable(map);
}

void NetSocket::updateIndexReciever(QVariantMap map)
{
  qDebug() << "updateIndexReciever " << map;
  emit updateIndex(map);
}

void NetSocket::storedPredecessorRequestReciever(Peer* peer) 
{
  emit sendCurrentPredecessor(peer);
}

void NetSocket::fingerTableUpdated(QList<QPair<int, int> > table) {
  emit fingerTableUpdatedSignal(table);
}


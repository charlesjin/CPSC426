#ifndef PEERSTER_PEERMANAGER_HH
#define PEERSTER_PEERMANAGER_HH

#include "peer.hh"

class PeerManager : public QObject
{
  Q_OBJECT

  friend class Peer;

public:
  PeerManager(QString originID, quint16 port);
  QList<Peer*> peerPorts; /* list of pointers to peers */
  QHash<QString, QPair<QHostAddress,quint16> > routingTable; /* <originID, <IP, port> > */ 

  /* returns a random peer for rumormongering */
  Peer* randomPeer();
  /* returns either an existing peer or initiates a new peer + returns new peer */
  Peer* checkPeer(QHostAddress sender, quint16 senderPort);
  /* overload function */
  void checkPeer(quint32 sender, quint16 senderPort);
  /* checks for whether the peer is already in peerPorts */
  int checkPeerPorts(Peer *);
  /* updates the routing table */
  void updateRoutes(QString originID, Peer *peer);
  /* gets the routes in the routing table for a particular originID */
  QPair<QHostAddress,quint16> getRoutes(QString originID);
  /* updates a peer for preference if direct */

public slots:
  void newPeer(QString peer);
  void addPeer();

signals:
  void initPeer(QString host);
  void addPeerToList(QString str);
  void newPeerReady(Peer *newPeer);

private:
  QString originID;
  quint16 myPort;
  QString myHostName;
};


#endif // PEERSTER_PEERMANAGER_HH

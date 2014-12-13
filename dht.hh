// dht.hh by Kayo Teramoto

#ifndef PEERSTER_DHT_HH
#define PEERSTER_DHT_HH

#include <QHash>
#include <QList>
#include "peer.hh"

class FingerEntry;
class Node;

class FingerEntry {
  public:
    int start;
    Node *node;
};

class Node : public QObject
{
  Q_OBJECT

  public:
    Node(int index, quint16 port, QHostAddress hostAddress);

    int index;
    quint16 port;
    QHostAddress hostAddress;
    Node *predecessor;
    Node *successor;

};

class DHTManager : public QObject
{
  Q_OBJECT

  public:
    DHTManager(quint16 port, QHostAddress hostAddress);
    bool isInDHT();
    void join(Node* nn, Peer* peer);
    void join(Peer* peer);
    int getIndex();

  public slots:
    Node* findSuccessor(int index, Peer* peer, QString peerOriginID);
    /* init */
    void initFingerTable(QMap<QString, QVariant> map, Peer* peer);
    void initFingerTable(QMap<QString, QVariant> map, Peer* peer, int i);
    void successorRequest(QMap<QString, QVariant> map, Peer* peer);

  signals:
//    void findSuccessor(int i, Peer *peer);
    void sendDHTMessage(QVariantMap map, quint16 port, QHostAddress hostAddress);

  private:
    int sizeDHT;
    Node *self;

    QList<FingerEntry> finger;
    Node* closestPrecedingNode(int index);
    void askForSuccessor(Node* nn, int index, Peer* peer, QString peerOriginID);
    void initFingerTable(Node* nn);
    void updateOthers();
    void updateFingerTable(Node* nn, int i);


    /* init */
    void initFingerTable(Node* nn, Peer* peer);
    void initFingerTable(Node* nn, int i);
};

#endif // PEERSTER_DHT_HH

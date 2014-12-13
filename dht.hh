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
    DHTManager(QString originID, quint16 port, QHostAddress hostAddress);
    bool isInDHT();
    void initializeDHT();
    void join(Peer* peer, QMap<QString, QVariant> map);
    int getIndex();

  public slots:
    Node* findSuccessor(int index, Peer* peer, QString peerOriginID);
    void successorRequest(QVariantMap map, Peer* peer);
    void updateFingerTable(QMap<QString, QVariant> map);
    void updatePredecessor(QVariantMap map);
    void newPredecessor(QMap<QString, QVariant> map);
//    void stabilize(QMap<QString, QVariant> map);
//    void notify(QMap<QString, QVariant> map);
//    void sendCurrentPredecessor(Peer* peer);

  signals:
    void sendDHTMessage(QVariantMap map, quint16 port, QHostAddress hostAddress);
    void fingerTableUpdatedSignal(QList<QPair<int, int> >);

  private:
    int sizeDHT;
    Node *self;
    QString originID;

    QList<FingerEntry> finger;
    Node* closestPrecedingNode(int index);
    void askForSuccessor(Node* nn, int index, Peer* peer, QString peerOriginID);
    void initFingerTable(QMap<QString, QVariant> map, Peer* peer);
    void updateOthers();
    void fingerTableUpdated();
//=======
//    void stabilizeBegin();
//>>>>>>> 01b5185c0ed74ed80c6db0950784229bd2de07c4
};

#endif // PEERSTER_DHT_HH

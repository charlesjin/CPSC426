// dht.hh by Kayo Teramoto

#ifndef PEERSTER_DHT_HH
#define PEERSTER_DHT_HH

#include <QHash>
#include <QList>
#include <QTimer>

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
    void initializeDHT(int peerIndex, quint16 port, QHostAddress hostAddress);
    void join(Peer* peer, QMap<QString, QVariant> map);
    int getIndex();

  public slots:
    Node* findSuccessor(int index, Peer* peer, QString peerOriginID,
        int fingerEntryNum);
    void successorRequest(QVariantMap map, Peer* peer);
    void updateFingerTable(QMap<QString, QVariant> map);
    void updatePredecessor(QVariantMap map);
    void newPredecessor(QMap<QString, QVariant> map);
    void stabilizeBegin();
    void stabilize(QMap<QString, QVariant> map);
    void notify(QMap<QString, QVariant> map);
    void sendCurrentPredecessor(Peer* peer);
    void checkPredecessor();
    void receivedHeartbeat();
    void predecessorFailed();
    void fixFingers();
    void updateFingerTableWithNewNode(int peerIndex, Peer *peer);
    void updatePredecessorReciever(QMap<QString, QVariant>);

  signals:
    void sendDHTMessage(QVariantMap map, quint16 port, QHostAddress hostAddress);
    void fingerTableUpdatedSignal(QList<QPair<int, int> >);

  private:
    int sizeDHT;
    Node *self;
    QString originID;
    int next; // determines which finger table entry to refresh next
    QTimer *stabilizeTimer;
    QTimer *checkPredecessorTimer;
    QTimer *failureTimer;
    QTimer *fixFingersTimer;    

    QList<FingerEntry> finger;
    Node* closestPrecedingNode(int index);
    void askForSuccessor(Node* nn, int index, Peer* peer, QString peerOriginID, int fingerEntryNum);
    void initFingerTable(QMap<QString, QVariant> map, Peer* peer);
    void initFingerTableHelper(int iterNum, Peer* peer);
    void initFingerTableHelper(QMap<QString, QVariant> map);
    void updateOthers();
    bool betweenInterval(int begin, int end, int x);
    void fingerTableUpdated();
    bool inRange(int i, int start, int end);
};

#endif // PEERSTER_DHT_HH

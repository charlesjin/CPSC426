// dht.hh by Kayo Teramoto
#include <QHash>
#include <QList>

class FingerEntry;
class Node;

class FingerEntry {
  public:
    Node *start;
    Node *node;
};

class Node {
  public:
    void join(Node *nn);

  private:
    int val;
    Node *predecessor;
    Node *successor;
    QList<FingerEntry> finger;
    Node *findSuccessor(int id);
    Node *findPredecessor(int id);
    Node *closestPrecedingFinger(int id);
    int getIndex(Node *nn);
    void initFingerTable(Node *nn);
    void updateOthers();
    void updateFingerTable(Node *nn, int i);
};


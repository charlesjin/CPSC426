#ifndef PEERSTER_NETSOCKET_HH
#define PEERSTER_NETSOCKET_HH

#include <QUdpSocket>
#include <QMap>
#include <QVariant>

#include "peer.hh"
#include "peermanager.hh"
#include "filemanager.hh"
#include "secretmanager.hh"

class NetSocket : public QUdpSocket
{
  Q_OBJECT

  friend class Peer;

  public:
    NetSocket();
    PeerManager *peerManager;
    FileManager *fileManager;
    SecretManager *secretManager;
    bool noForward;

    /* Bind this socket to a Peerster-specific default port. */
    bool initialize();
  
    /* adds a message to message store */
    void pushMessage(QMap<QString, QVariant> map);
    /* gets a message in want map from message store */
    QMap<QString, QVariant> getMessage(QMap<QString, QVariant> want);
  
    /* message recievers */
    void directMessageReciever(QMap<QString, QVariant> map, Peer* peer);
    void blockRequestReciever(QMap<QString, QVariant> map);
    void blockReplyReciever(QMap<QString, QVariant> map);
    void searchReplyReciever(QMap<QString, QVariant> map);
    void secretShareReciever(QMap<QString, QVariant> map);
    void secretRequestReciever(QMap<QString, QVariant> map);
    void secretReplyReciever(QMap<QString, QVariant> map);
    void searchRequestReciever(QMap<QString, QVariant> map, Peer *peer);
    void chatReciever(QMap<QString, QVariant> map, Peer* peer);
    void statusReciever(QMap<QString, QVariant> map, Peer* peer);

    /* block/search requests from fileDialog */
    void blockRequestSender(QString str);
    void searchRequestInit(QString str);
    void fileRequestFromSearch(QString fileName);
  
    /* rumormongering */
    void beginRumor(QByteArray data);
    void allRumor(QByteArray data);

  public slots:
    /* message senders */
    void messageSender(QMap<QString, QVariant> map);
    void sendDirectMessage(QMap<QString, QVariant> map);
    void fileRequestSender(QMap<QString, QVariant> map);
    void sendResponse(Peer* peer);
  
    void searchRequestSender(QMap<QString, QVariant> map);
    void searchRequestSender(QMap<QString, QVariant> map, Peer* peer);
  
    /* recieves message */
    void messageReciever();
  
    /* rumormongering */
    void antiEntropy();
    void resendRumor();
    void sendRouteRumor();
    void sendRouteRumorToPeer(Peer *peer);

    /* secret sharing */
    void sendSecret(quint32 secret);
    void recoverSecret(QString secretID);
  signals:
    void messageRecieved(QVariant chatText);
    void secretRecieved(QString secretID);
    void refreshSearchResults(QString fileName);
  void newSecretShare(QMap<QString, QVariant> map);

  private:
    int myPortMin, myPortMax, myPort;
    QString originID;
    quint32 seqNo; /* For messages */
    QMap<QString, QVariant> want; /* <originID, seqNO> */
    QMap<QString, QMap<QString, QVariant> >messages; /* <originID <message, seqNo> > */

    QPair<QString, QHash<QString, QString> > searchResponse; /* searchTerms <fileName, headerHash:peerID> */
    QList<QByteArray> searchHashIndex; /* headerHash */
    
    quint32 secretNo;
    QMap<QString, QMap<QString, QVariant> > secrets; /* <originID, <searchTerm, threshold/x/fx> > */
};

#endif // PEERSTER_NETSOCKET_HH

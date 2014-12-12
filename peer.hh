#ifndef PEERSTER_PEER_HH
#define PEERSTER_PEER_HH

#include <QUdpSocket>
#include <QMap>
#include <QVariant>
#include <QHostInfo>
#include <QTimer>

class Peer : public QObject
{
  Q_OBJECT

public:
  Peer();

  quint16 port;
  QString hostName;
  QHostAddress hostAddress;

  QTimer *timer;
  QByteArray data;

  public slots:
    void initPeer(QString host);
    void initPeer(QHostAddress address);
    void hostLookupDone(const QHostInfo &host);
    void reverseHostLookupDone(const QHostInfo &host);

  signals:
    void initPeerDone();

};

#endif // PEERSTER_PEER_HH

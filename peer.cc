/*****************************/
/*													 */
/*           Peer            */
/*													 */
/*****************************/

// peer class for maintaining a connection to a peer
// one peer is created per connection

#include "peer.hh"

// initializes peer
Peer::Peer()
{
  port = 0;
  timer = new QTimer(this);
  timer->setSingleShot(true);
  timer->setInterval(1500);
}

// finds the hostname/hostaddress
void Peer::initPeer(QString host)
{
  QHostAddress testHostAddress;
  testHostAddress =  QHostAddress( host );
  if (testHostAddress.isNull()){
    hostName = host;
    QHostInfo::lookupHost(host, this, SLOT(hostLookupDone(QHostInfo)));
  } else {
    hostAddress = testHostAddress;
    QHostInfo::lookupHost(host, this, SLOT(reverseHostLookupDone(QHostInfo)));
  }
}

// overload function for when we definitely know that the peer has an IP address
void Peer::initPeer(QHostAddress address)
{
  QHostInfo::lookupHost(address.toString(), this, SLOT(reverseHostLookupDone(QHostInfo)));
}

// hostName callback
void Peer::hostLookupDone(const QHostInfo &host)
{
  if (host.error() != QHostInfo::NoError) {
    qDebug() << "Lookup failed:" << host.errorString();
  }

  if (!host.addresses().isEmpty()) {
    hostAddress = host.addresses().first();
    emit initPeerDone();
    return;
  }

  delete this->timer;
  delete this;
  return;
}

// hostAddress callback
void Peer::reverseHostLookupDone(const QHostInfo &host)
{
  if (host.error() != QHostInfo::NoError) {
    qDebug() << "Lookup failed:" << host.errorString();
    delete this->timer;
    delete this;
    return;
  }

  hostName = host.hostName();
  emit initPeerDone();
}

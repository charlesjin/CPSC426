/*****************************/
/*													 */
/*       Peer Manager        */
/*													 */
/*****************************/

#include <QPair>

#include "peermanager.hh"

PeerManager::PeerManager(QString origin, quint16 port)
{
	originID = origin;
	myPort = port;
	myHostName = QHostInfo::localHostName();
}

// called whenever a new message is recieved
Peer* PeerManager::checkPeer(QHostAddress sender, quint16 senderPort)
{
	// check if host is already saved
 	Peer *newPeer = new Peer();
	newPeer->port = senderPort;
	newPeer->hostAddress = sender;

	int i = this->checkPeerPorts(newPeer);
 	if (i >= -1){
 		delete newPeer->timer;
 		delete newPeer;
		return peerPorts.at(i);
	} else {
		connect(newPeer, SIGNAL(initPeerDone()), this, SLOT(addPeer()));
		newPeer->initPeer(sender);
		return newPeer;
	}
}

void PeerManager::checkPeer(quint32 sender, quint16 senderPort)
{
	QHostAddress hostAddress;
	hostAddress =  QHostAddress( sender );
	if (!hostAddress.isNull()){
		this->checkPeer(hostAddress, senderPort);
	}
}

int PeerManager::checkPeerPorts(Peer *newPeer)
{
	if (newPeer->port == myPort && myHostName.compare(newPeer->hostName) == 0){
		return -1;
	}

	Peer *oldPeer;
	for (int i = 0; i < peerPorts.size(); ++i) {
		oldPeer = peerPorts.at(i);
		if (oldPeer->port == newPeer->port && oldPeer->hostAddress == newPeer->hostAddress){
			return i;
		}
 	}
 	return -2;
}

// creates new peer and initiates getting host info
void PeerManager::newPeer(QString peer)
{
	int idx = peer.lastIndexOf(":");
	if (idx == -1){
		return;
	}
	QString host = peer.left(idx);
	QString port = peer.right(peer.size() - idx - 1);
	
	// add peer then trigger get host
	Peer *newPeer = new Peer();
	newPeer->port = port.toInt();
	connect(newPeer, SIGNAL(initPeerDone()), this, SLOT(addPeer()));
	newPeer->initPeer(host);
}

// adds peer to the list of peers for rumormongering
void PeerManager::addPeer()
{
	Peer *newPeer = (Peer*) QObject::sender();
	disconnect(newPeer, 0, this, 0);

	if (this->checkPeerPorts(newPeer) >= -1){
		delete newPeer->timer;
		delete newPeer;
		return;
	}

	peerPorts.append(newPeer);
	emit newPeerReady(newPeer);
	connect(newPeer->timer, SIGNAL(timeout()), this->parent(), SLOT(resendRumor()));
}

// returns a random peer, or null if there are no peers
Peer* PeerManager::randomPeer()
{
	int size = peerPorts.size();

	if (size > 0){
		for (int i = 0; i <= 10; i++){
			int idx = rand() % size;
			Peer *randomPeer = peerPorts.at(idx);
			if (!randomPeer->hostAddress.isNull())
				return randomPeer;
		}
	}
	return NULL;
}

void PeerManager::updateRoutes(QString origin, Peer *peer)
{
	if (originID == origin)
		return;

	if (!routingTable.contains(origin))
		emit addPeerToList(origin);
	routingTable.insert(origin, qMakePair(peer->hostAddress, peer->port));
}

QPair<QHostAddress,quint16> PeerManager::getRoutes(QString origin)
{
	QPair<QHostAddress,quint16> response;
	if (routingTable.contains(origin))
		response = routingTable.value(origin);
	
	return response;
}

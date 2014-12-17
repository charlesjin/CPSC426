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
#include "shamir_charles.hh"
#include "dht.hh"

/*****************************/
/*                           */
/*      Secret Sharing       */
/*                           */
/*****************************/

void NetSocket::secretShareReciever(QMap<QString, QVariant> map)
{
  QString secretID = map["Origin"].toString() + ":" +
      map["SecretNo"].toString();
  QMap <QString, QVariant> secretMap;
  QVariantList secretList = map["SecretShare"].toList();

  secretMap.insert("Seed", map["Seed"]);
  secretMap.insert("Message", map["Message"]);
  secretMap.insert("Threshold", map["Threshold"]);
  secretMap.insert("Seed", map["Seed"]);
  secretMap.insert("Message", map["Message"]);
  if (!secretList.isEmpty()){
    secretMap.insert("x", secretList.first());
    secretMap.insert("fx", secretList.last());
  } else return;
  secrets.insert(secretID, secretMap);

  emit secretRecieved(secretID);
}

void NetSocket::secretRequestReciever(QMap<QString, QVariant> map) 
{
  QString secretID = map["SecretRequest"].toString();

  if (secrets.contains(secretID)){
    QMap<QString, QVariant> secretResponse;
    QMap<QString, QVariant>  secret = secrets[secretID];

    secretResponse.insert("x", secret["x"]);
    secretResponse.insert("fx", secret["fx"]);
    secretResponse.insert("SecretReply", secretID);
    secretResponse.insert("Dest", map["Origin"]);

    sendDirectMessage(secretResponse);
  }
}

void NetSocket::secretReplyReciever(QMap<QString, QVariant> map)
{
  emit newSecretShare(map);
}

void NetSocket::sendSecret(QString secret)
{

  // Get the number of nodes in the network to determine the threshold
  // (number of peers plus 1 to account for the node that generated
  // the secret).
  // Let the threshold k be 75% of the total number of nodes.
  quint16 numNodes = peerManager->routingTable.size() + 1;
  quint16 threshold = (numNodes * 3) / 4;
  // quint16 threshold = numNodes - 1;
  if (threshold <= 0)
    threshold++;

  qint32 secretKey = qrand() % 1298831;

  // Break up the secret into n shares, where n is the number of nodes.
  QList<QPair<qint16, qint64> > secretShares =
    ShamirSecret::generateSecrets(secretKey, numNodes, threshold);

  qint32 seed = qrand() % 1024;

  QString secretMessage = ShamirSecret::encryptMessage(secret, secretKey, QByteArray(QString::number(seed).toAscii().data()));

  // Build the message skeleton.
  QMap<QString, QVariant> secretMsg;
  secretMsg.insert("SecretNo", secretNo++);
  secretMsg.insert("Threshold", threshold);
  secretMsg.insert("Seed", seed);
  secretMsg.insert("Message", secretMessage);

  QHash<QString, QPair<QHostAddress, quint16> >::iterator i;
  QHash<QString, QPair<QHostAddress, quint16> >routingTable = peerManager->routingTable;
  int j = 0;
  for (i = routingTable.begin(); i != routingTable.end(); i++, j++){
    secretMsg.insert("Dest", i.key());

    QPair<qint16, qint64> sharePair = secretShares.at(j);
    QVariantList secretShare;
    secretShare.push_back((quint16) sharePair.first);
    secretShare.push_back((quint64) sharePair.second);
    secretMsg.insert("SecretShare", secretShare);

    sendDirectMessage(secretMsg);
  }

  // Send the last share to yourself.
  QPair<qint16, qint64> sharePair = secretShares.at(j);
  QVariantList secretShare;
  secretShare.push_back((quint16) sharePair.first);
  secretShare.push_back((quint64) sharePair.second);
  secretMsg.insert("SecretShare", secretShare);
  secretMsg.insert("Dest", originID);
  secretMsg.insert("Origin", originID);
  secretMsg.insert("HopLimit", 1);
  secretShareReciever(secretMsg);
}

void NetSocket::recoverSecret(QString secretID)
{
  if (secrets.contains(secretID)){
    QMap<QString, QVariant> shareMap;
    QMap<QString, QVariant>  secret = secrets[secretID];
    shareMap.insert("SecretReply", secretID);
    shareMap.insert("x", secret["x"]);
    shareMap.insert("fx", secret["fx"]);
    shareMap.insert("Threshold", secret["Threshold"]);
    shareMap.insert("Message", secret["Message"]);
    shareMap.insert("Seed", secret["Seed"]);
    emit newSecretShare(shareMap);

    QMap<QString, QVariant> recoverMsg;
    recoverMsg.insert("SecretRequest", secretID);

    QHash<QString, QPair<QHostAddress, quint16> >::iterator i;
    QHash<QString, QPair<QHostAddress, quint16> >routingTable = peerManager->routingTable;

    for (i = routingTable.begin(); i != routingTable.end(); i++){
      recoverMsg.insert("Dest", i.key());
      sendDirectMessage(recoverMsg);
    }
  }
}

void NetSocket::secretRecovered(QString recoveredSecret)
{
  emit secretReconstructed(recoveredSecret);
}

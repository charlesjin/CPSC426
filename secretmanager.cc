/*****************************/
/*                           */
/*      Secret Manager       */
/*                           */
/*****************************/

#include "secretmanager.hh"
#include "shamir.hh"
#include <QDebug>

SecretManager::SecretManager()
{
  // do nothing
}

void SecretManager::newSecretShare(QMap<QString, QVariant> map)
{
  QString secretID = map["SecretReply"].toString();

  if (!secrets.contains(secretID)){
    if (map.contains("Threshold")){
      QPair<quint16, QList<QPair<qint16, qint64> > > newSecretPair;
      QList<QPair<qint16, qint64> > newSecretList;
      newSecretPair.first = map["Threshold"].toUInt();
      newSecretPair.second = newSecretList;
      secrets.insert(secretID, newSecretPair);

      QMap<QString, QVariant> infoEntry;
      infoEntry.insert("encryptedMessage", map["Message"]);
      infoEntry.insert("seed", map["Seed"]);
      secretInfo.insert(secretID, infoEntry);
    } else return;
  }

  if (secrets[secretID].second.length() < secrets[secretID].first){
    QPair<qint16, qint64> point;
    point.first = (qint16) map["x"].toUInt();
    point.second = (qint64) map["fx"].toUInt();
    if (secrets[secretID].second.contains(point))
      return;
    secrets[secretID].second.push_back(point);
  }

  if (secrets[secretID].second.length() >= secrets[secretID].first){
    this->reconstructSecret(secretID);
    secrets.remove(secretID);
    secretInfo.remove(secretID);
    return;
  }
}

void SecretManager::reconstructSecret(QString secretID)
{
  qint32 key;
  key = ShamirSecret::solveSecret(secrets[secretID].second);

  QByteArray encMsg = QByteArray::fromHex(secretInfo[secretID]["encryptedMessage"].toString().toAscii());
  QByteArray init = QByteArray(QString::number(secretInfo[secretID]["seed"].toInt()).toAscii().data());

  QString decryptedMessage = ShamirSecret::decryptMessage(encMsg, key, init);
  emit secretRecovered(decryptedMessage);
}
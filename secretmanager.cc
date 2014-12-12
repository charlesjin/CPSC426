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
  qDebug() << "newSecretShare :" << map;
  QString secretID = map["SecretReply"].toString();

  if (!secrets.contains(secretID)){
    if (map.contains("Threshold")){
      QPair<quint16, QList<QPair<qint16, qint64> > > newSecretPair;
      QList<QPair<qint16, qint64> > newSecretList;
      newSecretPair.first = map["Threshold"].toUInt();
      newSecretPair.second = newSecretList;
      secrets.insert(secretID, newSecretPair);
    } else return;
  }

  if (secrets[secretID].second.length() < secrets[secretID].first){
    QPair<qint16, qint64> point;
    point.first = (qint16) map["x"].toUInt();
    point.second = (qint64) map["fx"].toUInt();
    secrets[secretID].second.push_back(point);
    qDebug() << "secrets after adding: " << secrets;
  }

  if (secrets[secretID].second.length() >= secrets[secretID].first){
    this->reconstructSecret(secretID);
    return;
  }
}

void SecretManager::reconstructSecret(QString secretID)
{
  qDebug() << secrets[secretID];
  qint32 answer;
  answer = ShamirSecret::solveSecret(secrets[secretID].second);
  qDebug() << answer;

  emit secretRecovered(answer);
}


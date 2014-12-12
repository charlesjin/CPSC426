#include "secretmanager.hh"
#include <QDebug>

/* Secret Manager */



SecretManager::SecretManager()
{
  // do nothing
}

void SecretManager::newSecretShare(QMap<QString, QVariant> map)
{
  qDebug() << "newSecretShare :" << map;
}

void SecretManager::reconstructSecret(QString secretID)
{
}


#ifndef SECRET_MANAGER_HH
#define SECRET_MANAGER_HH

#include <QObject>
#include <QMap>
#include <QPair>
#include <QVariant>

class SecretManager : public QObject
{
  Q_OBJECT
  
  public:
    SecretManager();

  public slots:
    void newSecretShare(QMap<QString, QVariant> map);

  signals:
    void secretRecovered(QString recoveredSecret);

  private:
    void reconstructSecret(QString secretID);
    QMap<QString, QPair<quint16, QList<QPair<qint16, qint64> > > > secrets; /* secretID, <threshold, <x, fx> > */
    QMap<QString, QMap<QString, QVariant> > secretInfo; /* secretID, <seed, encryptedMessage> */
};

#endif // SECRET_MANAGER_HH

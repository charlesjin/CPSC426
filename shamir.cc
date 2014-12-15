/*****************************/
/*                           */
/*   Shamir Secret Sharing   */
/*                           */
/*****************************/

#include <ctime>
#include <cmath>
#include <QtCrypto>

#include <QtCore>

#include "shamir.hh"

ShamirSecret::ShamirSecret()
{
  // intialize
}

ShamirSecret::~ShamirSecret()
{
  // destructor
}

QList<QPair<qint16, qint64> > ShamirSecret::generateSecrets(qint32 secret, qint16 noPlayers, quint16 threshold)
{
  QList<QPair<qint16, qint64> > response;
  if (noPlayers < threshold)
    return response;

  QList<qint64> polynomial;

  polynomial << secret;

  qsrand(time(0));
  for (int i = 1; i < threshold; i++)
    polynomial << qrand() % 1000;

  for (qint16 i = 1; i <= noPlayers; i++){
    qint64 value = 0;
    for (int j = 0; j < threshold; j++){
      value += polynomial.at(j) * pow(i, j);
    }
    response << qMakePair(i, value);
  }

  return response;
}

qint32 ShamirSecret::solveSecret(QList<QPair<qint16, qint64> > points)
{
  double response = 0.0;
  for (int i = 0; i < points.length(); i++){
    double term = points.at(i).second;
    for (int j = 0; j < points.length(); j++){
      if (i == j)
        continue;
      term *= -points.at(j).first;
      term /= (points.at(i).first - points.at(j).first);
    }
    response += term;
  }
  return (qint32) round(response);
}

QList<int> ShamirSecret::generatePoints(qint32 seed, qint16 noPlayers, int sizeDHT)
{
  QList<int> list;
  list << ((seed * 2038074133+48487) & sizeDHT);
  for (int i = 1; i < noPlayers; i++){
    list << ((list[i-1] * 2038074133+48487) & sizeDHT);
  }
  return list;
}

QString ShamirSecret::encryptMessage(QString secret, qint32 secretKey, QByteArray init)
{
  QCA::SecureArray key(secretKey);
  QCA::SymmetricKey keyObject(key);
  QCA::InitializationVector iv(init);

  QCA::Cipher cipher(QString("aes128"),QCA::Cipher::CBC,
    QCA::Cipher::DefaultPadding,
    QCA::Encode,
    keyObject, iv);

  QCA::SecureArray arg = secret.toAscii();
  QCA::SecureArray u = cipher.update(arg);
  QCA::SecureArray f = cipher.final();
  QCA::SecureArray cipherText = u.append(f);

  return QCA::arrayToHex(cipherText.toByteArray());
}

QString ShamirSecret::decryptMessage(QByteArray encryptedMessage, qint32 secretKey, QByteArray init)
{

  QCA::SecureArray key(secretKey);
  QCA::SymmetricKey keyObject(key);
  QCA::InitializationVector iv(init);

  QCA::Cipher cipher(QString("aes128"),QCA::Cipher::CBC,
    QCA::Cipher::DefaultPadding,
    QCA::Decode,
    keyObject, iv);

  QCA::SecureArray plainText = cipher.update(encryptedMessage);
  plainText = cipher.final();

  return plainText.data();
}

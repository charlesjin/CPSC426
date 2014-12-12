#ifndef PEERSTER_FILECONNECTION_HH
#define PEERSTER_FILECONNECTION_HH

#include <QObject>
#include <QStringList>
#include <QTimer>

class SearchRequest : public QObject
{
  Q_OBJECT

public:
  SearchRequest(QString searchString);
  QTimer *timer;
  QString searchString;
  QStringList matches;
  quint16 hopLimit;
  quint16 noMatches;
};

class BlockRequest : public QObject
{
  Q_OBJECT

public:
  BlockRequest(QString peerID, QByteArray headerHash, QString fileName);
  QTimer *timer;
  QString peerID;
  QByteArray nextBlock;
  QByteArray headerHash;
  QString fileName;
  quint16 noTries;
};

#endif // PEERSTER_FILECONNECTION_HH
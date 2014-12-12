#ifndef PEERSTER_FILEMANAGER_HH
#define PEERSTER_FILEMANAGER_HH

#include <QObject>
#include <QHash>
#include <QMap>
#include <QVariant>
#include <QStringList>

#include "fileconnection.hh"

typedef struct fileMetaData {
    quint32 size;
    QByteArray blocklist;
    QString fileName;
    int noBlocks;
} fileMetaData;

class FileManager : public QObject
{
  Q_OBJECT

public:
  FileManager();
  QMap<QString, QVariant> fileSearch(QString searchTerms);
  QMap<QString, QVariant> fileFinder(QVariant hash);
  bool checkFile(QMap<QString, QVariant> message);

  QByteArray getNextBlock(QMap<QString, QVariant> map);
  void newSearchRequest(QString searchTerms);

public slots:
  void addFiles(QStringList files);
  void refreshBlockRequest();
  void refreshSearchRequest();

signals:
  void sendBlockRefreshRequest(QMap<QString, QVariant> response);
  void sendSearchRefreshRequest(QMap<QString, QVariant> response);

private:
  void addFile(QString fileName);
  QPair<QByteArray, fileMetaData *> fileStoreSearch(QByteArray hash, 
  QHash<QByteArray, fileMetaData *> searchStore);

  QByteArray getFileBlock(QString fileName, qint16 blockNo);
  QByteArray getHashBlock(QByteArray hash, quint16 blockNo);

  void newBlockRequest(QString fileName, QByteArray headerHash, QByteArray nextHash, QString originID);
  void resetBlockRequest(QString fileName, QByteArray nextBlock);
  void deleteBlockRequest(BlockRequest *blockRequest);
  void deleteBlockRequest(QString fileName);

  void deleteSearchRequest(SearchRequest *searchRequest);

  QHash<QByteArray, fileMetaData *> fileStore; /* <hash, metadata_struct> */
  QHash<QByteArray, fileMetaData *> tempFileStore; /* <hash, metadata_struct> */
  QHash<QString, BlockRequest*> blockRequests; /* <fileName, blockrequest *> */

};

#endif // PEERSTER_FILEMANAGER_HH
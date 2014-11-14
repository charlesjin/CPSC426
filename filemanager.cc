/*****************************/
/*													 */
/*       File Manager        */
/*													 */
/*****************************/

#include <QDebug>
#include <QFile>
#include <QPair>
#include <QList>
#include <QDataStream>
#include <QtCrypto>

#include "filemanager.hh"

FileManager::FileManager()
{

}

void FileManager::addFiles(QStringList files)
{
 	QStringList::Iterator i = files.begin();
 	while (i != files.end()){
    this->addFile(*i);
    ++i;
 	}

 	QHash<QByteArray, fileMetaData *>::Iterator j = fileStore.begin();
 	while (j != fileStore.end()){
 		qDebug() << j.value()->fileName;
 		qDebug() << j.value()->size;
 		qDebug() << j.key();
 		// qDebug() << j.value().blocklist;
 		++j;
 	}
}

void FileManager::addFile(QString fileName)
{
	QFile f(fileName);
	f.open(QIODevice::ReadOnly);
	int size = 8 * 1024;
	qint64 bytesAvailable = f.bytesAvailable();

	fileMetaData *metaData = new fileMetaData();
	metaData->size = f.size();
	metaData->fileName = fileName;
	metaData->noBlocks = 0;

	while (bytesAvailable > 0){
		if (bytesAvailable < size)
			size = bytesAvailable;
		metaData->blocklist.append(QCA::Hash("sha1").hashToString(f.read(size)));
		bytesAvailable -= size;
	}

	f.close();

	QByteArray hash;
	hash.append(QCA::Hash("sha1").hashToString(QByteArray::fromHex(metaData->blocklist)));
	fileStore.insert(hash, metaData);
}

bool FileManager::checkFile(QMap<QString, QVariant> map)
{
	if (map.contains("BlockReply") && map.contains("Data")){
		QByteArray hash = map["BlockReply"].toByteArray().toHex();
		QByteArray data = map["Data"].toByteArray();
		QByteArray test;
		test.append(QCA::Hash("sha1").hashToString(data));
		if (test == hash)
			return true;
	}
	return false;
}

/*****************************/
/*													 */
/*      Block Requests       */
/*													 */
/*****************************/


// called from netsocket
QByteArray FileManager::getNextBlock(QMap<QString, QVariant> map)
{
	QByteArray response;
	QByteArray blockReply = map["BlockReply"].toByteArray().toHex();
	if (!this->checkFile(map))
		return response;

	QPair<QByteArray, fileMetaData*> tempFilePair = 
		this->fileStoreSearch(blockReply, tempFileStore);
	
	fileMetaData *tempFile;
	if (tempFilePair.second == NULL){
		// new file
		QString fileName = "download-" + blockReply;
		QString tempName = fileName;
		while (QFile::exists(tempName))
			tempName = fileName + QString::number(qrand() % 524288);
		fileName = tempName;

		QFile f(fileName);
		if (f.open(QIODevice::ReadWrite)) {
			tempFile = new fileMetaData();
			tempFile->size = 0;
			tempFile->blocklist = map["Data"].toByteArray().toHex();
			tempFile->fileName = fileName;
			tempFile->noBlocks = 0;
			tempFileStore.insert(blockReply, tempFile);
			f.close();

			// response is first block
			response = this->getHashBlock(tempFile->blocklist, 0);
			this->newBlockRequest(fileName, blockReply, response, map["Origin"].toString());
    } else
    	return response;
	} else {
		tempFile  = tempFilePair.second;
		QString fileName = tempFile->fileName;
		if (tempFilePair.first.isEmpty()){
			// new block
			qint16 i = tempFile->blocklist.indexOf(blockReply, tempFile->noBlocks*40);
			i /= 40;
			// check if next one
			if (tempFile->noBlocks == i){
				QByteArray data = map["Data"].toByteArray();

				QFile f(fileName);
				f.open(QIODevice::Append);
				int bytesWritten = f.write(data);
				f.close();
				if (bytesWritten > 0)
					tempFile->size += bytesWritten;
				else
					return response;

				if (tempFile->blocklist.size() - 40 <= i * 40)
					tempFile->noBlocks = -1;
				else
					tempFile->noBlocks = i+1;
			}
		} else
			return response;

		if (tempFile->noBlocks >= 0){
			response = this->getHashBlock(tempFile->blocklist, tempFile->noBlocks);
			this->resetBlockRequest(fileName, response);
		} else
			this->deleteBlockRequest(fileName);

	}

	return response;
}

// finds a file with a given hash
QMap<QString, QVariant> FileManager::fileFinder(QVariant hash)
{
	QMap<QString, QVariant> map;
	QByteArray hashVal = hash.toByteArray();
	QPair<QByteArray, fileMetaData*> response = this->fileStoreSearch(hashVal.toHex(), fileStore);
	if (!response.first.isEmpty()){
		map.insert("BlockReply", hashVal);
		map.insert("Data", response.first);
	}
 	return map;
}

// returns the entire data structure associated with a file
// as well as the particular block of data associated with the hash
QPair<QByteArray, fileMetaData *> FileManager::fileStoreSearch(QByteArray hash, 
	QHash<QByteArray, fileMetaData *> searchStore)
{

	QPair<QByteArray, fileMetaData *> response;

	QHash<QByteArray, fileMetaData *>::Iterator j = searchStore.begin();
 	while (j != searchStore.end()){
 		fileMetaData *metaData = j.value();
 		QByteArray blockList = metaData->blocklist;

 		if (j.key() == hash){
 			response.first = QByteArray::fromHex(blockList);
 			response.second = metaData;
 			return response;
 		} else {
 			qint16 i = blockList.indexOf(hash, metaData->noBlocks*40);
 			if (i >= 0 && i % 40 == 0){
 				response.first = this->getFileBlock(metaData->fileName, i/40);
 				response.second = metaData;
 				return response;
 			}
 		}
 		++j;
 	}
 	return response;
}

QByteArray FileManager::getFileBlock(QString fileName, qint16 blockNo)
{
	QByteArray response;
	QFile f(fileName);
	f.open(QIODevice::ReadOnly);
	int size = 8 * 1024;

	f.read(size * blockNo);
	if (f.bytesAvailable() < size)
		size = f.bytesAvailable();
	response.append(f.read(size));

	return response;
}

QByteArray FileManager::getHashBlock(QByteArray hash, quint16 blockNo)
{
	return QByteArray::fromHex(hash.mid(blockNo*40, 40));
}

/*****************************/
/*													 */
/*      Block Request        */
/*		 	  Connection				 */
/*													 */
/*****************************/

void FileManager::newBlockRequest(QString fileName, QByteArray headerHash, QByteArray fileHash, QString originID)
{
	BlockRequest *blockRequest = new BlockRequest(originID, headerHash, fileName);
	blockRequest->nextBlock = fileHash;
	connect(blockRequest->timer, SIGNAL(timeout()), this, SLOT(refreshBlockRequest()));
	blockRequests.insert(fileName, blockRequest);
	blockRequest->timer->start();
}

void FileManager::refreshBlockRequest()
{
	BlockRequest *blockRequest = qobject_cast<BlockRequest *> (QObject::sender()->parent());
	if (blockRequest){
		if (blockRequest->noTries <= 10){
			(blockRequest->noTries)++;
			QMap<QString, QVariant> response;
			response.insert("BlockRequest", blockRequest->nextBlock);
			response.insert("Dest", blockRequest->peerID);
			emit sendBlockRefreshRequest(response);
		} else {
			this->deleteBlockRequest(blockRequest);
			QFile::remove(blockRequest->fileName);
		}
	}
}

void FileManager::deleteBlockRequest(BlockRequest *blockRequest)
{
	if (blockRequest){
		blockRequests.remove(blockRequest->headerHash);
		delete blockRequest->timer;
		delete blockRequest;
	}
}

void FileManager::deleteBlockRequest(QString fileName)
{
	this->deleteBlockRequest(blockRequests[fileName]);
}

void FileManager::resetBlockRequest(QString fileName, QByteArray nextBlock)
{
	BlockRequest *blockRequest = blockRequests[fileName];
	if (blockRequest){
		blockRequest->noTries = 0;
		blockRequest->nextBlock = nextBlock;
		blockRequest->timer->stop();
		blockRequest->timer->start();
	}
}

/*****************************/
/*													 */
/*      Search Requests      */
/*													 */
/*****************************/

// searches for files that match a string of search terms delimited by spaces
QMap<QString, QVariant> FileManager::fileSearch(QString searchTerms)
{
	QStringList termList = searchTerms.split(" ", QString::SkipEmptyParts);

	QStringList::Iterator i;
	QList<QVariant> fileNames;
 	QList<QVariant> fileHashes;

	QHash<QByteArray, fileMetaData *>::Iterator j = fileStore.begin();
 	while (j != fileStore.end()){
 		fileMetaData *metaData = j.value();
 		QString fileName = metaData->fileName;
 		qDebug() << fileName;
 		i = termList.begin();
 		while (i != termList.end()){
 			qDebug() << *i;
 			if (fileName.contains(*i, Qt::CaseInsensitive)){
 				fileNames << fileName;
 				fileHashes << QByteArray::fromHex(j.key());
 				break;
 			}
 			++i;
	 	}
 		++j;
 	}

 	QMap<QString, QVariant> map;
 	if (fileNames.length() > 0){
 		map.insert("SearchReply", searchTerms);
 		map.insert("MatchNames", fileNames);
 		map.insert("MatchIDs", fileHashes);
 	}
	return map;
}

void FileManager::newSearchRequest(QString searchTerms)
{
	SearchRequest *searchRequest = new SearchRequest(searchTerms);
	connect(searchRequest->timer, SIGNAL(timeout()), this, SLOT(refreshSearchRequest()));
	searchRequest->timer->start();
}

void FileManager::refreshSearchRequest()
{
	SearchRequest *searchRequest = qobject_cast<SearchRequest *> (QObject::sender()->parent());
	if (searchRequest){
		if (searchRequest->hopLimit <= 128){
			(searchRequest->hopLimit) *= 2;
			QMap<QString, QVariant> response;
			response.insert("Search", searchRequest->searchString);
			response.insert("Budget", searchRequest->hopLimit);
			emit sendSearchRefreshRequest(response);
		} else {
			this->deleteSearchRequest(searchRequest);
		}
	}
}

void FileManager::deleteSearchRequest(SearchRequest *searchRequest)
{
	if (searchRequest){
		delete searchRequest->timer;
		delete searchRequest;
	}
}

/*****************************/
/*                           */
/*      File Connection      */
/*                           */
/*****************************/

#include "fileconnection.hh"

/*****************************/
/*                           */
/*      Search Requests      */
/*                           */
/*****************************/

SearchRequest::SearchRequest(QString searchString)
{
  this->searchString = searchString;
  timer = new QTimer(this);
  timer->setInterval(1000);
  hopLimit = 2;
  noMatches = 0;
}


/*****************************/
/*                           */
/*       Block Requests      */
/*                           */
/*****************************/

BlockRequest::BlockRequest(QString peerID, QByteArray headerHash, QString fileName)
{
  this->peerID = peerID;
  this->headerHash = headerHash;
  this->fileName = fileName;
  noTries = 0;
  timer = new QTimer(this);
  timer->setInterval(10000);
}
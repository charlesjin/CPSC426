/*****************************/
/*													 */
/*           Main            */
/*													 */
/*****************************/

// put main UI-related classes here

#include <QVBoxLayout>
#include <QApplication>
#include <QDebug>
#include <QDataStream>
#include <QLineEdit>
#include <QLabel>
#include <QFileDialog>
#include <QtCrypto>
#include <QKeyEvent>

#include "main.hh"
#include "netsocket.hh"
#include "shamir.hh"

/*****************************/
/*													 */
/*        Chat Dialog        */
/*													 */
/*****************************/

// initializes the chat dialog, which is the main IO
ChatDialog::ChatDialog()
{
	setWindowTitle("Peerster");

	// side window to view peers/initiate DM
	peerview = new QListWidget(this);
	peerview->setMaximumWidth(200);
	peerview->setMinimumWidth(150);
	connect(peerview, SIGNAL(itemClicked(QListWidgetItem*)), 
		this, SLOT(itemClicked(QListWidgetItem*)));

	// button to add peers
	QPushButton *button = new QPushButton("Add Peer", this);
	button->setAutoDefault(false);
	button->setDefault(false);
  connect(button, SIGNAL(clicked()), 
  	this, SLOT(addPeer()));

	QVBoxLayout *peerLayout = new QVBoxLayout();
	peerLayout->addWidget(button);
	peerLayout->addWidget(peerview);

	// main chat window
	// displays chats
	textview = new QTextEdit(this);
	textview->setReadOnly(true);
	textview->setMinimumWidth(250);

	// gets input for new messages
	textedit = new TextEdit();

	// upload file
	QPushButton *uploadFileButton = new QPushButton("Upload File", this);
	uploadFileButton->setAutoDefault(false);
	uploadFileButton->setDefault(false);
	connect(uploadFileButton, SIGNAL(clicked()), 
  	this, SLOT(uploadFile()));

	QPushButton *downloadFileButton = new QPushButton("Request File", this);
	downloadFileButton->setAutoDefault(false);
	downloadFileButton->setDefault(false);
	connect(downloadFileButton, SIGNAL(clicked()), 
  	this, SLOT(downloadFile()));

	QHBoxLayout *fileLayout = new QHBoxLayout();
	fileLayout->addWidget(uploadFileButton);
	fileLayout->addWidget(downloadFileButton);

	QVBoxLayout *chatLayout = new QVBoxLayout();
	chatLayout->addWidget(textview);
	chatLayout->addWidget(textedit);
	chatLayout->addLayout(fileLayout);

	QHBoxLayout *layout = new QHBoxLayout();
	layout->addLayout(peerLayout, 1);
	layout->addLayout(chatLayout, 0);
	this->setLayout(layout);
	textedit->setFocus();

	connect(textedit, SIGNAL(returnPressed()),
		this, SLOT(gotReturnPressed()));
}

// slot called from the textedit, which intercepts all returns
void ChatDialog::gotReturnPressed()
{
	QVariant str = textedit->toPlainText();

	// Don't send message if empty
	if (str.toString().size() > 0){
		QMap<QString, QVariant> map;
		map["ChatText"] = str;

		emit sendMessage(map);
		textview->setTextColor( QColor( "red" ) );
		textview->append("me: " + str.toString());
		textview->setTextColor( QColor( "black" ) );
	}

	// Clear the textedit to get ready for the next input message.
	textedit->clear();
}

// output a message when recieved from netsocket
void ChatDialog::recieveMessage(QVariant message)
{
	textview->append(message.toString());
}

// user clicks add peer button
void ChatDialog::addPeer()
{
	PeerDialog *addPeer = new PeerDialog();
	connect(addPeer, SIGNAL(sendPeer(QString)), 
		this, SLOT(sendPeer(QString)));
	addPeer->exec();
}

// relays user input of new peer from peerdialog to netsocket
void ChatDialog::sendPeer(QString str)
{
	emit newPeer(str);
}

void ChatDialog::addPeerToList(QString str)
{
	QListWidgetItem *newItem = new QListWidgetItem;
  newItem->setText(str);
  peerview->insertItem(1, newItem);
}

void ChatDialog::itemClicked(QListWidgetItem* origin)
{
	DMDialog *directMessageDialog = new DMDialog(origin->text());
	connect(directMessageDialog, SIGNAL(newDirectMessage(QMap<QString, QVariant>)), 
		this, SLOT(newDirectMessage(QMap<QString, QVariant>)));
	directMessageDialog->exec();
}

void ChatDialog::newDirectMessage(QMap<QString, QVariant> map)
{
	textview->setTextColor( QColor( "blue" ) );
	textview->append("me (to " + map["Dest"].toString() + "): " + map["ChatText"].toString());
	textview->setTextColor( QColor( "black" ) );
	emit sendDirectMessage(map);
}

void ChatDialog::uploadFile()
{
	QFileDialog *fileDialog = new QFileDialog();
	QStringList files = fileDialog->getOpenFileNames(
    this,
    "Select one or more files to send"
  );
  emit filesSelected(files);
}

void ChatDialog::downloadFile()
{
	FileDialog *fileDialog = new FileDialog();
	connect(fileDialog, SIGNAL(fileRequest(QMap<QString, QVariant>)), 
		this, SLOT(fileRequest(QMap<QString, QVariant>)));
	connect(fileDialog, SIGNAL(showSearchResults(QString)), 
		this, SLOT(showSearchResults(QString)));
	fileDialog->exec();
}

void ChatDialog::fileRequest(QMap<QString, QVariant> map)
{
	emit newFileRequest(map);
}

void ChatDialog::showSearchResults(QString searchTerms)
{
	SearchDialog *searchDialog = new SearchDialog(searchTerms);

	connect(searchDialog->searchview, SIGNAL(itemClicked(QListWidgetItem*)), 
		this, SLOT(searchItemClicked(QListWidgetItem*)));
	connect(this, SIGNAL(showSearchResult(QString)), 
		searchDialog, SLOT(newSearchResults(QString)));

	searchDialog->exec();
}

void ChatDialog::gotSearchResult(QString fileName)
{
	emit showSearchResult(fileName);
}

void ChatDialog::searchItemClicked(QListWidgetItem* item)
{
	QMap<QString, QVariant> map;
	map.insert("fileName", item->text());
	emit newFileRequestFromSearch(map);
}

/*****************************/
/*													 */
/*       Search Dialog       */
/*													 */
/*****************************/

SearchDialog::SearchDialog(QString searchTerms)
{
	searchview = new QListWidget(this);
	searchview->setMinimumWidth(300);
	searchview->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	QLabel *text = new QLabel(this);
	text->setText("Search Terms: " + searchTerms);

	QVBoxLayout *layout = new QVBoxLayout();
	layout->addWidget(text);
	layout->addWidget(searchview);
	this->setLayout(layout);

	connect(searchview, SIGNAL(itemClicked(QListWidgetItem*)), 
		this, SLOT(closeDialog()));
}

void SearchDialog::newSearchResults(QString fileName)
{
	QListWidgetItem *newItem = new QListWidgetItem;
  newItem->setText(fileName);
  searchview->insertItem(searchview->count(), newItem);
}

void SearchDialog::closeDialog()
{
	this->close();
}

/*****************************/
/*													 */
/*         Text Edit         */
/*													 */
/*****************************/

TextEdit::TextEdit()
{
  this->setTabChangesFocus(true);
  this->setWordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
  this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  this->setFixedHeight(sizeHint().height());
}

QSize TextEdit::sizeHint() const
{
  QFontMetrics fm(font());
  int h = (qMax(fm.height(), 14) + 4) * 3;
  int w = fm.width(QLatin1Char('x')) * 17 + 4;
  QStyleOptionFrameV2 opt;
  opt.initFrom(this);
  return (style()->sizeFromContents(QStyle::CT_LineEdit, &opt,
  	QSize(w, h).expandedTo(QApplication::globalStrut()), this));
}

void TextEdit::keyPressEvent(QKeyEvent *event)
{
  if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter){
    event->ignore();
  	emit returnPressed();
  } else
    QTextEdit::keyPressEvent(event);
}

/*****************************/
/*													 */
/*         Add Peer          */
/*													 */
/*****************************/

// IO element for user adding peer
PeerDialog::PeerDialog()
{
	setWindowTitle("Add Peer!");
	QLabel *text = new QLabel(this);
	textLine = new QLineEdit(this);
	text->setText("Enter hostname:port OR ipaddr:port");

	QVBoxLayout *layout = new QVBoxLayout();
	layout->addWidget(text);
	layout->addWidget(textLine);
	setLayout(layout);
	
	connect(textLine, SIGNAL(returnPressed()),
		this, SLOT(gotReturnPressed()));
}

// sends peer string to chatdialog
void PeerDialog::gotReturnPressed()
{
	QString str = textLine->text();

	// Don't send message if empty
	if (str.size() > 0){
		emit sendPeer(str);
		this->close();
	}
}

/*****************************/
/*													 */
/*      Search for File      */
/*													 */
/*****************************/

FileDialog::FileDialog()
{
	setWindowTitle("File Download");

	QLabel *downloadText = new QLabel(this);
	downloadLine = new QLineEdit(this);
	downloadText->setText("Direct download using nodeID:filehash");

	QLabel *searchText = new QLabel(this);
	searchLine = new QLineEdit(this);
	searchText->setText("Search for downloads by keyword");

	QVBoxLayout *layout = new QVBoxLayout();
	layout->addWidget(downloadText);
	layout->addWidget(downloadLine);
	layout->addWidget(searchText);
	layout->addWidget(searchLine);
	this->setLayout(layout);
	
	connect(downloadLine, SIGNAL(returnPressed()),
		this, SLOT(downloadReturnPressed()));
	connect(searchLine, SIGNAL(returnPressed()),
		this, SLOT(searchReturnPressed()));
}

void FileDialog::downloadReturnPressed()
{
	QString str = downloadLine->text();
	if (str.size() > 0){
		QMap<QString, QVariant> map;
		map["blockRequest"] = str;
		emit fileRequest(map);
		this->close();
	}
}

void FileDialog::searchReturnPressed()
{
	QString str = searchLine->text();
	if (str.size() > 0){
		QMap<QString, QVariant> map;
		map["searchRequest"] = str;
		emit fileRequest(map);
		emit showSearchResults(str);
		this->close();
	}
}


/*****************************/
/*													 */
/*      Direct Message       */
/*													 */
/*****************************/

DMDialog::DMDialog(QString str)
{
	origin = str;
	setWindowTitle("Direct Message!");
	QLabel *text = new QLabel(this);
	textLine = new QLineEdit(this);
	text->setText("Send a direct message to " + origin);

	QVBoxLayout *layout = new QVBoxLayout();
	layout->addWidget(text);
	layout->addWidget(textLine);
	setLayout(layout);
	
	connect(textLine, SIGNAL(returnPressed()),
		this, SLOT(gotReturnPressed()));
}

// sends DM to chatdialog
void DMDialog::gotReturnPressed()
{
	QString message = textLine->text();
	QMap<QString, QVariant> map;
	map.insert("ChatText", message);
	map.insert("Dest", origin);

	// Don't send message if empty
	if (message.size() > 0){
		emit newDirectMessage(map);
		this->close();
	}
}

/*****************************/
/*													 */
/*           MAIN            */
/*													 */
/*****************************/

int main(int argc, char **argv)
{
	// Initialize Qt toolkit
	QApplication app(argc,argv);

	// initialize crypto
	QCA::Initializer qcainit;

	// Create an initial chat dialog window
	ChatDialog *dialog = new ChatDialog();
	dialog->show();

	// Create a UDP network socket
	NetSocket *sock = new NetSocket();
	if (!sock->initialize())
		exit(1);

	QObject::connect(dialog, SIGNAL(filesSelected(QStringList)),
		sock->fileManager, SLOT(addFiles(QStringList)));
	QObject::connect(dialog, SIGNAL(newFileRequest(QMap<QString, QVariant>)),
		sock, SLOT(fileRequestSender(QMap<QString, QVariant>)));
	QObject::connect(sock, SIGNAL(refreshSearchResults(QString)), 
		dialog, SLOT(gotSearchResult(QString)));
	QObject::connect(dialog, SIGNAL(newFileRequestFromSearch(QMap<QString, QVariant>)),
		sock, SLOT(fileRequestSender(QMap<QString, QVariant>)));

	QObject::connect(dialog, SIGNAL(newPeer(QString)), 
		sock->peerManager, SLOT(newPeer(QString)));
	QObject::connect(sock->peerManager, SIGNAL(addPeerToList(QString)), 
		dialog, SLOT(addPeerToList(QString)));
	QObject::connect(dialog, SIGNAL(sendDirectMessage(QMap<QString, QVariant>)),
		sock, SLOT(sendDirectMessage(QMap<QString, QVariant>)));

	QObject::connect(dialog, SIGNAL(sendMessage(QMap<QString, QVariant>)), 
		sock, SLOT(messageSender(QMap<QString, QVariant>)));
	QObject::connect(sock, SIGNAL(readyRead()), 
		sock, SLOT(messageReciever()));
	QObject::connect(sock, SIGNAL(messageRecieved(QVariant)), 
		dialog, SLOT(recieveMessage(QVariant)));

	// Enter the Qt main loop; everything else is event driven
	return app.exec();
}

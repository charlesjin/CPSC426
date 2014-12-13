#ifndef PEERSTER_MAIN_HH
#define PEERSTER_MAIN_HH

#include <QDialog>
#include <QFileDialog>
#include <QTextEdit>
#include <QLineEdit>
#include <QMap>
#include <QVariant>
#include <QPushButton>
#include <QListWidget>
#include <QStyle>
#include <QStyleOptionFrameV2>
#include <QApplication>
#include <QTableWidget>

class ChatDialog : public QDialog
{
  Q_OBJECT

  public:
    ChatDialog();

  public slots:
    void gotReturnPressed();
    void recieveMessage(QVariant chatText);
    void addPeer();
    void addPeerToList(QString str);
    void sendPeer(QString peer);
    void itemClicked(QListWidgetItem* origin);
    void newDirectMessage(QMap<QString, QVariant> map);
    void uploadFile();
    void downloadFile();
    void fileRequest(QMap<QString, QVariant> map);
    void showSearchResults(QString searchTerms);
    void searchItemClicked(QListWidgetItem* item);
    void gotSearchResult(QString fileName);
    void clickedShareSecret();
    void newSecret(QString);
    void newSecretRecieved(QString secretID);
    void clickedRecoverSecret();
    void secretClicked(QListWidgetItem* secret);
    void secretReconstructed(QString reconstructedSecret);
    void fingerTableUpdated(QList<QPair<int, int> > table);

  signals:
    void sendMessage(QMap<QString, QVariant> map);
    void sendDirectMessage(QMap<QString, QVariant> map);
    void newPeer(QString peer);
    void filesSelected(QStringList);
    void newFileRequest(QMap<QString, QVariant> map);
    void showSearchResult(QString fileName);
    void newFileRequestFromSearch(QMap<QString, QVariant> map);
    void shareSecret(QString secret);
    void recoverSecret(QString secretID);
    void showReconstructedSecret(QString reconstructedSecret);

  private:
    QTextEdit *textview;
    QTextEdit *textedit;
    QListWidget *peerview;
    QPushButton *button;
    QStringList *secretList;
    QTableWidget *dHTFingerTable;
};

class TextEdit : public QTextEdit
{
  Q_OBJECT

  public:
    TextEdit();
    QSize sizeHint() const;

  protected:
    virtual void keyPressEvent(QKeyEvent *e);

  signals:
    void returnPressed();
};


class PeerDialog : public QDialog
{
  Q_OBJECT

  public:
    PeerDialog();

  public slots:
    void gotReturnPressed();

  signals:
    void sendPeer(QString peer);

  private:
    QLineEdit *textLine;
};

class FileDialog : public QDialog
{
  Q_OBJECT

  public:
    FileDialog();

  public slots:
    void downloadReturnPressed();
    void searchReturnPressed();

  signals:
    void fileRequest(QMap<QString, QVariant>);
    void showSearchResults(QString searchTerms);

  private:
    QLineEdit *searchLine;
    QLineEdit *downloadLine;

};

class SearchDialog : public QDialog
{
  Q_OBJECT

  public:
    SearchDialog(QString searchTerms);
    QListWidget *searchview;

  public slots:
    void newSearchResults(QString fileName);
    void closeDialog();

  signals:
    void newFileRequest(QString fileName);

};

class DMDialog : public QDialog
{
  Q_OBJECT

  public:
    DMDialog(QString origin);

  public slots:
    void gotReturnPressed();

  signals:
    void newDirectMessage(QMap<QString, QVariant> map);

  private:
    QLineEdit *textLine;
    QString origin;

};

class ShareSecretDialog : public QDialog
{
  Q_OBJECT

  public:
    ShareSecretDialog();

  public slots:
    void gotReturnPressed();

  signals:
    void enteredSecret(QString secret);

  private:
    QLineEdit *secretLine;

};

class RecoverSecretDialog : public QDialog
{
  Q_OBJECT

  public:
    RecoverSecretDialog(QStringList secretList);
    QListWidget *secretlistview;
    QTextEdit *recoveredsecretview;

  public slots:
    void closeDialog();
    void showReconstructedSecret(QString reconstructedSecret);

  signals:

  private:

};

#endif // PEERSTER_MAIN_HH

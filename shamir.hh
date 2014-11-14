#ifndef PEERSTER_SHAMIR_HH
#define PEERSTER_SHAMIR_HH

#include <QObject>
#include <QPair>
#include <QList>

class ShamirSecret : public QObject
{
	Q_OBJECT

public:
	static QList<QPair<qint16, qint64> > generateSecrets(qint32 secret, qint16 noPlayers, quint16 threshold);
	static qint32 solveSecret(QList<QPair<qint16, qint64> > points);

private:
	ShamirSecret();
	~ShamirSecret();

};


#endif // PEERSTER_SHAMIR_HH
/*****************************/
/*													 */
/*   Shamir Secret Sharing   */
/*													 */
/*****************************/

#include <ctime>
#include <cmath>

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

	qDebug() << response;
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

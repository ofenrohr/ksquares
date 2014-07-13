#include <QtTest>
#include <QtCore>

#include "aicontroller.h"
#include "ksquaresgame.h"
#include "ksquaresio.h"

// generated
#include "testboardpath.h"

// kde
#include <KDebug>

// other
#include <iostream>

class testRegression: public QObject
{
	Q_OBJECT
	private slots:
		void testBoard001();
};
 
void testRegression::testBoard001()
{
	QScopedPointer<KSquaresGame> sGame(new KSquaresGame());
	QScopedPointer< QList<int> > lines(new QList<int>());
	QWARN(QString(QString(TESTBOARDPATH) + "/4x4-test1.dbl").toLatin1().data());
	QVERIFY(KSquaresIO::loadGame(QString(TESTBOARDPATH) + "/4x4-test1.dbl", sGame.data(), lines.data()));
	//aiController ai(-1, sGame->board()->lines(), QList<int>(), sGame->board()->width(), sGame->board()->height(), 2);
	QVERIFY( true );
}

QTEST_MAIN(testRegression)
#include "regression.moc"
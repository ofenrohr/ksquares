#include <QtTest>
#include <QtCore>

#include "aicontroller.h"
#include "ksquaresgame.h"
#include "ksquaresio.h"

// generated
#include "testboardpath.h"

// kde
#include <kDebug>

class testRegression: public QObject
{
	Q_OBJECT
	private slots:
		void testBoard001();
		void testBoard003();
};
 
/**
 * Test that hard ai doesn't know about the chain rule
 */
void testRegression::testBoard001()
{
	QScopedPointer<KSquaresGame> sGame(new KSquaresGame());
	QList<int> lines;
	QVERIFY(KSquaresIO::loadGame(QString(TESTBOARDPATH) + "/4x4-test1.dbl", sGame.data(), &lines));
	QWARN(lines.toString().toLatin1().data());
	aiController ai(-1, sGame->board()->lines(), sGame->board()->squares(), sGame->board()->width(), sGame->board()->height(), 2);
	int aiLine = ai.chooseLine();
	QCOMPARE( aiLine, 22 );
}

/**
 * Test that hard ai does a hard hearted handout
 */
void testRegression::testBoard003()
{
	QScopedPointer<KSquaresGame> sGame(new KSquaresGame());
	QScopedPointer< QList<int> > lines(new QList<int>());
	QVERIFY(KSquaresIO::loadGame(QString(TESTBOARDPATH) + "/3x3-test3.dbl", sGame.data(), lines.data()));
	aiController ai(-1, sGame->board()->lines(), sGame->board()->squares(), sGame->board()->width(), sGame->board()->height(), 2);
	int aiLine = ai.chooseLine();
	QCOMPARE( aiLine, 5 );
}

QTEST_MAIN(testRegression)
#include "regression.moc"
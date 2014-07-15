#include <QtTest>
#include <QtCore>

#include "aicontroller.h"
#include "ksquaresgame.h"
#include "ksquaresio.h"

// generated
#include "testboardpath.h"

class hardAi: public QObject
{
	Q_OBJECT
	private slots:
		void testBoard001();
		void testBoard002();
		void testBoard003();
		void testBoard005();
};

/**
 * hard ai
 * Two short chains, one chain opened with loony move
 * Expectation: take short chain and open second short chain with hard hearted
 * handout
 */
void hardAi::testBoard001()
{
	QScopedPointer<KSquaresGame> sGame(new KSquaresGame());
	QList<int> lines;
	QVERIFY(KSquaresIO::loadGame(QString(TESTBOARDPATH) + "/4x4-test1.dbl", sGame.data(), &lines));
	for (int i = 0; i < lines.size(); i++)
	{
		sGame->addLineToIndex(lines.at(i));
	}
	aiController ai(-1, sGame->board()->lines(), sGame->board()->squares(), sGame->board()->width(), sGame->board()->height(), 2);
	int aiLine = ai.chooseLine();
	QCOMPARE( aiLine, 18 );
}

/**
 * hard ai
 * Two short chains, one chain opened with loony move
 * Expectation: take short chain and open second short chain with hard hearted
 * handout
 */
void hardAi::testBoard002()
{
	QScopedPointer<KSquaresGame> sGame(new KSquaresGame());
	QList<int> lines;
	QVERIFY(KSquaresIO::loadGame(QString(TESTBOARDPATH) + "/4x4-test2.dbl", sGame.data(), &lines));
	for (int i = 0; i < lines.size(); i++)
	{
		sGame->addLineToIndex(lines.at(i));
	}
	aiController ai(-1, sGame->board()->lines(), sGame->board()->squares(), sGame->board()->width(), sGame->board()->height(), 2);
	int aiLine = ai.chooseLine();
	QCOMPARE( aiLine, 15 );
}

/**
 * hard ai
 * One short chain, one long chain
 * Expectation: ai does a hard hearted handout
 */
void hardAi::testBoard003()
{
	QScopedPointer<KSquaresGame> sGame(new KSquaresGame());
	QList<int> lines;
	QVERIFY(KSquaresIO::loadGame(QString(TESTBOARDPATH) + "/3x3-test3.dbl", sGame.data(), &lines));
	for (int i = 0; i < lines.size(); i++)
	{
		sGame->addLineToIndex(lines.at(i));
	}
	aiController ai(-1, sGame->board()->lines(), sGame->board()->squares(), sGame->board()->width(), sGame->board()->height(), 2);
	int aiLine = ai.chooseLine();
	QCOMPARE( aiLine, 5 );
}

/**
 * hard ai
 * One short chain, one long chain
 * Expectation: ai does a hard hearted handout
 */
void hardAi::testBoard005()
{
	QScopedPointer<KSquaresGame> sGame(new KSquaresGame());
	QList<int> lines;
	QVERIFY(KSquaresIO::loadGame(QString(TESTBOARDPATH) + "/3x2-test5.dbl", sGame.data(), &lines));
	for (int i = 0; i < lines.size(); i++)
	{
		sGame->addLineToIndex(lines.at(i));
	}
	aiController ai(-1, sGame->board()->lines(), sGame->board()->squares(), sGame->board()->width(), sGame->board()->height(), 2);
	int aiLine = ai.chooseLine();
	QCOMPARE( aiLine, 5 );
}

QTEST_MAIN(hardAi)
#include "hardAi.moc"
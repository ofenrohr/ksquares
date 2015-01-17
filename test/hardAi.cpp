#include <QtTest>
#include <QtCore>

#include "aicontroller.h"
#include "ksquaresgame.h"
#include "ksquaresio.h"

//Qt
#include <KDebug>

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
		//void testBoard006();
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
	aiController ai(-1, 1, sGame->board()->width(), sGame->board()->height(), 2);
	int aiLine = ai.chooseLine(sGame->board()->lines(), sGame->board()->squares());
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
	aiController ai(-1, 1, sGame->board()->width(), sGame->board()->height(), 2);
	int aiLine = ai.chooseLine(sGame->board()->lines(), sGame->board()->squares());
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
	aiController ai(-1, 1, sGame->board()->width(), sGame->board()->height(), 2);
	int aiLine = ai.chooseLine(sGame->board()->lines(), sGame->board()->squares());
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
	aiController ai(-1, 1, sGame->board()->width(), sGame->board()->height(), 2);
	int aiLine = ai.chooseLine(sGame->board()->lines(), sGame->board()->squares());
	QCOMPARE( aiLine, 9 );
}

/**
 * hard ai
 * TODO
 *
void hardAi::testBoard006()
{
	QList<int> lines;
  int linesSize = 2 * 6 * 6 + 6 + 6;
  bool linesb[linesSize];
  for (int i = 0; i < linesSize; i++)
  {
    linesb[i] = false;
  }
  QScopedPointer<KSquaresGame> sGame(new KSquaresGame());
  kDebug() << "loading: " << QString(TESTBOARDPATH) << "/6x6-hard.dbl";
  QVERIFY(KSquaresIO::loadGame(QString(TESTBOARDPATH) + "/6x6-hard.dbl", sGame.data(), &lines));
  kDebug() << "drawn lines: " << lines;
  for (int i = 0; i < lines.size(); i++)
  {
    linesb[lines.at(i)] = true;
  }
  QList<QList<int> > ownChains;
  aiFunctions aift(6,6);
  int squaresCnt = aift.findOwnChains(linesb, linesSize, 4, 4, &ownChains);
  kDebug() << "returned squaresCnt = " << squaresCnt;
  kDebug() << "ownChains.size() = " << ownChains.size();
  kDebug() << "input board: " << aift.boardToString(linesb);
	QVERIFY(false);
*/

QTEST_MAIN(hardAi)
#include "hardAi.moc"
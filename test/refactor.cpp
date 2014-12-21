#include <QtTest>
#include <QtCore>

#include "aifunctions.h"
#include "ksquaresgame.h"
#include "ksquaresio.h"

#include <KDebug>

// generated
#include "testboardpath.h"

class refactor : public QObject
{
	Q_OBJECT
	private slots:
		void testFindOwnChains001();
    void testClassifyChain002();
    void testClassifyChain003();
    void testFindOwnChains004();
    void testFindChains005();
};

/**
 * test findOwnChains
 */
void refactor::testFindOwnChains001()
{
	QList<int> lines;
	bool linesb[24];
	for (int i = 0; i < 24; i++)
	{
		linesb[i] = false;
	}
	QScopedPointer<KSquaresGame> sGame(new KSquaresGame());
	kDebug() << "loading: " << QString(TESTBOARDPATH) << "/3x3_chaintest.dbl";
	QVERIFY(KSquaresIO::loadGame(QString(TESTBOARDPATH) + "/3x3_chaintest.dbl", sGame.data(), &lines));
	kDebug() << "drawn lines: " << lines;
	for (int i = 0; i < lines.size(); i++)
	{
		linesb[lines.at(i)] = true;
	}
	QList<QList<int> > ownChains;
	aiFunctions aift(3,3);
	aift.findOwnChains(linesb, 24, 3, 3, &ownChains);
	kDebug() << "ownChains.size() = " << ownChains.size();
	QList<int> chainLengths;
	for (int i = 0; i < ownChains.size(); i++)
	{
		kDebug() << "  " << ownChains.at(i);
		chainLengths.append(ownChains.at(i).size());
	}
	QVERIFY(chainLengths.size()==3);
	QVERIFY(chainLengths.contains(1));
	QVERIFY(chainLengths.contains(2));
	QVERIFY(chainLengths.contains(6));
}

/**
 * test classifyChain
 */
void refactor::testClassifyChain002()
{
  QList<int> lines;
  int linesSize = 2 * 4 * 4 + 4 + 4;
  bool linesb[linesSize];
  for (int i = 0; i < linesSize; i++)
  {
    linesb[i] = false;
  }
  QScopedPointer<KSquaresGame> sGame(new KSquaresGame());
  kDebug() << "loading: " << QString(TESTBOARDPATH) << "/4x4-classify.dbl";
  QVERIFY(KSquaresIO::loadGame(QString(TESTBOARDPATH) + "/4x4-classify.dbl", sGame.data(), &lines));
  QString tmpstr = "";
  for (int i = 0; i < lines.size(); i++)
  {
    linesb[lines.at(i)] = true;
    kDebug() << lines.at(i);
  }
  aiFunctions aift(4,4);
  kDebug() << "input board: " << aift.linelistToString(lines);
  //kDebug() << "input board: " << aift.boardToString(linesb);
  
  KSquares::ChainType result;
  
  QList<int> singleChain;
  singleChain.append(0);
  singleChain.append(4);
  result = aift.classifyChain(singleChain, linesb);
  kDebug() << "single chain result: " << result;
  QVERIFY(result == KSquares::CHAIN_SHORT);
  
  QList<int> longChain1;
  longChain1.append(13);
  longChain1.append(18);
  longChain1.append(27);
  longChain1.append(31);
  result = aift.classifyChain(longChain1, linesb);
  kDebug() << "long chain (shorter one) result: " << result;
  QVERIFY(result == KSquares::CHAIN_LONG);
  
  QList<int> quad;
  quad.append(6);
  quad.append(10);
  quad.append(11);
  quad.append(15);
  result = aift.classifyChain(quad, linesb);
  kDebug() << "quad result: " << result;
  QVERIFY(result == KSquares::CHAIN_LOOP);
}

/**
 * test classifyChain
 */
void refactor::testClassifyChain003()
{
  QList<int> lines;
  int linesSize = 2 * 4 * 4 + 4 + 4;
  bool linesb[linesSize];
  for (int i = 0; i < linesSize; i++)
  {
    linesb[i] = false;
  }
  QScopedPointer<KSquaresGame> sGame(new KSquaresGame());
  kDebug() << "loading: " << QString(TESTBOARDPATH) << "/4x4-classif-2y.dbl";
  QVERIFY(KSquaresIO::loadGame(QString(TESTBOARDPATH) + "/4x4-classify-2.dbl", sGame.data(), &lines));
  QString tmpstr = "";
  for (int i = 0; i < lines.size(); i++)
  {
    linesb[lines.at(i)] = true;
    kDebug() << lines.at(i);
  }
  aiFunctions aift(4,4);
  kDebug() << "input board: " << aift.linelistToString(lines);
  //kDebug() << "input board: " << aift.boardToString(linesb);
  
  KSquares::ChainType result;
  
  QList<int> shortChain1;
  shortChain1.append(0);
  shortChain1.append(9);
  shortChain1.append(13);
  result = aift.classifyChain(shortChain1, linesb);
  kDebug() << "shortChain1 result: " << result;
  QVERIFY(result == KSquares::CHAIN_SHORT);
  
  QList<int> shortChain2;
  shortChain2.append(24);
  shortChain2.append(29);
  result = aift.classifyChain(shortChain2, linesb);
  kDebug() << "shortChain2 result: " << result;
  QVERIFY(result == KSquares::CHAIN_SHORT);
}

/**
 * test findOwnChains
 */
void refactor::testFindOwnChains004()
{
  QList<int> lines;
  int linesSize = 2 * 4 * 4 + 4 + 4;
  bool linesb[linesSize];
  for (int i = 0; i < linesSize; i++)
  {
    linesb[i] = false;
  }
  QScopedPointer<KSquaresGame> sGame(new KSquaresGame());
  kDebug() << "loading: " << QString(TESTBOARDPATH) << "/4x4-chaintest.dbl";
  QVERIFY(KSquaresIO::loadGame(QString(TESTBOARDPATH) + "/4x4-chaintest.dbl", sGame.data(), &lines));
  kDebug() << "drawn lines: " << lines;
  for (int i = 0; i < lines.size(); i++)
  {
    linesb[lines.at(i)] = true;
  }
  QList<QList<int> > ownChains;
  aiFunctions aift(4,4);
  int squaresCnt = aift.findOwnChains(linesb, linesSize, 4, 4, &ownChains);
  kDebug() << "returned squaresCnt = " << squaresCnt;
  kDebug() << "ownChains.size() = " << ownChains.size();
  kDebug() << "input board: " << aift.boardToString(linesb);
  QList<int> chainLengths;
  for (int i = 0; i < ownChains.size(); i++)
  {
    //kDebug() << "  " << ownChains.at(i);
    chainLengths.append(ownChains.at(i).size());
    kDebug() << "board " << i << ": " << aift.linelistToString(ownChains.at(i));
  }
  QVERIFY(chainLengths.size()==1);
  QVERIFY(chainLengths.contains(3));
  QVERIFY(squaresCnt == 4);
}

/**
 * test findChains
 */
void refactor::testFindChains005()
{
  QList<int> lines;
	QScopedPointer<KSquaresGame> sGame(new KSquaresGame());
  QVERIFY(KSquaresIO::loadGame(QString(TESTBOARDPATH) + "/4x4-chaintest-2.dbl", sGame.data(), &lines));
	for (int i = 0; i < lines.size(); i++)
	{
		bool nextPlayer, boardFilled;
		QList<int> completedSquares;
		sGame->board()->addLine(lines[i], &nextPlayer, &boardFilled, &completedSquares);
	}
	
	aiBoard::Ptr board(new aiBoard(sGame->board()));
	
	QList<KSquares::Chain> chains;
	aiFunctions aift(4,4);
	aift.findChains(board, &chains);
	
	QCOMPARE(chains.size(), 2);
	QVERIFY(chains[0].squares.size() == 5 || chains[1].squares.size() == 5);
	QVERIFY(chains[0].squares.size() == 9 || chains[1].squares.size() == 9);
}
QTEST_MAIN(refactor)
#include "refactor.moc"
#include <QtTest>
#include <QtCore>

#include "aifunctions.h"
#include "aiEasyMediumHard.h"
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
    void testFindChains006();
		void testIsJointInCycle007();
		void testIsJointInCycle008();
    void testFindChains009();
    void testFindChains010();
    void testAnalyseBoard011();
    void testAnalyseBoard012();
    void testFindChains013();
		void testClassifyChain014();
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
	aiEasyMediumHard::findOwnChains(linesb, 24, 3, 3, &ownChains);
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
  int squaresCnt = aiEasyMediumHard::findOwnChains(linesb, linesSize, 4, 4, &ownChains);
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
	
	for (int i = 0; i < chains.size(); i++)
	{
		kDebug() << "chain["<<i<<"]: " << aiFunctions::linelistToString(chains[i].lines, board->linesSize, board->width, board->height);
	}
	QCOMPARE(chains.size(), 2);
	QVERIFY(chains[0].squares.size() == 5 || chains[1].squares.size() == 5);
	QVERIFY(chains[0].squares.size() == 9 || chains[1].squares.size() == 9);
}

/**
 * test findChains
 */
void refactor::testFindChains006()
{
  QList<int> lines;
	QScopedPointer<KSquaresGame> sGame(new KSquaresGame());
  QVERIFY(KSquaresIO::loadGame(QString(TESTBOARDPATH) + "/6x3-barker-korf.dbl", sGame.data(), &lines));
	for (int i = 0; i < lines.size(); i++)
	{
		bool nextPlayer, boardFilled;
		QList<int> completedSquares;
		sGame->board()->addLine(lines[i], &nextPlayer, &boardFilled, &completedSquares);
	}
	
	aiBoard::Ptr board(new aiBoard(sGame->board()));
	
	QList<KSquares::Chain> chains;
	aiFunctions aift(6,3);
	aift.findChains(board, &chains);
	
	kDebug() << "board: " << aift.boardToString(board);
	
	for (int i = 0; i < chains.size(); i++)
	{
		kDebug() << "chain: " << chains[i].lines << ", " << chains[i].squares;
		//kDebug() << "visualized chain: " << aift.linelistToString(chains[i].lines, board->linesSize, board->width, board->height);
	}
	
	QCOMPARE(chains.size(), 3);
	
	QList<int> seq1;
	seq1.append(13);
	seq1.append(26);
	seq1.append(33);
	seq1.append(34);
	seq1.append(35);
	seq1.append(36);
	
	QList<int> seq2;
	seq2.append(1);
	seq2.append(14);
	seq2.append(21);
	seq2.append(22);
	seq2.append(23);
	seq2.append(30);
	seq2.append(43);
	
	QList<int> seq3;
	seq3.append(9);
	seq3.append(10);
	seq3.append(11);
	seq3.append(18);
	seq3.append(31);
	
	bool foundChain = false;
	for (int i = 0; i < chains.size(); i++)
	{
		if (chains[i].lines == seq1)
			foundChain = true;
	}
	QVERIFY(foundChain);
}


void refactor::testIsJointInCycle007()
{
	QList<int> lines;
	QScopedPointer<KSquaresGame> sGame(new KSquaresGame());
  QVERIFY(KSquaresIO::loadGame(QString(TESTBOARDPATH) + "/6x3-barker-korf.dbl", sGame.data(), &lines));
	for (int i = 0; i < lines.size(); i++)
	{
		bool nextPlayer, boardFilled;
		QList<int> completedSquares;
		sGame->board()->addLine(lines[i], &nextPlayer, &boardFilled, &completedSquares);
	}
	
	aiBoard::Ptr board(new aiBoard(sGame->board()));
	
	QMap<int, int> squareValences; // square, valence (WARNING: not really the valence, it's the count of border lines!)
	for (int i = 0; i < board->squareOwners.size(); i++)
	{
		if (board->squareOwners[i] == -1)
		{
			squareValences[i] = aiFunctions::countBorderLines(board->width, board->height, i, board->lines);
		}
	}
	
	aiFunctions aift(6,3);
	QVERIFY(! aift.jointInCycle(board, 16, 10, squareValences));
}


void refactor::testIsJointInCycle008()
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
	
	QMap<int, int> squareValences; // square, valence (WARNING: not really the valence, it's the count of border lines!)
	for (int i = 0; i < board->squareOwners.size(); i++)
	{
		if (board->squareOwners[i] == -1)
		{
			squareValences[i] = aiFunctions::countBorderLines(board->width, board->height, i, board->lines);
		}
	}
	
	aiFunctions aift(board->width, board->height);
	QVERIFY(aift.jointInCycle(board, 5, 4, squareValences));
}

/**
 * test findChains
 */
void refactor::testFindChains009()
{
  QList<int> lines;
	QScopedPointer<KSquaresGame> sGame(new KSquaresGame());
  QVERIFY(KSquaresIO::loadGame(QString(TESTBOARDPATH) + "/5x5-chaintest.dbl", sGame.data(), &lines));
	for (int i = 0; i < lines.size(); i++)
	{
		bool nextPlayer, boardFilled;
		QList<int> completedSquares;
		sGame->board()->addLine(lines[i], &nextPlayer, &boardFilled, &completedSquares);
	}
	
	aiBoard::Ptr board(new aiBoard(sGame->board()));
	
	QList<KSquares::Chain> chains;
	aiFunctions aift(board->width, board->height);
	aift.findChains(board, &chains);
	
	QCOMPARE(chains.size(), 9);
}

/**
 * test findChains
 */
void refactor::testFindChains010()
{
  QList<int> lines;
	QScopedPointer<KSquaresGame> sGame(new KSquaresGame());
  QVERIFY(KSquaresIO::loadGame(QString(TESTBOARDPATH) + "/3x3-chaintest-2.dbl", sGame.data(), &lines));
	for (int i = 0; i < lines.size(); i++)
	{
		bool nextPlayer, boardFilled;
		QList<int> completedSquares;
		sGame->board()->addLine(lines[i], &nextPlayer, &boardFilled, &completedSquares);
	}
	
	aiBoard::Ptr board(new aiBoard(sGame->board()));
	
	QList<KSquares::Chain> chains;
	aiFunctions aift(board->width, board->height);
	aift.findChains(board, &chains);
	
	QCOMPARE(chains.size(), 3);
	for (int i = 0; i < chains.size(); i++)
	{
		QVERIFY(
			chains[i].squares.size() == 2 ||
			chains[i].squares.size() == 3 ||
			chains[i].squares.size() == 6
		);
		
		QVERIFY(
			chains[i].lines.size() == 3 ||
			chains[i].lines.size() == 4 ||
			chains[i].lines.size() == 7
		);
	}
}

/**
 * test analyseBoard
 */
void refactor::testAnalyseBoard011()
{
  QList<int> lines;
	QScopedPointer<KSquaresGame> sGame(new KSquaresGame());
  QVERIFY(KSquaresIO::loadGame(QString(TESTBOARDPATH) + "/3x3-chaintest-2.dbl", sGame.data(), &lines));
	for (int i = 0; i < lines.size(); i++)
	{
		bool nextPlayer, boardFilled;
		QList<int> completedSquares;
		sGame->board()->addLine(lines[i], &nextPlayer, &boardFilled, &completedSquares);
	}
	
	aiBoard::Ptr board(new aiBoard(sGame->board()));
	
	KSquares::BoardAnalysis analysis = BoardAnalysisFunctions::analyseBoard(board);
	
	QCOMPARE(analysis.chainsAfterCapture.size(), 3);
	for (int i = 0; i < analysis.chainsAfterCapture.size(); i++)
	{
		QVERIFY(
			analysis.chainsAfterCapture[i].squares.size() == 2 ||
			analysis.chainsAfterCapture[i].squares.size() == 3 ||
			analysis.chainsAfterCapture[i].squares.size() == 6
		);
		
		QVERIFY(
			analysis.chainsAfterCapture[i].lines.size() == 3 ||
			analysis.chainsAfterCapture[i].lines.size() == 4 ||
			analysis.chainsAfterCapture[i].lines.size() == 7
		);
	}
	
	QCOMPARE(analysis.openShortChains.size(), 1);
	QCOMPARE(analysis.openLongChains.size(), 2);
	QCOMPARE(analysis.openLoopChains.size(), 0);
}

/**
 * test analyseBoard
 */
void refactor::testAnalyseBoard012()
{
  QList<int> lines;
	QScopedPointer<KSquaresGame> sGame(new KSquaresGame());
  QVERIFY(KSquaresIO::loadGame(QString(TESTBOARDPATH) + "/3x3-chaintest-3.dbl", sGame.data(), &lines));
	for (int i = 0; i < lines.size(); i++)
	{
		bool nextPlayer, boardFilled;
		QList<int> completedSquares;
		sGame->board()->addLine(lines[i], &nextPlayer, &boardFilled, &completedSquares);
	}
	
	aiBoard::Ptr board(new aiBoard(sGame->board()));
	
	KSquares::BoardAnalysis analysis = BoardAnalysisFunctions::analyseBoard(board);
	
	
	QCOMPARE(analysis.chains.size(), 2);
	for (int i = 0; i < analysis.chains.size(); i++)
	{
		kDebug() << "chains["<<i<<"].type = "<<analysis.chains[i].type;
		kDebug() << aiFunctions::linelistToString(analysis.chains[i].lines, board->linesSize, board->width, board->height);
		QVERIFY(analysis.chains[i].squares.size() == 1);
		QVERIFY(analysis.chains[i].lines.size() == 1);
	}
	for (int i = 0; i < analysis.chainsAfterCapture.size(); i++)
	{
		kDebug() << "chainsAfterCapture["<<i<<"].type = "<<analysis.chainsAfterCapture[i].type;
		kDebug() << aiFunctions::linelistToString(analysis.chainsAfterCapture[i].lines, board->linesSize, board->width, board->height);
	}
	
	QCOMPARE(analysis.capturableShortChains.size(), 2);
	QCOMPARE(analysis.openLongChains.size(), 1);
	QCOMPARE(analysis.openLoopChains.size(), 1);
}


/**
 * test findChains
 */
void refactor::testFindChains013()
{
  QList<int> lines;
	QScopedPointer<KSquaresGame> sGame(new KSquaresGame());
  QVERIFY(KSquaresIO::loadGame(QString(TESTBOARDPATH) + "/3x3-chaintest-4.dbl", sGame.data(), &lines));
	for (int i = 0; i < lines.size(); i++)
	{
		bool nextPlayer, boardFilled;
		QList<int> completedSquares;
		sGame->board()->addLine(lines[i], &nextPlayer, &boardFilled, &completedSquares);
	}
	
	aiBoard::Ptr board(new aiBoard(sGame->board()));
	
	QList<KSquares::Chain> chains;
	aiFunctions aift(board->width, board->height);
	aift.findChains(board, &chains);
	
	for (int i = 0; i < chains.size(); i++)
	{
		kDebug() << "chain["<<i<<"]: cap: " << chains[i].ownChain << ", on board: " << aiFunctions::linelistToString(chains[i].lines, board->linesSize, board->width, board->height);
	}
	
	QCOMPARE(chains.size(), 3);
	QVERIFY(chains[0].squares.size() == 6 || chains[1].squares.size() == 6 || chains[2].squares.size() == 6);
	QVERIFY(chains[0].squares.size() == 2 || chains[1].squares.size() == 2 || chains[2].squares.size() == 2);
	QVERIFY(chains[0].squares.size() == 3 || chains[1].squares.size() == 3 || chains[2].squares.size() == 3);
	QVERIFY(chains[0].lines.size() == 6 || chains[1].lines.size() == 6 || chains[2].lines.size() == 6);
	QVERIFY(chains[0].lines.size() == 3 || chains[1].lines.size() == 3 || chains[2].lines.size() == 3);
	QVERIFY(chains[0].lines.size() == 4 || chains[1].lines.size() == 4 || chains[2].lines.size() == 4);
}


/**
 * test findChains
 */
void refactor::testClassifyChain014()
{
  QList<int> lines;
	QScopedPointer<KSquaresGame> sGame(new KSquaresGame());
  QVERIFY(KSquaresIO::loadGame(QString(TESTBOARDPATH) + "/2x2-classify.dbl", sGame.data(), &lines));
	for (int i = 0; i < lines.size(); i++)
	{
		bool nextPlayer, boardFilled;
		QList<int> completedSquares;
		sGame->board()->addLine(lines[i], &nextPlayer, &boardFilled, &completedSquares);
	}
	
	aiBoard::Ptr board(new aiBoard(sGame->board()));
	
	QList<KSquares::Chain> chains;
	aiFunctions aift(board->width, board->height);
	aift.findChains(board, &chains);
	
	for (int i = 0; i < chains.size(); i++)
	{
		kDebug() << "chain["<<i<<"]: cap: " << chains[i].ownChain << ", on board: " << aiFunctions::linelistToString(chains[i].lines, board->linesSize, board->width, board->height);
	}
	
	QCOMPARE(chains.size(), 2);
	QVERIFY((chains[0].squares.size() == 4) ^ (chains[0].squares.size() == 0));
	QVERIFY((chains[0].lines.size() == 5) ^ (chains[0].lines.size() == 1));
	QVERIFY((chains[0].type == KSquares::CHAIN_LOOP) ^ (chains[0].type == KSquares::CHAIN_SPECIAL));
	QVERIFY((chains[1].squares.size() == 4) ^ (chains[1].squares.size() == 0));
	QVERIFY((chains[1].lines.size() == 5) ^ (chains[1].lines.size() == 1));
	QVERIFY((chains[1].type == KSquares::CHAIN_LOOP) ^ (chains[1].type == KSquares::CHAIN_SPECIAL));
}

QTEST_MAIN(refactor)
#include "refactor.moc"
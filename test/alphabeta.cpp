#include <QtTest>
#include <QtCore>

#include "aifunctions.h"
#include "ksquaresgame.h"
#include "ksquaresio.h"
#include "aiBoard.h"
#include "aiAlphaBeta.h"

#include <QList>
#include <QElapsedTimer>
#include <KDebug>

// generated
#include "testboardpath.h"

class alphabeta : public QObject
{
	Q_OBJECT
	private slots:
		void testAlphaBeta001();
		//void testAlphaBeta002();
		void testAlphaBeta003();
		void testBerlekamp007();
		void testHeuristic001();
		void testAlphaBeta004();
		void testCornerLines001();
		void testAlphaBeta005();
};

template <typename T>
bool qListBeginsWith(QList<T> list, QList<T> begin)
{
	if (list.size() < begin.size())
		return false;
	for (int i = 0; i < begin.size(); i++)
		if (begin[i] != list[i])
			return false;
	return true;
}

/**
 * Test getMoveSequences
 */
void alphabeta::testAlphaBeta001()
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
	
	KSquares::BoardAnalysis analysis = aiFunctions::analyseBoard(board);
	QList<QList<int> > moveSequences = aiAlphaBeta::getMoveSequences(board, analysis);
	
	kDebug() << "generated move sequences: " << moveSequences;
	
	
	// closed chain
	QList<int> closedChainA;
	closedChainA.append(9);
	closedChainA.append(10);
	closedChainA.append(11);
	closedChainA.append(18);
	closedChainA.append(31);
	QList<int> closedChainB;
	closedChainB.append(31);
	closedChainB.append(18);
	closedChainB.append(11);
	closedChainB.append(10);
	closedChainB.append(9);
	// half open chain - no double dealing
	QList<int> seq1a;
	seq1a.append(closedChainA);
	seq1a.append(13);
	seq1a.append(26);
	seq1a.append(33);
	seq1a.append(34);
	seq1a.append(35);
	seq1a.append(36);
	//seq1a.append(22);
	QList<int> seq1b;
	seq1b.append(closedChainB);
	seq1b.append(13);
	seq1b.append(26);
	seq1b.append(33);
	seq1b.append(34);
	seq1b.append(35);
	seq1b.append(36);
	//seq1b.append(22);
	kDebug() << "seq1a: " << seq1a;
	kDebug() << "seq1b: " << seq1b;
	
	// half open chain - with double dealing
	QList<int> seq2a;
	seq2a.append(closedChainA);
	seq2a.append(13);
	seq2a.append(26);
	seq2a.append(33);
	seq2a.append(34);
	seq2a.append(36);
	QList<int> seq2b;
	seq2b.append(closedChainB);
	seq2b.append(13);
	seq2b.append(26);
	seq2b.append(33);
	seq2b.append(34);
	seq2b.append(36);
	
	
	QVERIFY((bool)moveSequences.contains(seq2a) ^ (bool)moveSequences.contains(seq2b));
	
	int matches = 0;
	for (int i = 0; i < moveSequences.size(); i++)
	{
		if (qListBeginsWith(moveSequences[i], seq1a))
			matches++;
		if (qListBeginsWith(moveSequences[i], seq1b))
			matches++;
	}
	QCOMPARE(matches, 1);
}

/**
 * Test getMoveSequences
 * board to complicated for testing...
 */

/**
 * Test getMoveSequences
 */
void alphabeta::testAlphaBeta003()
{
	QList<int> lines;
	QScopedPointer<KSquaresGame> sGame(new KSquaresGame());
  QVERIFY(KSquaresIO::loadGame(QString(TESTBOARDPATH) + "/berlekamp-3.1.dbl", sGame.data(), &lines));
	for (int i = 0; i < lines.size(); i++)
	{
		bool nextPlayer, boardFilled;
		QList<int> completedSquares;
		sGame->board()->addLine(lines[i], &nextPlayer, &boardFilled, &completedSquares);
	}
	
	aiBoard::Ptr board(new aiBoard(sGame->board()));
	
	KSquares::BoardAnalysis analysis = aiFunctions::analyseBoard(board);
	QList<QList<int> > moveSequences = aiAlphaBeta::getMoveSequences(board, analysis);
	
	kDebug() << moveSequences;
}

/**
 * test based on berlekamps book "sophisticated child's play"
 * page 15 & 16
 */
void alphabeta::testBerlekamp007()
{
  QList<int> lines;
	QScopedPointer<KSquaresGame> sGame(new KSquaresGame());
  QVERIFY(KSquaresIO::loadGame(QString(TESTBOARDPATH) + "/berlekamp-3.7.dbl", sGame.data(), &lines));
	for (int i = 0; i < lines.size(); i++)
	{
		bool nextPlayer, boardFilled;
		QList<int> completedSquares;
		sGame->board()->addLine(lines[i], &nextPlayer, &boardFilled, &completedSquares);
	}
	
	QList<int> expectedLines;
	expectedLines.append(9);
	
	aiAlphaBeta ai(lines.size() % 2, 1, sGame->board()->width(), sGame->board()->height(), -1);
	ai.setDebug(true);
	int aiLine = ai.chooseLine(sGame->board()->lines(), sGame->board()->squares());
	// write dot tree to file
	QFile file("/tmp/berlekamp-7.dot");
	if (!file.open(QIODevice::ReadWrite | QIODevice::Truncate))
	{
		kDebug() << "error: Can't open file";
		return;
	}
	QTextStream fileoutput(&file);
	fileoutput << "graph {\n" << ai.getDebugDot() << "}";
	file.close();
	
	QVERIFY(expectedLines.contains(aiLine));
}

void alphabeta::testHeuristic001()
{
	QList<int> lines;
	QScopedPointer<KSquaresGame> sGame(new KSquaresGame());
  QVERIFY(KSquaresIO::loadGame(QString(TESTBOARDPATH) + "/berlekamp-3.1.dbl", sGame.data(), &lines));
	for (int i = 0; i < lines.size(); i++)
	{
		bool nextPlayer, boardFilled;
		QList<int> completedSquares;
		sGame->board()->addLine(lines[i], &nextPlayer, &boardFilled, &completedSquares);
	}
	aiBoard::Ptr board(new aiBoard(sGame->board()));
	
	aiHeuristic heuristic(false, false, true);
	heuristic.setDebug(true);
	float result1 = heuristic.evaluate(board);
	
	board->doMove(16);
	heuristic.reset();
	float result2 = heuristic.evaluate(board);
	
	board->doMove(17);
	heuristic.reset();
	float result3 = heuristic.evaluate(board);
	
	QVERIFY(result1 < 0);
	QVERIFY(result2 < 0);
	QVERIFY(result3 > 0);
}

void alphabeta::testAlphaBeta004()
{
	QList<int> lines;
	QScopedPointer<KSquaresGame> sGame(new KSquaresGame());
  QVERIFY(KSquaresIO::loadGame(QString(TESTBOARDPATH) + "/berlekamp-3.1.dbl", sGame.data(), &lines));
	for (int i = 0; i < lines.size(); i++)
	{
		bool nextPlayer, boardFilled;
		QList<int> completedSquares;
		sGame->board()->addLine(lines[i], &nextPlayer, &boardFilled, &completedSquares);
	}
	aiBoard::Ptr board(new aiBoard(sGame->board()));
	
	
	aiAlphaBeta ai(0, 1, board->width, board->height, -1);
	ai.setDepth(1);
	ai.setDebug(true);
	int aiLine = ai.chooseLine(sGame->board()->lines(), sGame->board()->squares());
	kDebug() << "ai line = " << aiLine;
	
	// write dot tree to file
	QFile file("/tmp/berlekamp-1.dot");
	if (!file.open(QIODevice::ReadWrite | QIODevice::Truncate))
	{
		kDebug() << "error: Can't open file";
		return;
	}
	QTextStream fileoutput(&file);
	fileoutput << "graph {\n" << ai.getDebugDot() << "}";
	file.close();
	
	// do sth not so smart
	bool nextPlayer, gameOver = false;
	QList<int> completedSquares;
	sGame->board()->addLine(13, &nextPlayer, &gameOver, &completedSquares);
	
	// next iteration
	int aiLine2 = ai.chooseLine(sGame->board()->lines(), sGame->board()->squares());
	kDebug() << "ai line 2 = " << aiLine2;
	
	// write dot tree to file
	QFile file2("/tmp/berlekamp-1-2.dot");
	if (!file2.open(QIODevice::ReadWrite | QIODevice::Truncate))
	{
		kDebug() << "error: Can't open file";
		return;
	}
	QTextStream fileoutput2(&file2);
	fileoutput2 << "graph {\n" << ai.getDebugDot() << "}";
	file2.close();
}


void alphabeta::testCornerLines001()
{
	bool lines[4];
	QList<int> owners;
	owners.append(-1);
	aiBoard::Ptr board(new aiBoard(lines, 4, 1, 1, owners, 0, 1));
	
	for (int j = 0; j < 4; j++)
	{
		QList<int> ignore = aiAlphaBeta::ignoreCornerLines(board);
		for (int i = 0; i < 4; i++)
		{
			if (board->lines[i])
				continue;
			if (ignore.contains(i))
				continue;
			board->doMove(i);
			break;
		}
	}
	for (int i = 0; i < 4; i++)
	{
		QVERIFY(board->lines[i]);
	}
}

void alphabeta::testAlphaBeta005()
{
	/*
	QList<int> lines;
	QScopedPointer<KSquaresGame> sGame(new KSquaresGame());
  QVERIFY(KSquaresIO::loadGame(QString(TESTBOARDPATH) + "/berlekamp-3.1.dbl", sGame.data(), &lines));
	for (int i = 0; i < lines.size(); i++)
	{
		bool nextPlayer, boardFilled;
		QList<int> completedSquares;
		sGame->board()->addLine(lines[i], &nextPlayer, &boardFilled, &completedSquares);
	}
	aiBoard::Ptr board(new aiBoard(sGame->board()));
	
	
	aiAlphaBeta ai(0, 1, board->width, board->height, -1);
	int aiLine = ai.chooseLine(sGame->board()->lines(), sGame->board()->squares());
	
	kDebug() << "aiLine 0: " << aiLine;
	
	if (aiLine != 16)
		kDebug() << "ERROR: aiLine != 16";
	
	bool nextPlayer, gameOver = false;
	int i = 1;
	while (!gameOver)
	{
		QList<int> squaresCompleted;
		aiLine = ai.chooseLine(sGame->board()->lines(), sGame->board()->squares());
		sGame->board()->addLine(aiLine, &nextPlayer, &gameOver, &squaresCompleted);
		kDebug() << "aiLine " << i++ << ": " << aiLine;
	}
	*/
}

QTEST_MAIN(alphabeta)
#include "alphabeta.moc"
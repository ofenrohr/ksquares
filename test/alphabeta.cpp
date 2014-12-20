#include <QtTest>
#include <QtCore>

#include "aifunctions.h"
#include "ksquaresgame.h"
#include "ksquaresio.h"
#include "aiBoard.h"
#include "aiAlphaBeta.h"

#include <QElapsedTimer>
#include <KDebug>

// generated
#include "testboardpath.h"

class alphabeta : public QObject
{
	Q_OBJECT
	private slots:
		void testAlphaBeta001();
		void testAlphaBeta002();
};

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
	
	QList<QList<int> > moveSequences = aiAlphaBeta::getMoveSequences(board);
	
	kDebug() << moveSequences;
	
	
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
	seq1a.append(22);
	QList<int> seq1b;
	seq1b.append(closedChainB);
	seq1b.append(13);
	seq1b.append(26);
	seq1b.append(33);
	seq1b.append(34);
	seq1b.append(35);
	seq1b.append(36);
	seq1b.append(22);
	QVERIFY((bool)moveSequences.contains(seq1a) ^ (bool)moveSequences.contains(seq1b));
	
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
}

/**
 * Test getMoveSequences
 */
void alphabeta::testAlphaBeta002()
{
	QList<int> lines;
	QScopedPointer<KSquaresGame> sGame(new KSquaresGame());
  QVERIFY(KSquaresIO::loadGame(QString(TESTBOARDPATH) + "/6x3-move-seq-01.dbl", sGame.data(), &lines));
	for (int i = 0; i < lines.size(); i++)
	{
		bool nextPlayer, boardFilled;
		QList<int> completedSquares;
		sGame->board()->addLine(lines[i], &nextPlayer, &boardFilled, &completedSquares);
	}
	
	aiBoard::Ptr board(new aiBoard(sGame->board()));
	
	QList<QList<int> > moveSequences = aiAlphaBeta::getMoveSequences(board);
	
	kDebug() << moveSequences;
	
}

QTEST_MAIN(alphabeta)
#include "alphabeta.moc"
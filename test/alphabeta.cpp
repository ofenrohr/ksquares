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
	
	// closed chain
	QList<int> ccA;
	ccA.append(18);
	ccA.append(31);
	ccA.append(37);
	ccA.append(36);
	QList<int> ccB;
	ccB.append(36);
	ccB.append(37);
	ccB.append(31);
	ccB.append(18);
	
	// short half open chains
	QList<int> sc1;
	sc1.append(13);
	sc1.append(19);
	QList<int> sc2;
	sc2.append(2);
	QList<int> sc3;
	sc3.append(3);
	
	// long half open chain
	QList<int> lc;
	lc.append(28);
	lc.append(22);
	lc.append(23);
	lc.append(17);
	lc.append(4);
	
	// long half open chain with double dealing
	QList<int> lcdd;
	lcdd.append(28);
	lcdd.append(22);
	lcdd.append(23);
	lcdd.append(4);
	
	// allowed combinations
	QList<int> seq1a, seq2a, seq3a, seq4a, seq5a, seq6a;
	seq1a.append(sc1);
	seq1a.append(sc2);
	seq1a.append(sc3);
	seq1a.append(ccA);
	seq1a.append(lc);
	
	seq2a.append(sc1);
	seq2a.append(sc3);
	seq2a.append(sc2);
	seq2a.append(ccA);
	seq2a.append(lc);
	
	seq3a.append(sc2);
	seq3a.append(sc1);
	seq3a.append(sc3);
	seq3a.append(ccA);
	seq3a.append(lc);
	
	seq4a.append(sc2);
	seq4a.append(sc3);
	seq4a.append(sc1);
	seq4a.append(ccA);
	seq4a.append(lc);
	
	seq5a.append(sc3);
	seq5a.append(sc1);
	seq5a.append(sc2);
	seq5a.append(ccA);
	seq5a.append(lc);
	
	seq6a.append(sc3);
	seq6a.append(sc2);
	seq6a.append(sc1);
	seq6a.append(ccA);
	seq6a.append(lc);
	
	QList<int> seq1b, seq2b, seq3b, seq4b, seq5b, seq6b;
	seq1b.append(sc1);
	seq1b.append(sc2);
	seq1b.append(sc3);
	seq1b.append(ccB);
	seq1b.append(lc);
	
	seq2b.append(sc1);
	seq2b.append(sc3);
	seq2b.append(sc2);
	seq2b.append(ccB);
	seq2b.append(lc);
	
	seq3b.append(sc2);
	seq3b.append(sc1);
	seq3b.append(sc3);
	seq3b.append(ccB);
	seq3b.append(lc);
	
	seq4b.append(sc2);
	seq4b.append(sc3);
	seq4b.append(sc1);
	seq4b.append(ccB);
	seq4b.append(lc);
	
	seq5b.append(sc3);
	seq5b.append(sc1);
	seq5b.append(sc2);
	seq5b.append(ccB);
	seq5b.append(lc);
	
	seq6b.append(sc3);
	seq6b.append(sc2);
	seq6b.append(sc1);
	seq6b.append(ccB);
	seq6b.append(lc);
	
	kDebug() << seq1a << seq2a << seq3a << seq4a << seq5a << seq6a;
	kDebug() << seq1b << seq2b << seq3b << seq4b << seq5b << seq6b;
	kDebug() << "testing non double dealing variant";
	QVERIFY(
		(bool)moveSequences.contains(seq1a) ^ (bool)moveSequences.contains(seq1b) ||
		(bool)moveSequences.contains(seq2a) ^ (bool)moveSequences.contains(seq2b) ||
		(bool)moveSequences.contains(seq3a) ^ (bool)moveSequences.contains(seq3b) ||
		(bool)moveSequences.contains(seq4a) ^ (bool)moveSequences.contains(seq4b) ||
		(bool)moveSequences.contains(seq5a) ^ (bool)moveSequences.contains(seq5b) ||
		(bool)moveSequences.contains(seq6a) ^ (bool)moveSequences.contains(seq6b));
	
	// double dealing variant
	seq1a.removeLast(); seq1a.removeAt(seq1a.size() - 2);
	seq2a.removeLast(); seq2a.removeAt(seq2a.size() - 2);
	seq3a.removeLast(); seq3a.removeAt(seq3a.size() - 2);
	seq4a.removeLast(); seq4a.removeAt(seq4a.size() - 2);
	seq5a.removeLast(); seq5a.removeAt(seq5a.size() - 2);
	seq6a.removeLast(); seq6a.removeAt(seq6a.size() - 2);
	
	seq1b.removeLast(); seq1b.removeAt(seq1b.size() - 2);
	seq2b.removeLast(); seq2b.removeAt(seq2b.size() - 2);
	seq3b.removeLast(); seq3b.removeAt(seq3b.size() - 2);
	seq4b.removeLast(); seq4b.removeAt(seq4b.size() - 2);
	seq5b.removeLast(); seq5b.removeAt(seq5b.size() - 2);
	seq6b.removeLast(); seq6b.removeAt(seq6b.size() - 2);
	
	kDebug() << "testing double dealing variant";
	QVERIFY((bool)moveSequences.contains(seq1a) ^ (bool)moveSequences.contains(seq1b) ||
		(bool)moveSequences.contains(seq2a) ^ (bool)moveSequences.contains(seq2b) ||
		(bool)moveSequences.contains(seq3a) ^ (bool)moveSequences.contains(seq3b) ||
		(bool)moveSequences.contains(seq4a) ^ (bool)moveSequences.contains(seq4b) ||
		(bool)moveSequences.contains(seq5a) ^ (bool)moveSequences.contains(seq5b) ||
		(bool)moveSequences.contains(seq6a) ^ (bool)moveSequences.contains(seq6b));
	
}

QTEST_MAIN(alphabeta)
#include "alphabetb.moc"
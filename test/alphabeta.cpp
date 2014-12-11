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
	
	// half open chain - no double dealing
	QList<int> seq1;
	seq1.append(13);
	seq1.append(26);
	seq1.append(33);
	seq1.append(34);
	seq1.append(35);
	seq1.append(36);
	QVERIFY(moveSequences.contains(seq1));
	
	// half open chain - with double dealing
	QList<int> seq2;
	seq2.append(13);
	seq2.append(26);
	seq2.append(33);
	seq2.append(34);
	//seq2.append(35);
	seq2.append(36);
	QVERIFY(moveSequences.contains(seq2));
}

QTEST_MAIN(alphabeta)
#include "alphabeta.moc"
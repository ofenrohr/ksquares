#include <QtTest>
#include <QtCore>

#include "aifunctions.h"
#include "ksquaresgame.h"
#include "ksquaresio.h"
#include "aiBoard.h"
#include "aiMiniMax.h"

#include <KDebug>

// generated
#include "testboardpath.h"

class minimax : public QObject
{
	Q_OBJECT
	private slots:
		void testMiniMax001();
		void testMiniMax002();
};

/**
 * test minimax in general
 */
void minimax::testMiniMax001()
{
	int width = 2;
	int height = 1;
	QList<int> lines;
  int linesSize = 2 * width * height + width + height;
  bool linesb[linesSize];
  for (int i = 0; i < linesSize; i++)
  {
    linesb[i] = false;
  }
	QScopedPointer<KSquaresGame> sGame(new KSquaresGame());
	kDebug() << "loading: " << QString(TESTBOARDPATH) << "/2x1-minimax.dbl";
	QVERIFY(KSquaresIO::loadGame(QString(TESTBOARDPATH) + "/2x1-minimax.dbl", sGame.data(), &lines));
	
	for (int i = 0; i < lines.size(); i++) linesb[lines.at(i)] = true;
	QList<int> squareOwners;
	squareOwners.append(-1);
	squareOwners.append(-1);
	
	aiBoard::Ptr board = aiBoard::Ptr(new aiBoard(linesb, linesSize, width, height, squareOwners));
	kDebug() << "evaluation pid 0 = " << aiMiniMax::evaluate(board, 0);
	kDebug() << "evaluation pid 1 = " << aiMiniMax::evaluate(board, 1);
	
	aiMiniMax ai(0, width, height, -1);
	int line = -2;
	float res = ai.minimax(board, 4, 0, 0, &line);
	kDebug() << "pid 0: minimax result: " << res << ", line: " << line;
	res = ai.minimax(board, 4, 1, 0, &line);
	kDebug() << "pid 0: minimax result: " << res << ", line: " << line;
}

/**
 * test aiFunctions::getLeader
 */
void minimax::testMiniMax002()
{
	QList<int> squareOwners;
	squareOwners.append(-1);
	squareOwners.append(-1);
	QVERIFY(aiFunctions::getLeader(squareOwners) == -1);
	
	squareOwners.clear();
	squareOwners.append(0);
	squareOwners.append(-1);
	QVERIFY(aiFunctions::getLeader(squareOwners) == 0);
	
	squareOwners.clear();
	squareOwners.append(-1);
	squareOwners.append(0);
	QVERIFY(aiFunctions::getLeader(squareOwners) == 0);
	
	squareOwners.clear();
	squareOwners.append(-1);
	squareOwners.append(0);
	squareOwners.append(0);
	squareOwners.append(1);
	QVERIFY(aiFunctions::getLeader(squareOwners) == 0);
	
	
	squareOwners.clear();
	squareOwners.append(1);
	squareOwners.append(0);
	squareOwners.append(0);
	squareOwners.append(1);
	QVERIFY(aiFunctions::getLeader(squareOwners) == -2);
}

QTEST_MAIN(minimax)
#include "minimax.moc"
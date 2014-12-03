#include <QtTest>
#include <QtCore>

#include "aifunctions.h"
#include "ksquaresgame.h"
#include "ksquaresio.h"
#include "aiBoard.h"
#include "aiMiniMax.h"

#include <QElapsedTimer>
#include <KDebug>

// generated
#include "testboardpath.h"

class minimax : public QObject
{
	Q_OBJECT
	private slots:
		void testMiniMax001();
		void testMiniMax002();
		void testMiniMax003();
		void testMiniMax004();
		void testMiniMax005();
};

/**
 * test minimax in general
 * board:
 * +--+  +
 *         
 * +  +--+
 */
void minimax::testMiniMax001()
{
	int width = 2;
	int height = 1;
	QList<int> lines;
	QList<bool> linesbl;
  int linesSize = 2 * width * height + width + height;
  for (int i = 0; i < linesSize; i++)
  {
		linesbl.append(false);
  }
	QScopedPointer<KSquaresGame> sGame(new KSquaresGame());
	kDebug() << "loading: " << QString(TESTBOARDPATH) << "/2x1-minimax.dbl";
	QVERIFY(KSquaresIO::loadGame(QString(TESTBOARDPATH) + "/2x1-minimax.dbl", sGame.data(), &lines));
  aiFunctions aift(width,height);
	
	for (int i = 0; i < lines.size(); i++)
	{
		linesbl[lines.at(i)] = true;
	}
	
	QList<int> squareOwners;
	squareOwners.append(-1);
	squareOwners.append(-1);
	
	aiMiniMax ai(0, 1, width, height, -1);
	int line = ai.chooseLine(linesbl, squareOwners);
	QCOMPARE(line, 3);
}

/**
 * test aiFunctions::getLeader
 */
void minimax::testMiniMax002()
{
	QList<int> squareOwners;
	squareOwners.append(-1);
	squareOwners.append(-1);
	QCOMPARE(aiFunctions::getLeader(squareOwners), -1);
	
	squareOwners.clear();
	squareOwners.append(0);
	squareOwners.append(-1);
	QCOMPARE(aiFunctions::getLeader(squareOwners), 0);
	
	squareOwners.clear();
	squareOwners.append(-1);
	squareOwners.append(0);
	QCOMPARE(aiFunctions::getLeader(squareOwners), 0);
	
	squareOwners.clear();
	squareOwners.append(-1);
	squareOwners.append(0);
	squareOwners.append(0);
	squareOwners.append(1);
	QCOMPARE(aiFunctions::getLeader(squareOwners), 0);
	
	squareOwners.clear();
	squareOwners.append(1);
	squareOwners.append(0);
	squareOwners.append(0);
	squareOwners.append(1);
	QCOMPARE(aiFunctions::getLeader(squareOwners), -2);
}

/**
 * generate dot file of search tree
 */
void minimax::testMiniMax003()
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
	QList<int> squareOwners;
	squareOwners.append(-1);
	squareOwners.append(-1);
  aiBoard::Ptr board = aiBoard::Ptr(new aiBoard(linesb, linesSize, width, height, squareOwners, 0, 1));
	aiMiniMax ai(0, 1, width, height, -1);
	ai.setDebug(true);
	int line = -10;
	ai.minimax(board, 2, &line);
	
	QFile file("/tmp/minimax.dot");
	QVERIFY(file.open(QIODevice::ReadWrite | QIODevice::Truncate));
	
	QTextStream outStream(&file);
	outStream << "graph {\n" << ai.getDebugDot() << "}";
}

/**
 * test minimax in general
 * board:
 * +  +
 * |     
 * +  +
 *    |  
 * +  +
 */
void minimax::testMiniMax004()
{
	int width = 1;
	int height = 2;
	QList<int> lines;
	QList<bool> linesbl;
  int linesSize = 2 * width * height + width + height;
  for (int i = 0; i < linesSize; i++)
  {
		linesbl.append(false);
  }
	QScopedPointer<KSquaresGame> sGame(new KSquaresGame());
	kDebug() << "loading: " << QString(TESTBOARDPATH) << "/1x2-minimax.dbl";
	QVERIFY(KSquaresIO::loadGame(QString(TESTBOARDPATH) + "/1x2-minimax.dbl", sGame.data(), &lines));
  aiFunctions aift(width,height);
	
	for (int i = 0; i < lines.size(); i++)
	{
		linesbl[lines.at(i)] = true;
	}
	
	QList<int> squareOwners;
	squareOwners.append(-1);
	squareOwners.append(-1);
	
	aiMiniMax ai(0, 1, width, height, -1);
	int line = ai.chooseLine(linesbl, squareOwners);
	QCOMPARE(line, 3);
}

/**
 * test minimax in general
 * board:
 * +--+--+
 * |      
 * +  +  +
 * |     |
 * +--+  +
 */
void minimax::testMiniMax005()
{
	int width = 2;
	int height = 2;
	QList<int> lines;
	QList<bool> linesbl;
  int linesSize = 2 * width * height + width + height;
  bool linesb[linesSize];
  for (int i = 0; i < linesSize; i++)
  {
    linesb[i] = false;
		linesbl.append(false);
  }
	QScopedPointer<KSquaresGame> sGame(new KSquaresGame());
	kDebug() << "loading: " << QString(TESTBOARDPATH) << "/2x2-minimax.dbl";
	QVERIFY(KSquaresIO::loadGame(QString(TESTBOARDPATH) + "/2x2-minimax.dbl", sGame.data(), &lines));
  aiFunctions aift(width,height);
	
	for (int i = 0; i < lines.size(); i++)
	{
		linesb[lines.at(i)] = true;
		linesbl[lines.at(i)] = true;
	}
	
	QList<int> squareOwners;
	squareOwners.append(-1);
	squareOwners.append(-1);
	squareOwners.append(-1);
	squareOwners.append(-1);
	
	aiMiniMax ai(0, 1, width, height, -1);
	
	
	/*
  aiBoard::Ptr board = aiBoard::Ptr(new aiBoard(linesb, linesSize, width, height, squareOwners, 0, 1));
	ai.setDebug(true);
	int line = -10;
	
	float res = ai.minimax(board, 2, &line);
	
	QFile file("/tmp/minimax2.dot");
	QVERIFY(file.open(QIODevice::ReadWrite | QIODevice::Truncate));
	
	QTextStream outStream(&file);
	outStream << "graph {\n" << ai.getDebugDot() << "}";
	*/
	
	int line = ai.chooseLine(linesbl, squareOwners);
	QCOMPARE(line, 6);
}


QTEST_MAIN(minimax)
#include "minimax.moc"
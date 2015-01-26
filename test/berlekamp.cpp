#include <QtTest>
#include <QtCore>

#include "aifunctions.h"
#include "ksquaresgame.h"
#include "ksquaresio.h"
#include "aiBoard.h"
#include "aicontroller.h"

#include <QElapsedTimer>
#include <QTextStream>
#include <KDebug>

// generated
#include "testboardpath.h"

class berlekamp : public QObject
{
	Q_OBJECT
	private slots:
		void testBerlekamp001(); // 16
		void testBerlekamp002(); // 17, 21
		void testBerlekamp003(); // 7
		void testBerlekamp004(); // 12
		void testBerlekamp005(); // 0
		void testBerlekamp006(); // 10
		void testBerlekamp007(); // 9
		void testBerlekamp008(); // 0
		void testBerlekamp009(); // 0
		void testBerlekamp010(); // 11
		void testBerlekamp011(); // 2
		void testBerlekamp012(); // 15
		void testBerlekamp013(); // 17
};

void executeAi(Board *board, int player, QString name, QList<int> expectedLines)
{
  // open the file
	QString filename = "test-" + name + ".log";
	QFile file(filename);
	if (!file.open(QIODevice::ReadWrite | QIODevice::Truncate))
	{
		kDebug() << "error: Can't open file";
		return;
	}
	
	QTextStream summary(&file);
	
	summary << "Summary for " << name << ": \n";
	for (int i = 0; i <= aiController::getMaxAiLevel(); i++)
	{
		aiController aic(player, 1, board->width(), board->height(), i);
		KSquaresAi::Ptr ai = aic.getAi();
		int aiLine = ai->chooseLine(board->lines(), board->squares());
		if (expectedLines.contains(aiLine))
		{
			summary << "PASS " << name << ": " << ai->getName() << "\n";
		}
		else
		{
			summary << "FAIL " << name << ": " << ai->getName() << ", returned: " << aiLine << ", expected: ";
			for (int j = 0; j < expectedLines.size(); j++)
				summary << QString::number(expectedLines[j]) << (j != expectedLines.size() -1 ? "," : "");
			summary << "\n";
		}
	}
	
	file.close();
}

/**
 * test based on berlekamps book "sophisticated child's play"
 * page 13 & 14
 */
void berlekamp::testBerlekamp001()
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
	
	QList<int> expectedLines;
	expectedLines.append(16);
	executeAi(sGame->board(), lines.size() % 2, "berlekamp-01", expectedLines);
}

/**
 * test based on berlekamps book "sophisticated child's play"
 * page 13 & 14
 */
void berlekamp::testBerlekamp002()
{
  QList<int> lines;
	QScopedPointer<KSquaresGame> sGame(new KSquaresGame());
  QVERIFY(KSquaresIO::loadGame(QString(TESTBOARDPATH) + "/berlekamp-3.2.dbl", sGame.data(), &lines));
	for (int i = 0; i < lines.size(); i++)
	{
		bool nextPlayer, boardFilled;
		QList<int> completedSquares;
		sGame->board()->addLine(lines[i], &nextPlayer, &boardFilled, &completedSquares);
	}
	
	QList<int> expectedLines;
	expectedLines.append(17);
	expectedLines.append(21);
	executeAi(sGame->board(), lines.size() % 2, "berlekamp-02", expectedLines);
}

/**
 * test based on berlekamps book "sophisticated child's play"
 * page 13 & 14
 */
void berlekamp::testBerlekamp003()
{
  QList<int> lines;
	QScopedPointer<KSquaresGame> sGame(new KSquaresGame());
  QVERIFY(KSquaresIO::loadGame(QString(TESTBOARDPATH) + "/berlekamp-3.3.dbl", sGame.data(), &lines));
	for (int i = 0; i < lines.size(); i++)
	{
		bool nextPlayer, boardFilled;
		QList<int> completedSquares;
		sGame->board()->addLine(lines[i], &nextPlayer, &boardFilled, &completedSquares);
	}
	
	QList<int> expectedLines;
	expectedLines.append(7);
	executeAi(sGame->board(), lines.size() % 2, "berlekamp-03", expectedLines);
}

/**
 * test based on berlekamps book "sophisticated child's play"
 * page 15 & 16
 */
void berlekamp::testBerlekamp004()
{
  QList<int> lines;
	QScopedPointer<KSquaresGame> sGame(new KSquaresGame());
  QVERIFY(KSquaresIO::loadGame(QString(TESTBOARDPATH) + "/berlekamp-3.4.dbl", sGame.data(), &lines));
	for (int i = 0; i < lines.size(); i++)
	{
		bool nextPlayer, boardFilled;
		QList<int> completedSquares;
		sGame->board()->addLine(lines[i], &nextPlayer, &boardFilled, &completedSquares);
	}
	
	QList<int> expectedLines;
	expectedLines.append(12);
	executeAi(sGame->board(), lines.size() % 2, "berlekamp-04", expectedLines);
}

/**
 * test based on berlekamps book "sophisticated child's play"
 * page 15 & 16
 */
void berlekamp::testBerlekamp005()
{
  QList<int> lines;
	QScopedPointer<KSquaresGame> sGame(new KSquaresGame());
  QVERIFY(KSquaresIO::loadGame(QString(TESTBOARDPATH) + "/berlekamp-3.5.dbl", sGame.data(), &lines));
	for (int i = 0; i < lines.size(); i++)
	{
		bool nextPlayer, boardFilled;
		QList<int> completedSquares;
		sGame->board()->addLine(lines[i], &nextPlayer, &boardFilled, &completedSquares);
	}
	
	QList<int> expectedLines;
	expectedLines.append(0);
	executeAi(sGame->board(), lines.size() % 2, "berlekamp-05", expectedLines);
}

/**
 * test based on berlekamps book "sophisticated child's play"
 * page 15 & 16
 */
void berlekamp::testBerlekamp006()
{
  QList<int> lines;
	QScopedPointer<KSquaresGame> sGame(new KSquaresGame());
  QVERIFY(KSquaresIO::loadGame(QString(TESTBOARDPATH) + "/berlekamp-3.6.dbl", sGame.data(), &lines));
	for (int i = 0; i < lines.size(); i++)
	{
		bool nextPlayer, boardFilled;
		QList<int> completedSquares;
		sGame->board()->addLine(lines[i], &nextPlayer, &boardFilled, &completedSquares);
	}
	
	QList<int> expectedLines;
	expectedLines.append(10);
	executeAi(sGame->board(), lines.size() % 2, "berlekamp-06", expectedLines);
}

/**
 * test based on berlekamps book "sophisticated child's play"
 * page 15 & 16
 */
void berlekamp::testBerlekamp007()
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
	executeAi(sGame->board(), lines.size() % 2, "berlekamp-07", expectedLines);
}

/**
 * test based on berlekamps book "sophisticated child's play"
 * page 17 & 18
 */
void berlekamp::testBerlekamp008()
{
  QList<int> lines;
	QScopedPointer<KSquaresGame> sGame(new KSquaresGame());
  QVERIFY(KSquaresIO::loadGame(QString(TESTBOARDPATH) + "/berlekamp-3.8.dbl", sGame.data(), &lines));
	for (int i = 0; i < lines.size(); i++)
	{
		bool nextPlayer, boardFilled;
		QList<int> completedSquares;
		sGame->board()->addLine(lines[i], &nextPlayer, &boardFilled, &completedSquares);
	}
	
	QList<int> expectedLines;
	expectedLines.append(0);
	executeAi(sGame->board(), sGame->board()->currentPlayer(), "berlekamp-08", expectedLines);
}

/**
 * test based on berlekamps book "sophisticated child's play"
 * page 17 & 18
 */
void berlekamp::testBerlekamp009()
{
  QList<int> lines;
	QScopedPointer<KSquaresGame> sGame(new KSquaresGame());
  QVERIFY(KSquaresIO::loadGame(QString(TESTBOARDPATH) + "/berlekamp-3.9.dbl", sGame.data(), &lines));
	for (int i = 0; i < lines.size(); i++)
	{
		bool nextPlayer, boardFilled;
		QList<int> completedSquares;
		sGame->board()->addLine(lines[i], &nextPlayer, &boardFilled, &completedSquares);
	}
	
	QList<int> expectedLines;
	expectedLines.append(0);
	executeAi(sGame->board(), sGame->board()->currentPlayer(), "berlekamp-09", expectedLines);
}

/**
 * test based on berlekamps book "sophisticated child's play"
 * page 17 & 18
 */
void berlekamp::testBerlekamp010()
{
  QList<int> lines;
	QScopedPointer<KSquaresGame> sGame(new KSquaresGame());
  QVERIFY(KSquaresIO::loadGame(QString(TESTBOARDPATH) + "/berlekamp-3.10.dbl", sGame.data(), &lines));
	for (int i = 0; i < lines.size(); i++)
	{
		bool nextPlayer, boardFilled;
		QList<int> completedSquares;
		sGame->board()->addLine(lines[i], &nextPlayer, &boardFilled, &completedSquares);
	}
	
	QList<int> expectedLines;
	expectedLines.append(11);
	executeAi(sGame->board(), sGame->board()->currentPlayer(), "berlekamp-10", expectedLines);
}

/**
 * test based on berlekamps book "sophisticated child's play"
 * page 17 & 18
 */
void berlekamp::testBerlekamp011()
{
  QList<int> lines;
	QScopedPointer<KSquaresGame> sGame(new KSquaresGame());
  QVERIFY(KSquaresIO::loadGame(QString(TESTBOARDPATH) + "/berlekamp-3.11.dbl", sGame.data(), &lines));
	for (int i = 0; i < lines.size(); i++)
	{
		bool nextPlayer, boardFilled;
		QList<int> completedSquares;
		sGame->board()->addLine(lines[i], &nextPlayer, &boardFilled, &completedSquares);
	}
	
	QList<int> expectedLines;
	expectedLines.append(2);
	executeAi(sGame->board(), sGame->board()->currentPlayer(), "berlekamp-11", expectedLines);
}

/**
 * test based on berlekamps book "sophisticated child's play"
 * page 19 & 20
 */
void berlekamp::testBerlekamp012()
{
  QList<int> lines;
	QScopedPointer<KSquaresGame> sGame(new KSquaresGame());
  QVERIFY(KSquaresIO::loadGame(QString(TESTBOARDPATH) + "/berlekamp-3.12.dbl", sGame.data(), &lines));
	for (int i = 0; i < lines.size(); i++)
	{
		bool nextPlayer, boardFilled;
		QList<int> completedSquares;
		sGame->board()->addLine(lines[i], &nextPlayer, &boardFilled, &completedSquares);
	}
	
	QList<int> expectedLines;
	expectedLines.append(15);
	executeAi(sGame->board(), sGame->board()->currentPlayer(), "berlekamp-12", expectedLines);
}

/**
 * test based on berlekamps book "sophisticated child's play"
 * page 19 & 20
 */
void berlekamp::testBerlekamp013()
{
  QList<int> lines;
	QScopedPointer<KSquaresGame> sGame(new KSquaresGame());
  QVERIFY(KSquaresIO::loadGame(QString(TESTBOARDPATH) + "/berlekamp-3.13.dbl", sGame.data(), &lines));
	for (int i = 0; i < lines.size(); i++)
	{
		bool nextPlayer, boardFilled;
		QList<int> completedSquares;
		sGame->board()->addLine(lines[i], &nextPlayer, &boardFilled, &completedSquares);
	}
	
	QList<int> expectedLines;
	expectedLines.append(17);
	executeAi(sGame->board(), sGame->board()->currentPlayer(), "berlekamp-13", expectedLines);
}

QTEST_MAIN(berlekamp)
#include "berlekamp.moc"
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
};

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
	
	int expectedLine = 16;
	QString summaryStr = "Summary: \n";
	QTextStream summary(&summaryStr);
	for (int i = 0; i <= aiController::getMaxAiLevel(); i++)
	{
		aiController aic(lines.size() % 2, 1, sGame->board()->width(), sGame->board()->height(), i);
		KSquaresAi::Ptr ai = aic.getAi();
		int aiLine = ai->chooseLine(sGame->board()->lines(), sGame->board()->squares());
		if (aiLine == expectedLine)
		{
			summary << "PASS: " << ai->getName() << "\n";
		}
		else
		{
			summary << "FAIL: " << ai->getName() << ", returned: " << aiLine << ", expected: " << expectedLine << "\n";
		}
	}
	
	kDebug() << summaryStr;
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
	QString summaryStr = "Summary: \n";
	QTextStream summary(&summaryStr);
	for (int i = 0; i <= aiController::getMaxAiLevel(); i++)
	{
		aiController aic(lines.size() % 2, 1, sGame->board()->width(), sGame->board()->height(), i);
		KSquaresAi::Ptr ai = aic.getAi();
		int aiLine = ai->chooseLine(sGame->board()->lines(), sGame->board()->squares());
		if (expectedLines.contains(aiLine))
		{
			summary << "PASS: " << ai->getName() << "\n";
		}
		else
		{
			summary << "FAIL: " << ai->getName() << ", returned: " << aiLine << ", expected: ";
			for (int j = 0; j < expectedLines.size(); j++)
				summary << QString::number(expectedLines[j]) << (j != expectedLines.size() -1 ? "," : "");
			summary << "\n";
		}
	}
	
	kDebug() << summaryStr;
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
	
	int expectedLine = 7;
	QString summaryStr = "Summary: \n";
	QTextStream summary(&summaryStr);
	for (int i = 0; i <= aiController::getMaxAiLevel(); i++)
	{
		aiController aic(lines.size() % 2, 1, sGame->board()->width(), sGame->board()->height(), i);
		KSquaresAi::Ptr ai = aic.getAi();
		int aiLine = ai->chooseLine(sGame->board()->lines(), sGame->board()->squares());
		if (aiLine == expectedLine)
		{
			summary << "PASS: " << ai->getName() << "\n";
		}
		else
		{
			summary << "FAIL: " << ai->getName() << ", returned: " << aiLine << ", expected: " << expectedLine << "\n";
		}
	}
	
	kDebug() << summaryStr;
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
	
	int expectedLine = 12;
	QString summaryStr = "Summary: \n";
	QTextStream summary(&summaryStr);
	for (int i = 0; i <= aiController::getMaxAiLevel(); i++)
	{
		aiController aic(lines.size() % 2, 1, sGame->board()->width(), sGame->board()->height(), i);
		KSquaresAi::Ptr ai = aic.getAi();
		int aiLine = ai->chooseLine(sGame->board()->lines(), sGame->board()->squares());
		if (aiLine == expectedLine)
		{
			summary << "PASS: " << ai->getName() << "\n";
		}
		else
		{
			summary << "FAIL: " << ai->getName() << ", returned: " << aiLine << ", expected: " << expectedLine << "\n";
		}
	}
	
	kDebug() << summaryStr;
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
	
	int expectedLine = 0;
	QString summaryStr = "Summary: \n";
	QTextStream summary(&summaryStr);
	for (int i = 0; i <= aiController::getMaxAiLevel(); i++)
	{
		aiController aic(lines.size() % 2, 1, sGame->board()->width(), sGame->board()->height(), i);
		KSquaresAi::Ptr ai = aic.getAi();
		int aiLine = ai->chooseLine(sGame->board()->lines(), sGame->board()->squares());
		if (aiLine == expectedLine)
		{
			summary << "PASS: " << ai->getName() << "\n";
		}
		else
		{
			summary << "FAIL: " << ai->getName() << ", returned: " << aiLine << ", expected: " << expectedLine << "\n";
		}
	}
	
	kDebug() << summaryStr;
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
	
	int expectedLine = 10;
	QString summaryStr = "Summary: \n";
	QTextStream summary(&summaryStr);
	for (int i = 0; i <= aiController::getMaxAiLevel(); i++)
	{
		aiController aic(lines.size() % 2, 1, sGame->board()->width(), sGame->board()->height(), i);
		KSquaresAi::Ptr ai = aic.getAi();
		int aiLine = ai->chooseLine(sGame->board()->lines(), sGame->board()->squares());
		if (aiLine == expectedLine)
		{
			summary << "PASS: " << ai->getName() << "\n";
		}
		else
		{
			summary << "FAIL: " << ai->getName() << ", returned: " << aiLine << ", expected: " << expectedLine << "\n";
		}
	}
	
	kDebug() << summaryStr;
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
	
	int expectedLine = 9;
	QString summaryStr = "Summary: \n";
	QTextStream summary(&summaryStr);
	for (int i = 0; i <= aiController::getMaxAiLevel(); i++)
	{
		aiController aic(lines.size() % 2, 1, sGame->board()->width(), sGame->board()->height(), i);
		KSquaresAi::Ptr ai = aic.getAi();
		int aiLine = ai->chooseLine(sGame->board()->lines(), sGame->board()->squares());
		if (aiLine == expectedLine)
		{
			summary << "PASS: " << ai->getName() << "\n";
		}
		else
		{
			summary << "FAIL: " << ai->getName() << ", returned: " << aiLine << ", expected: " << expectedLine << "\n";
		}
	}
	
	kDebug() << summaryStr;
}

QTEST_MAIN(berlekamp)
#include "berlekamp.moc"
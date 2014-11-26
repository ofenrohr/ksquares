#include <QtTest>
#include <QtCore>

#include "aicontroller.h"
#include "ksquaresgame.h"
#include "ksquaresio.h"

// generated
#include "testboardpath.h"

class middleAi: public QObject
{
	Q_OBJECT
	private slots:
		void testBoard004();
		void testMiddle001();
};

/**
 * middle ai
 * two single boxes, 3 box chain, 4 box chain
 * Expectation: ai opens one single box
 */
void middleAi::testBoard004()
{
	QScopedPointer<KSquaresGame> sGame(new KSquaresGame());
	QList<int> lines;
	QVERIFY(KSquaresIO::loadGame(QString(TESTBOARDPATH) + "/3x3-test4.dbl", sGame.data(), &lines));
	for (int i = 0; i < lines.size(); i++)
	{
		sGame->addLineToIndex(lines.at(i));
	}
	aiController ai(-1, 1, sGame->board()->width(), sGame->board()->height(), 1);
	int aiLine = ai.chooseLine(sGame->board()->lines(), sGame->board()->squares());
	QList<int> goodLines;
	goodLines.append(17);
	goodLines.append(20);
	goodLines.append(21);
	goodLines.append(23);
	QVERIFY( goodLines.contains(aiLine) );
}

/**
 * middle ai
 * 2 box chain, 3 box chain, quad
 * Expectation: ai opens 2 box chain at any point
 */
void middleAi::testMiddle001()
{
	QScopedPointer<KSquaresGame> sGame(new KSquaresGame());
	QList<int> lines;
	QVERIFY(KSquaresIO::loadGame(QString(TESTBOARDPATH) + "/3x3-middle-1.dbl", sGame.data(), &lines));
	for (int i = 0; i < lines.size(); i++)
	{
		sGame->addLineToIndex(lines.at(i));
	}
	aiController ai(-1, 1, sGame->board()->width(), sGame->board()->height(), 1);
	int aiLine = ai.chooseLine(sGame->board()->lines(), sGame->board()->squares());
	QList<int> goodLines;
  goodLines.append(13);
	goodLines.append(16);
	goodLines.append(23);
	QVERIFY( goodLines.contains(aiLine) );
}

QTEST_MAIN(middleAi)
#include "middleAi.moc"
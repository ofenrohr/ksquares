#include <QtTest>
#include <QtCore>

#include "aiBoard.h"

#include <KDebug>

// generated
#include "testboardpath.h"

class aiboard : public QObject
{
	Q_OBJECT
	private slots:
		void testAiBoard001();
};


void aiboard::testAiBoard001()
{
	int width = 2;
	int height = 2;
	int linesSize = 2 * width * height + width + height;
	bool *lines = new bool[linesSize];
	
	for (int i = 0; i < linesSize; i++) lines[i] = false;
	QList<int> squareOwners;
	for (int i = 0; i < width * height; i++) squareOwners.append(-1);
	
	aiBoard board(lines, linesSize, width, height, squareOwners, 0, 1);
	
	board.doMove(0);
	QVERIFY(board.lines[0]);
	QCOMPARE(board.playerId, 1);
}

QTEST_MAIN(aiboard)
#include "aiboard.moc"
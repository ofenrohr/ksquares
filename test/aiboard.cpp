#include "aiboard.h"

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

void aiboard::testAiBoard002()
{
	int width = 2;
	int height = 2;
	int linesSize = 2 * width * height + width + height;
	bool *lines = new bool[linesSize];
	
	for (int i = 0; i < linesSize; i++) lines[i] = false;
	QList<int> squareOwners;
	for (int i = 0; i < width * height; i++) squareOwners.append(-1);
	
	aiBoard board(lines, linesSize, width, height, squareOwners, 0, 1);
	
	QCOMPARE(board.playerId, 0);
	board.doMove(0);
	QCOMPARE(board.playerId, 1);
	board.doMove(1);
	QCOMPARE(board.playerId, 0);
	board.doMove(2);
	QCOMPARE(board.playerId, 1);
	board.doMove(7);
	QCOMPARE(board.playerId, 0);
	board.doMove(9);
	QCOMPARE(board.playerId, 1);
	board.doMove(10);
	QCOMPARE(board.playerId, 0);
	board.doMove(5);
	QCOMPARE(board.playerId, 1);
	board.doMove(3);
	QCOMPARE(board.playerId, 1);
	QCOMPARE(board.squareOwners[0], 1);
	board.doMove(8);
	QCOMPARE(board.playerId, 1);
	QCOMPARE(board.squareOwners[2], 1);
	board.doMove(4);
	QCOMPARE(board.playerId, 0);
	board.doMove(6);
	QCOMPARE(board.playerId, 0);
	QCOMPARE(board.squareOwners[1], 0);
	board.doMove(11);
	QCOMPARE(board.playerId, 0);
	QCOMPARE(board.squareOwners[3], 0);
	
	qDebug() << "board: " << aiFunctions::boardToString(board.lines, board.linesSize, board.width, board.height);
	
	board.undoMove(11);
	QCOMPARE(board.playerId, 0);
	QCOMPARE(board.squareOwners[3], -1);
	board.undoMove(6);
	QCOMPARE(board.playerId, 0);
	QCOMPARE(board.squareOwners[1], -1);
	board.undoMove(4);
	QCOMPARE(board.playerId, 1);
	board.undoMove(8);
	QCOMPARE(board.playerId, 1);
	QCOMPARE(board.squareOwners[2], -1);
	board.undoMove(3);
	QCOMPARE(board.playerId, 1);
	QCOMPARE(board.squareOwners[0], -1);
	board.undoMove(5);
	QCOMPARE(board.playerId, 0);
	board.undoMove(10);
	QCOMPARE(board.playerId, 1);
	board.undoMove(9);
	QCOMPARE(board.playerId, 0);
	board.undoMove(7);
	QCOMPARE(board.playerId, 1);
	board.undoMove(2);
	QCOMPARE(board.playerId, 0);
	board.undoMove(1);
	QCOMPARE(board.playerId, 1);
	board.undoMove(0);
	QCOMPARE(board.playerId, 0);
}

//QTEST_MAIN(aiboard)
//#include "aiboard.moc"

//
// Created by ofenrohr on 8/10/17.
//

#include <QtCore/QElapsedTimer>
#include "aiDabbleNative.h"

aiDabbleNative::aiDabbleNative(int newPlayerId, int newMaxPlayerId, int newWidth, int newHeight, int newLevel, int thinkTime) : KSquaresAi(newWidth, newHeight), playerId(newPlayerId), maxPlayerId(newMaxPlayerId), level(newLevel), moveTimeout(thinkTime)
{
    width = newWidth;
	height = newHeight;
	linesSize = toLinesSize(width, height);
	lines = new bool[linesSize];
	for (int i = 0; i < linesSize; i++)
        lines[i] = false;
}

aiDabbleNative::~aiDabbleNative()
{
	delete[] lines;
}
int aiDabbleNative::chooseLine(const QList<bool> &newLines, const QList<int> &newSquareOwners, const QList<Board::Move> &/*lineHistory*/) {
    QElapsedTimer moveTimer;
    moveTimer.start();

    if (newLines.size() != linesSize) {
        qCritical() << "something went terribly wrong: newLines.size() != linesSize";
    }
    for (int i = 0; i < linesSize; i++)
        lines[i] = newLines[i];
    squareOwners = newSquareOwners;

    int line = -1;

    // do ai stuff
    m_game = new dabble_nohash::DBGame(w, h);
    m_game->searchDepth = m_defaultDepth;
    m_game->timeLimit = m_defaultTimeLimit;
    m_game->gameLimit = m_defaultGameLimit;
    m_game->hWnd = m_hWnd;

    int x1, x2, y1, y2;
    dabble_nohash::Coords c;
    while (fscanf(f, "(%d, %d) - (%d, %d)", &x1, &y1, &x2, &y2) && !feof(f))
    {
        c.x1 = x1;
        c.x2 = x2;
        c.y1 = y1;
        c.y2 = y2;
        m_game->rgEdgeRemoved[m_game->maxEdgesRemoved] = c;
        m_game->maxEdgesRemoved++;
        fscanf(f, "\n");
    }
    // done with ai stuff

	if (line < 0 || line >= linesSize)
	{
		qDebug() << "dabble native didn't return a correct line: " << line;
		qDebug() << "coosing random valid move";
		QList<int> freeLines = aiFunctions::getFreeLines(lines, linesSize);
		if (freeLines.size() <= 0)
		{
			qDebug() << "no valid lines left!";
			turnTime = moveTimer.elapsed();
			return 0;
		}
		turnTime = moveTimer.elapsed();
		return freeLines.at(qrand() % freeLines.size());
	}

	turnTime = moveTimer.elapsed();
	return line;
}

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
int aiDabbleNative::chooseLine(const QList<bool> &newLines, const QList<int> &newSquareOwners, const QList<Board::Move> &lineHistory) {
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
    m_game = new dabble_nohash::DBGame(width + 1, height + 1);
	int m_defaultTimeLimit = 5;
    int m_defaultGameLimit = -5;
    int m_defaultDepth = 20;
    m_game->searchDepth = m_defaultDepth;
    m_game->timeLimit = m_defaultTimeLimit;
    m_game->gameLimit = m_defaultGameLimit;
    //m_game->hWnd = m_hWnd;

    dabble_nohash::Coords c;

    //while (fscanf(f, "(%d, %d) - (%d, %d)", &x1, &y1, &x2, &y2) && !feof(f))
    for (int i = 0; i < lineHistory.size(); i++)
    {
        QPoint p1;
        QPoint p2;
        if (!Board::indexToPoints(lineHistory[i].line, &p1, &p2, width, height, false))
        {
            qDebug() << "DabbleNative error: invalid line in history";
            return -1;
        }
        //qDebug() << "conversion step one: dots and boxes coordinates: " << p1 << ", " << p2;
        QPair<QPoint, QPoint> dblPoints = Board::pointsToCoins(p1, p2, width, height);
        //qDebug() << "conversion step two: strings and coins coordinated: " << dblPoints.first << ", " << dblPoints.second;
        //outStream << "(" << dblPoints.first.x() << ", " << dblPoints.first.y() << ") - (" << dblPoints.second.x() << ", " << dblPoints.second.y() << ")\n";

        c.x1 = dblPoints.first.x();
        c.x2 = dblPoints.first.y();
        c.y1 = dblPoints.second.x();
        c.y2 = dblPoints.second.y();
        m_game->rgEdgeRemoved[m_game->maxEdgesRemoved] = c;
        m_game->maxEdgesRemoved++;
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

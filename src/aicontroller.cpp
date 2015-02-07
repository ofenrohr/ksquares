/***************************************************************************
 *   Copyright (C) 2006 by Matthew Williams    <matt@milliams.com>         *
 *   Copyright (C) 2014 by Tom Vincent Peters  <kde@vincent-peters.de>     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "aicontroller.h"

#include "aiEasyMediumHard.h"
//#include "aiMiniMax.h"
#include "aiAlphaBeta.h"
#include "dbgame.h"
#include "dbgame-nohash.h"

#include <ctime>
#include <kdebug.h>

#include <QSet>

aiController::aiController(int newPlayerId, int newMaxPlayerId, int newWidth, int newHeight, int newLevel, int thinkTime) : playerId(newPlayerId), maxPlayerId(newMaxPlayerId), width(newWidth), height(newHeight), level(newLevel), aiThinkTime(thinkTime)
{
	//kDebug() << "aiController init: nw = " << newWidth << ", nh = " << newHeight << ", w = " << width << ", h = " << height;
	//linesSize = aiFunctions::toLinesSize(width, height);
	//lines = new bool[linesSize];
	srand( (unsigned)time( NULL ) );
	//kDebug() << "AI: Starting AI level" << level;
}

aiController::~aiController()
{
	//delete[] lines;
}

QList<int> aiController::autoFill(int safeMovesLeft, int width, int height)
{
	QList<int> fillLines;
	
	int linesSize = aiFunctions::toLinesSize(width, height);
	bool *lines = new bool[linesSize];
	for (int i = 0; i < linesSize; i++)
	{
		lines[i] = false;
	}
	// add a random safe moves while there are safe moves left
	QList<int> next;
	//kDebug() << safeMoves().isEmpty();
	while( !( (next = aiFunctions::safeMoves(width, height, linesSize, lines)).isEmpty() ) )
	{
		int nextLine = next[rand() % next.size()];
		lines[nextLine] = true;
		//kDebug() << nextLine;
		fillLines << nextLine;
	}
	
	// safeMovesLeft times delete a line from fillLines
	for (int i = 0; i<safeMovesLeft; ++i)
	{
		if (fillLines.isEmpty()) break;
		int index = rand() % fillLines.size();
		fillLines.removeAt(index);
	}
	
	return fillLines;
}

int aiController::chooseLine(const QList<bool> &newLines, const QList<int> &newSquareOwners, const QList<Board::Move> &lineHistory)
{
	KSquaresAi::Ptr ai = getAi();
	
	return ai->chooseLine(newLines, newSquareOwners, lineHistory);
}

KSquaresAi::Ptr aiController::getAi()
{
	switch (level)
	{
		default:
			kDebug() << "Unknown ai level " << level;
		case 0:
		case 1:
		case 2:
			//kDebug() << "creating aiEasyMediumHard: w = " << width << ", h = " << height;
			ai = KSquaresAi::Ptr(new aiEasyMediumHard(playerId, width, height, level));
		break;
		case 3:
			//kDebug() << "creating aiAlphaBeta";
			if (ai.isNull())
				ai = KSquaresAi::Ptr(new aiAlphaBeta(playerId, maxPlayerId, width, height, level, aiThinkTime));
		break;
		case 4:
			if (ai.isNull())
			{
				kDebug() << "creating dabble ai";
				dabble::DBGame *m_game = new dabble::DBGame(width+1, height+1);
				m_game->searchDepth = 20;
				m_game->timeLimit = 5;
				m_game->gameLimit = -5;
				ai = KSquaresAi::Ptr(m_game);
			}
		break;
		case 5:
			if (ai.isNull())
			{
				kDebug() << "creating dabble.nohash ai";
				dabble_nohash::DBGame *m_game = new dabble_nohash::DBGame(width+1, height+1);
				m_game->searchDepth = 20;
				m_game->timeLimit = 5;
				m_game->gameLimit = -5;
				ai = KSquaresAi::Ptr(m_game);
			}
		break;
	}
	return ai;
}
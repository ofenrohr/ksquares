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
//#include "dbgame-nohash.h"
#include "qdab.h"
#include "knox.h"
#include "aiMCTS.h"

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
	lastTurnTime = -2;
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
	
	int retryCnt = 0;
	while (retryCnt < 3)
	{
		int line = ai->chooseLine(newLines, newSquareOwners, lineHistory);
		if (line < 0)
		{
			retryCnt++;
			kDebug() << "ai returned line index < 0, retry " << retryCnt;
		}
		else
		{
			lastTurnTime = ai->lastMoveTime();
			return line;
		}
	}
	return -1;
}

KSquaresAi::Ptr aiController::getAi()
{
	switch (level)
	{
		default:
			kDebug() << "Unknown ai level " << level;
		case KSquares::AI_EASY:
		case KSquares::AI_MEDIUM:
		case KSquares::AI_HARD:
			//kDebug() << "creating aiEasyMediumHard: w = " << width << ", h = " << height;
			ai = KSquaresAi::Ptr(new aiEasyMediumHard(playerId, width, height, level));
		break;
		case KSquares::AI_VERYHARD:
			//kDebug() << "creating aiAlphaBeta";
			if (ai.isNull())
				ai = KSquaresAi::Ptr(new aiAlphaBeta(playerId, maxPlayerId, width, height, level, aiThinkTime));
		break;
		case KSquares::AI_DABBLE:
			if (ai.isNull())
				ai = KSquaresAi::Ptr(new Dabble(playerId, maxPlayerId, width, height, 0, aiThinkTime));
		break;
		case KSquares::AI_DABBLENOHASH:
			if (ai.isNull())
				ai = KSquaresAi::Ptr(new Dabble(playerId, maxPlayerId, width, height, 1, aiThinkTime));
		case KSquares::AI_QDAB:
			if (ai.isNull())
				ai = KSquaresAi::Ptr(new QDab(playerId, maxPlayerId, width, height, level, aiThinkTime));
		break;
		case KSquares::AI_KNOX:
			if (ai.isNull())
				ai = KSquaresAi::Ptr(new Knox(playerId, maxPlayerId, width, height, level, aiThinkTime));
		break;
		case KSquares::AI_MCTS:
			if (ai.isNull())
				ai = KSquaresAi::Ptr(new aiMCTS(playerId, maxPlayerId, width, height, level, aiThinkTime));
		break;
	}
	return ai;
}
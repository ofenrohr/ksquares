/***************************************************************************
 *   Copyright (C) 2006 by Tom Vincent Peters   <kde@vincent-peters.de>    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "aiBoard.h"
#include "aifunctions.h"

#include <KDebug>

aiBoard::aiBoard(bool *newLines, int newLinesSize, int newWidth, int newHeight, const QList<int> newSquareOwners, int newPlayerId, int newMaxPlayerId) : lines(newLines), linesSize(newLinesSize), width(newWidth), height(newHeight), squareOwners(newSquareOwners), playerId(newPlayerId), maxPlayerId(newMaxPlayerId)
{
	deleteLines = false;
}

aiBoard::aiBoard(Board *board)
{
	linesSize = board->lines().size();
	deleteLines = true;
	lines = new bool[linesSize];
	for (int i = 0; i < linesSize; i++)
		lines[i] = board->lines()[i];
	width = board->width();
	height = board->height();
	squareOwners = board->squares();
	playerId = board->currentPlayer();
	maxPlayerId = board->getNumOfPlayers() - 1;
}

aiBoard::~aiBoard()
{
	kDebug() << "aiBoard destruct";
	if (deleteLines)
		delete[] lines;
}

void aiBoard::doMove(int line)
{
	// TODO: remove this check
	if (lines[line] || line < 0 || line >= linesSize)
	{
		QString lineDebug = "";
		for (int i = 0; i < linesSize; i++)
			lineDebug += lines[i] ? "1" : "0";
		kDebug() << "WARNING: adding an invalid line! line = " << line << ", lines = " << lineDebug;
		//return;
	}
	
	// check adjacent squares
	bool squareCompleted = false;
	QList<int> squares = aiFunctions::squaresFromLine(width, height, line);
	QList<int> prevBorderLines;
	for (int i = 0; i < squares.size(); i++)
	{
		prevBorderLines.append(aiFunctions::countBorderLines(width, height, squares[i], lines));
	}
	
	// add the line
	lines[line] = true;
	
	// check for completed squares
	for (int i = 0; i < squares.size(); i++)
	{
		int borderLines = aiFunctions::countBorderLines(width, height, squares[i], lines);
		if (borderLines == 4 && borderLines > prevBorderLines[i]) // found completed square
		{
			squareCompleted = true;
			squareOwners[squares[i]] = playerId;
		}
	}
	
	// cycle players
	if (!squareCompleted)
	{
		playerId++;
		if (playerId > maxPlayerId) playerId = 0;
	}
}

void aiBoard::undoMove(int line)
{
	// TODO: remove this check
	if (!lines[line] || line < 0 || line >= linesSize)
	{
		QString lineDebug = "";
		for (int i = 0; i < linesSize; i++)
			lineDebug += lines[i] ? "1" : "0";
		kDebug() << "WARNING: removing an invalid line! line = " << line << ", linesSize = " << linesSize << ", lines = " << lineDebug;
		//return;
	}
	
	// check adjacent squares
	bool squareUncompleted = false;
	QList<int> squares = aiFunctions::squaresFromLine(width, height, line);
	QList<int> prevBorderLines;
	for (int i = 0; i < squares.size(); i++)
	{
		prevBorderLines.append(aiFunctions::countBorderLines(width, height, squares[i], lines));
	}
	
	// remove line
	lines[line] = false;
	
	// check for now incomplete squares
	for (int i = 0; i < squares.size(); i++)
	{
		int borderLines = aiFunctions::countBorderLines(width, height, squares[i], lines);
		if (borderLines == 3 && prevBorderLines[i] > borderLines) // found now incomplete square
		{
			squareOwners[squares[i]]= -1;
			squareUncompleted = true;
		}
	}
	
	// cycle players
	if (!squareUncompleted)
	{
		playerId--;
		if (playerId < 0) playerId = maxPlayerId;
	}
}


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

aiBoard::aiBoard(bool *newLines, int newLinesSize, int newWidth, int newHeight, const QList<int> &newSquareOwners) : lines(newLines), linesSize(newLinesSize), width(newWidth), height(newHeight), squareOwners(newSquareOwners)
{
	
}

void aiBoard::doMove(int line, int playerId)
{
	// TODO: remove this check
	if (lines[line] || line < 0 || line >= linesSize)
	{
		kDebug() << "WARNING: adding an invalid line! line = " << line;
		//return;
	}
	
	lines[line] = true;
	
	// check for completed squares
	QList<int> squares = aiFunctions::squaresFromLine(width, height, line);
	for (int i = 0; i < squares.size(); i++)
	{
		int borderLines = aiFunctions::countBorderLines(width, height, squares[i], lines);
		if (borderLines == 4) // found completed square
		{
			squareOwners[squares[i]] = playerId;
		}
	}
}

void aiBoard::undoMove(int line)
{
	// TODO: remove this check
	if (!lines[line] || line < 0 || line >= linesSize)
	{
		kDebug() << "WARNING: removing an invalid line! line = " << line;
		//return;
	}
	
	lines[line] = false;
	
	// check for completed squares
	QList<int> squares = aiFunctions::squaresFromLine(width, height, line);
	for (int i = 0; i < squares.size(); i++)
	{
		int borderLines = aiFunctions::countBorderLines(width, height, squares[i], lines);
		if (borderLines == 3) // found now uncomplete square
		{
			squareOwners[squares[i]] = -1;
		}
	}
}
/***************************************************************************
 *   Copyright (C) 2014 by Tom Vincent Peters   <kde@vincent-peters.de>    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef AIBOARD_H
#define AIBOARD_H

#include "board.h"
#include "aistructs.h"

#include <QList>
#include <QSharedPointer>


// WARNING: take a look at the warnings on the == operator in aiBoard.cpp!
class aiBoard
{
public:
	typedef QSharedPointer<aiBoard> Ptr;
	
	aiBoard(bool *newLines, int newLinesSize, int newWidth, int newHeight, const QList<int> newSquareOwners, int playerId, int maxPlayerId);
	aiBoard(Board *board);
	~aiBoard();
	
	void doMove(int line);
	void undoMove(int line);
	
	bool *lines;
	int linesSize;
	int width;
	int height;
	QList<int> squareOwners;
	int playerId;
	int maxPlayerId;
};

#endif
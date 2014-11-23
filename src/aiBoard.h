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

#include <QList>
#include <QSharedPointer>

class aiBoard
{
public:
	typedef QSharedPointer<aiBoard> Ptr;
	
	aiBoard(bool *newLines, int newLinesSize, int newWidth, int newHeight, const QList<int> &newSquareOwners);
	
	void doMove(int line, int playerId);
	void undoMove(int line);
	
	bool *lines;
	int linesSize;
	int width;
	int height;
	QList<int> squareOwners;
};

#endif
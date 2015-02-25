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
#include <KDebug>


class aiBoard
{
public:
	typedef QSharedPointer<aiBoard> Ptr;
	
	aiBoard(bool *newLines, int newLinesSize, int newWidth, int newHeight, const QList<int> newSquareOwners, int playerId, int maxPlayerId, QList<int> *newHashLines = NULL);
	aiBoard(Board *board);
	~aiBoard();
	
	void doMove(int line);
	void undoMove(int line);
	
	bool deleteLines;
	bool *lines;
	int linesSize;
	int width;
	int height;
	QList<int> squareOwners;
	int playerId;
	int maxPlayerId;
	
	QList<int> *hashLines;
};

// the == operator and qHash are required to use an aiBoard as a key for QHash to reuse previous analysis of the board
// see http://qt-project.org/doc/qt-4.8/qhash.html#details for details

// WARNING: only the drawn lines are taken into consideration!
// WARNING: this operator doesn't care for the current player or the owned squares!
// WARNING: do not store boards of different sizes in the same QHash!
inline bool operator==(const aiBoard &b1, const aiBoard &b2)
{
	if (b1.linesSize != b2.linesSize)
		return false;
	if (b1.width != b2.width)
		return false;
	// WARNING: no checking for invalid values (width, linesSize < 0) or for height
	for (int i = 0; i < b1.linesSize; i++)
		if (b1.lines[i] != b2.lines[i])
			return false;
	return true;
}

inline bool operator==(const aiBoard::Ptr &b1, const aiBoard::Ptr &b2)
{
	if (b1->linesSize != b2->linesSize)
		return false;
	if (b1->width != b2->width)
		return false;
	// WARNING: no checking for invalid values (width, linesSize < 0) or for height
	for (int i = 0; i < b1->linesSize; i++)
		if (b1->lines[i] != b2->lines[i])
			return false;
	return true;
}

inline uint qHash(const aiBoard &key)
{
	//kDebug() << "qHash aiBoard called!";
	uint ret = 0;
	uint add = 1;
	uint i = 0;
	while (add > 0 && i < key.hashLines->size())
	{
		if (key.lines[key.hashLines->at(i)])
			ret += add;
		add <<= 1;
		i++;
	}
	return ret;
}

inline uint qHash(const aiBoard::Ptr &key)
{
	//kDebug() << "qHash aiBoard::Ptr called!";
	if (key->hashLines == NULL)
	{
		kDebug() << "ERROR: hashLines is NULL";
		return 1;
	}
	uint ret = 0;
	uint add = 1;
	uint i = 0;
	while (add > 0 && i < key->hashLines->size())
	{
		if (key->lines[key->hashLines->at(i)])
			ret += add;
		add <<= 1;
		i++;
	}
	return ret;
}

inline bool operator<(const aiBoard::Ptr &b1, const aiBoard::Ptr &b2)
{
	return qHash(b1) < qHash(b2);
}

inline bool operator<(const aiBoard &b1, const aiBoard &b2)
{
	return qHash(b1) < qHash(b2);
}

#endif
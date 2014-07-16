/***************************************************************************
 *   Copyright (C) 2014 by Tom Vincent Peters    <kde@vincent-peters.de>   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "board.h"

//kde
#include <KDebug>

Board::Board()
{
	reset();
}

Board::Board(int numOfPlayers, int width, int height)
{
	numOfPlayers_ = numOfPlayers;
	width_ = width;
	height_ = height;
	setSize(width, height);
}

void Board::reset()
{
	lineList_.clear();
	squareOwnerTable_.clear();
	lineHistory_.clear();
	numOfPlayers_ = 0;
	width_ = 0;
	height_ = 0;
	currentPlayerId_ = 0;
}

bool Board::addLine(int lineIndex, bool *nextPlayer, bool *boardFilled, QList<int> *completedSquares)
{
	if (lineIndex >= lineList_.size() || lineIndex < 0) // invalid line?
	{
		return false;
	}
	if (lineList_[lineIndex]) // line already drawn
	{
		return false;
	}
	*nextPlayer = !lineCompletesSquare(lineIndex, completedSquares); // check for completed squares first!
	lineList_[lineIndex] = true; // draw line
  // remember move
  Board::Move move;
  move.line = lineIndex;
  move.player = currentPlayerId_;
  move.squares = *completedSquares;
  lineHistory_.append(move);
  // update square owner table
	for (int i = 0; i < completedSquares->size(); i++)
	{
		squareOwnerTable_[completedSquares->at(i)] = currentPlayerId_;
	}
  // switch player?
	if (*nextPlayer) 
	{
		if (currentPlayerId_ >= numOfPlayers_-1)
		{
			currentPlayerId_ = 0;
		}
		else
		{
			currentPlayerId_++;
		}
	}
	kDebug() << "current player id: " << currentPlayerId_;
	*boardFilled = !lineList_.contains(false); // game over?
	return true;
}

bool Board::lineCompletesSquare(int lineIndex, QList<int> *completedSquares)
{
	bool squareCompleted = false;
	QList<int> adjacentSquares = squaresFromLine(lineIndex);
	QList<int> borderLineCount;
	for (int i = 0; i < adjacentSquares.size(); i++)
	{
		borderLineCount.append(countBorderLines(adjacentSquares.at(i), &lineList_));
	}
	QList<bool> lineListWithNewLine = lineList_;
	lineList_[lineIndex] = true;
	for (int i = 0; i < adjacentSquares.size(); i++)
	{
		int borderLines = countBorderLines(adjacentSquares.at(i), &lineList_);
		if (borderLines == 4 && borderLines > borderLineCount[i])
		{
			squareCompleted = true;
			completedSquares->append(adjacentSquares.at(i));
		}
	}
	lineList_[lineIndex] = false;
	return squareCompleted;
}

int Board::pointsToIndex(QPoint p1, QPoint p2)
{
	return pointsToIndex(p1, p2, width_, height_);
}

int Board::pointsToIndex(QPoint p1, QPoint p2, int w, int h)
{
	int ret = -1;
	if ( (p1-p2).manhattanLength() != 1)
	{
		kDebug() << "KSquaresIO::pointsToIndex error: manhattanLength != 1";
		return ret;
	}
	if (p1.x() > w || p1.y() > h || p2.x() > w || p2.y() > h ||
			p1.x() < 0 || p1.y() < 0 || p2.x() < 0 || p2.y() < 0
	)
	{
		kDebug() << "KSquaresIO::pointsToIndex error: invalid points";
		return ret;
	}
	
	QPoint pa, pb; // pa is the smaller point, pb the bigger
	if (p1.manhattanLength() > p2.manhattanLength())
	{
		pa = p2;
		pb = p1;
	}
	else
	{
		pa = p1;
		pb = p2;
	}
	
	if (pb.x() == pa.x())
	{
		// vertical line
		ret = w * pb.y() + (w+1) * pa.y() + pa.x();
	}
	else 
	{
		// horizontal line
		ret = pa.y() * w + pa.y() * (w+1) + pa.x();
	}
	
	kDebug() << p1 << " - " << p2 << " = " << ret;
	
	return ret;
}

// TODO: invert y axis?
bool Board::indexToPoints(const int lineIndex, QPoint *p1, QPoint *p2)
{
  int index2 = lineIndex % ( ( 2 * width_ ) + 1 );
  p1->setY( lineIndex / ( ( 2 * width_ ) + 1) );
  KSquares::Direction dir = lineDirection(lineIndex);
  if (dir == KSquares::HORIZONTAL)
  {
    p1->setX(index2);
    p2->setY(p1->y());
    p2->setX(p1->x() + 1);
  }
  else 
  {
    p1->setX(index2 - width_);
    p2->setY(p1->y() + 1);
    p2->setX(p1->x());
  }
  return true;
}

void Board::linesFromSquare(QList<int> *linesFromSquare, int squareIndex) const
{
	int index1 = (squareIndex/width_) * ((2*width_) + 1) + (squareIndex%width_);
	int index2 = index1 + width_;
	int index3 = index2 + 1;
	int index4 = index3 + width_;
	linesFromSquare->clear();
	linesFromSquare->append(index1);
	linesFromSquare->append(index2);
	linesFromSquare->append(index3);
	linesFromSquare->append(index4);
}

QList<int> Board::squaresFromLine(int lineIndex) const
{
	//kDebug() << "Line:" << lineIndex;
	QList<int> squareList;
	if (lineDirection(lineIndex) == KSquares::HORIZONTAL)
	{
		squareList.append(lineIndex - ( (width_+1) * (lineIndex/((width_*2)+1)) ));
		squareList.append(squareList.at(0) - width_);
		if(squareList.at(1) < 0)
			squareList.removeAt(1);
		if(squareList.at(0) >= (width_*height_))
			squareList.removeAt(0);
			
	}
	else if (lineDirection(lineIndex) == KSquares::VERTICAL)
	{
		squareList.append(lineIndex - ( (lineIndex/((width_*2)+1))*width_ + (lineIndex/((width_*2)+1)) + width_ ));
		squareList.append(squareList.at(0) - 1);
		if(lineIndex%((2*width_)+1) == width_)
			squareList.removeAt(1);
		if(lineIndex%((2*width_)+1) == 2*width_)
			squareList.removeAt(0);
	}
	//kDebug() << "Size:" << squareList.size();
	//kDebug() << "squares:" << squareList.at(0) << " " << squareList.at(1);
	return squareList;
}

void Board::setSize(int width, int height)
{
	reset();
	width_ = width;
	height_ = height;
	for(int i = 0; i < (2*width*height + width + height); i++)
	{
		lineList_.append(false);
	}
	for(int i = 0; i < (width*height); i++)
	{
		squareOwnerTable_.append(-1);
	}
}

int Board::countBorderLines(QList<int> *sidesOfSquare, int squareIndex, const QList<bool> *linesList) const
{
	int count = 0;
	
	linesFromSquare(sidesOfSquare, squareIndex);
	
	if(linesList->at(sidesOfSquare->at(0)) == true)
		count++;
	if(linesList->at(sidesOfSquare->at(1)) == true)
		count++;
	if(linesList->at(sidesOfSquare->at(2)) == true)
		count++;
	if(linesList->at(sidesOfSquare->at(3)) == true)
		count++;
	return count;
}

int Board::countBorderLines(int squareIndex, const QList<bool> *linesList) const
{
	QList<int> tempLineList;
	return countBorderLines(&tempLineList, squareIndex, linesList);
}

KSquares::Direction Board::lineDirection(int lineIndex) const
{
	int index2 = lineIndex % ((2*width_) + 1);
	KSquares::Direction dir;
	if(index2 < width_)
		dir = KSquares::HORIZONTAL;
	else
		dir = KSquares::VERTICAL;
	
	return dir;
}

/***************************************************************************
 *   Copyright (C) 2006 by Tom Vincent Peters   <kde@vincent-peters.de>    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "aifunctions.h"

aiFunctions::aiFunctions(int w, int h) : width(w), height(h)
{
	
}

int aiFunctions::countBorderLines(int *sidesOfSquare, int squareIndex, const bool *linesList) const
{
	int count = 0;
	
	linesFromSquare(sidesOfSquare, squareIndex);
	
	//TODO: replace this with a QList 'count' type function?
	if(linesList[sidesOfSquare[0]] == true)
		count++;
	if(linesList[sidesOfSquare[1]] == true)
		count++;
	if(linesList[sidesOfSquare[2]] == true)
		count++;
	if(linesList[sidesOfSquare[3]] == true)
		count++;
	//kDebug() << "AI: Square" << squareIndex << "is bordered by" << count << "lines";
	return count;
}

// static variant
int aiFunctions::countBorderLines(int width, int height, int *sidesOfSquare, int squareIndex, const bool *linesList)
{
	aiFunctions aif(width, height);
	return aif.countBorderLines(sidesOfSquare, squareIndex, linesList);
}

int aiFunctions::countBorderLines(int squareIndex, const bool *linesList) const
{
	int tempLineList[4];
	return countBorderLines(tempLineList, squareIndex, linesList);
}

// static variant
int aiFunctions::countBorderLines(int width, int height, int squareIndex, const bool *linesList)
{
	aiFunctions aif(width, height);
	return aif.countBorderLines(squareIndex, linesList);
}

QList<int> aiFunctions::squaresFromLine(int lineIndex) const
{
	//kDebug() << "Line:" << lineIndex;
	QList<int> squareList;
	if (lineDirection(lineIndex) == KSquares::HORIZONTAL)
	{
		squareList.append(lineIndex - ( (width+1) * (lineIndex/((width*2)+1)) ));
		squareList.append(squareList.at(0) - width);
		if(squareList.at(1) < 0)
			squareList.removeAt(1);
		if(squareList.at(0) >= (width*height))
			squareList.removeAt(0);
			
	}
	else if (lineDirection(lineIndex) == KSquares::VERTICAL)
	{
		squareList.append(lineIndex - ( (lineIndex/((width*2)+1))*width + (lineIndex/((width*2)+1)) + width ));
		squareList.append(squareList.at(0) - 1);
		if(lineIndex%((2*width)+1) == width)
			squareList.removeAt(1);
		if(lineIndex%((2*width)+1) == 2*width)
			squareList.removeAt(0);
	}
	//kDebug() << "Size:" << squareList.size();
	//kDebug() << "squares:" << squareList.at(0) << " " << squareList.at(1);
	return squareList;
}

// static variant
QList<int> aiFunctions::squaresFromLine(int width, int height, int lineIndex)
{
	aiFunctions aif(width, height);
	return aif.squaresFromLine(lineIndex);
}

void aiFunctions::linesFromSquare(int *linesFromSquare, int squareIndex) const
{
	int index1 = (squareIndex/width) * ((2*width) + 1) + (squareIndex%width);
	int index2 = index1 + width;
	int index3 = index2 + 1;
	int index4 = index3 + width;
	linesFromSquare[0] = index1;
	linesFromSquare[1] = index2;
	linesFromSquare[2] = index3;
	linesFromSquare[3] = index4;
}

// static variant
void aiFunctions::linesFromSquare(int width, int height, int *linesFromSquare, int squareIndex)
{
	aiFunctions aif(width, height);
	return aif.linesFromSquare(linesFromSquare, squareIndex);
}

KSquares::Direction aiFunctions::lineDirection(int lineIndex) const
{
	int index2 = lineIndex % ((2*width) + 1);
	KSquares::Direction dir;
	if(index2 < width)
		dir = KSquares::HORIZONTAL;
	else
		dir = KSquares::VERTICAL;
	
	return dir;
}

// static variant
KSquares::Direction aiFunctions::lineDirection(int width, int height, int lineIndex)
{
	aiFunctions aif(width, height);
	return aif.lineDirection(lineIndex);
}

/*=========================================*/

QList<int> aiFunctions::findLinesCompletingBoxes(int linesSize, const bool *lines) const
{
	QList<int> choiceList;
	for(int i=0; i<linesSize; i++)	//trying to get points. looking for squares with 3 lines
	{
		if(!lines[i])
		{
			QList<int> adjacentSquares = squaresFromLine(i);
			for(int j=0; j<adjacentSquares.size(); j++)
			{
				if(countBorderLines(adjacentSquares.at(j), lines) == 3)	//if 3 lines, draw there to get points!
				{
					choiceList.append(i);
					//kDebug() << "AI: 1. Adding" << i << "to choices";
				}
			}
		}
	}
	return choiceList;
}

// static variant
QList<int> aiFunctions::findLinesCompletingBoxes(int width, int height, int linesSize, const bool *lines)
{
	aiFunctions aif(width, height);
	return aif.findLinesCompletingBoxes(linesSize, lines);
}

QList<int> aiFunctions::safeMoves(int linesSize, const bool *lines) const
{
	QList<int> safeLines;
	for(int i=0; i<linesSize; i++)	//finding totally safe moves. avoiding squares with 2 lines
	{
		if(!lines[i])
		{
			QList<int> adjacentSquares = squaresFromLine(i);
			int badCount = 0;
			for(int j=0; j<adjacentSquares.size(); j++)
			{
				if(countBorderLines(adjacentSquares.at(j), lines) == 2)	//don't want to make 3 lines around a square
				{
					badCount++;
				}
			}
			if(badCount == 0)
			{
				safeLines.append(i);
				//kDebug() << "AI: 2. Adding" << i << "to choices";
			}
		}
	}
	return safeLines;
}

QList<int> aiFunctions::safeMoves(int width, int height, int linesSize, const bool *lines)
{
	aiFunctions aif(width, height);
	return aif.safeMoves(linesSize, lines);
}

void aiFunctions::findOwnChains(bool *lines, int linesSize, int width, int height, QList<QList<int> > *ownChains) const
{
	int sidesOfSquare[4];
	QScopedArrayPointer<bool> myLines(new bool[linesSize]); //make temporary local copies of lists
	int ownLinesCnt = 0; // count of how many lines ai will take in this run
	int ownSquaresCnt = 0; // count of how many squares ai will get in this run
	memcpy(myLines.data(), lines, linesSize); // lines --> myLines (complete own chains) --> linesCopy (analyze damage/chains for next runs)
	bool chainFound;
	// since chooseLeastDamaging will be called early during the game if playing against hard ai, we need to finish open chains in linesCopy before computing the number of residual chains
	do // this loop completes all chains the opponent gave to ai
	{
		chainFound = false;
		for(int curSquare = 0; curSquare < width*height; curSquare++) // walk through squares and search for start of chain
		{
			QList<int> ownChain; // remember completed chain lines
			int chainSquare = curSquare;
			bool squareFound;
			do { // this loop walks through a chain square by square
				squareFound = false;
				if(countBorderLines(sidesOfSquare, chainSquare, &(*myLines)) == 3) // found a square for ai
				{
					for(int sideOfSquare = 0; sideOfSquare <= 3; sideOfSquare++)
					{
						if(!myLines[sidesOfSquare[sideOfSquare]]) // found missing line of square
						{
							ownLinesCnt++;
							
							int nextSquareFound=-1;
							QList<int> adjacentSquares = squaresFromLine(sidesOfSquare[sideOfSquare]);
							for(int i = 0; i < adjacentSquares.size(); i++)
							{
								int chainSquareBorderCnt = countBorderLines(adjacentSquares.at(i), &(*myLines));
								if(chainSquare != adjacentSquares.at(i) &&
										chainSquareBorderCnt == 3)	// check if a second square will be completed by this line
								{
									ownSquaresCnt++; // add extra square
								}
								if(chainSquareBorderCnt == 2)	// look for next square in chain
								{
									nextSquareFound = adjacentSquares.at(i);
								}
									
							}
							myLines[sidesOfSquare[sideOfSquare]] = true; // complete line
							if(nextSquareFound >= 0)
							{
								chainSquare = nextSquareFound;
							}
							ownChain.append(sidesOfSquare[sideOfSquare]);
						}
					}
					squareFound = true;
					chainFound = true;
					ownSquaresCnt++;
				}
			} while(squareFound);
			if(chainFound)
			{
				ownChains->append(ownChain);
				break;
			}
		}
	} while (chainFound);
}

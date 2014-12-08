/***************************************************************************
 *   Copyright (C) 2014 by Tom Vincent Peters   <kde@vincent-peters.de>    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "aifunctions.h"
#include <KDebug>
#include <QStack>
#include <algorithm>

aiFunctions::aiFunctions(int w, int h) : width(w), height(h)
{
	linesSize = toLinesSize(width, height);
}

int aiFunctions::toLinesSize(int w, int h)
{
	return 2 * w * h + w + h;
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
					if (!choiceList.contains(i))
					{
						choiceList.append(i);
						//kDebug() << "AI: 1. Adding" << i << "to choices";
					}
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

int aiFunctions::findOwnChains(bool *lines, int linesSize, int width, int height, QList<QList<int> > *ownChains)
{
  /*
	kDebug() << "find own chains" << linesSize << ", " << width << ", " << height;
	QString linesStr;
	for (int i = 0; i < linesSize; i++)
		linesStr += lines[i]?"1":"0";
	kDebug() << "lines:" <<linesStr;
  */
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
				if(countBorderLines(width, height, sidesOfSquare, chainSquare, &(*myLines)) == 3) // found a square for ai
				{
					//kDebug() << "found square:" << chainSquare;
					for(int sideOfSquare = 0; sideOfSquare <= 3; sideOfSquare++)
					{
						if(!myLines[sidesOfSquare[sideOfSquare]]) // found missing line of square
						{
							ownLinesCnt++;
							
							int nextSquareFound=-1;
							QList<int> adjacentSquares = squaresFromLine(width, height, sidesOfSquare[sideOfSquare]);
							for(int i = 0; i < adjacentSquares.size(); i++)
							{
								int chainSquareBorderCnt = countBorderLines(width, height, adjacentSquares.at(i), &(*myLines));
								if(chainSquare != adjacentSquares.at(i) &&
										chainSquareBorderCnt == 3)	// check if a second square will be completed by this line
								{
                  //kDebug() << "found double cross between " << chainSquare << " and " << adjacentSquares.at(i);
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
				//std::sort(ownChain.begin(), ownChain.end());
				//qSort(ownChain);
				//if (!ownChains->contains(ownChain))
					ownChains->append(ownChain);
				break;
			}
		}
	} while (chainFound);
  
	//qSort(ownChains);
	//kDebug() << "findOwnChains returns: " << *ownChains;
  return ownSquaresCnt;
}


void aiFunctions::findChains(bool *lines, int linesSize, int width, int height, QList<KSquares::Chain> *foundChains)
{
	QList<QList<int> > ownChains;
	// find opened chains
	aiFunctions::findOwnChains(lines, linesSize, width, height, &ownChains);
	for (int i = 0; i < ownChains.size(); i++)
	{
		KSquares::Chain foundChain;
		foundChain.lines = ownChains[i];
		foundChain.type = classifyChain(width, height, ownChains[i], lines, &(foundChain.squares));
		foundChain.ownChain = true;
		
		foundChains->append(foundChain);
		
		for (int j = 0; j < ownChains[i].size(); j++)
		{
			lines[ownChains[i][j]] = true;
		}
	}
	
	// find chains
	QList<int> freeLines = aiFunctions::getFreeLines(lines, linesSize);
	QList<QList<int> > chains;
	QList<QList<int> > chainList;
	QList<QList<int> > chainSet;
	
	while (!freeLines.isEmpty())
	{
		int line = freeLines.takeLast();
		lines[line] = true;
		chains.clear();
		aiFunctions::findOwnChains(lines, linesSize, width, height, &chains);
		
		for (int i = 0; i < chains.size(); i++)
		{
			for (int j = 0; j < chains[i].size(); j++)
			{
				freeLines.removeAll(chains[i][j]);
			}
			chains[i].append(line);
			std::sort(chains[i].begin(), chains[i].end());
			chainList.append(chains[i]);
			//kDebug() << "chain: " << chains[i];
		}
		
		lines[line] = false;
	}
	
	// TODO: required?
	// remove duplicate chains
	for (int i = 0; i < chainList.size(); i++)
	{
		bool newChain = true;
		QList<int> chainCheck = chainList[i]; // this is the chain we might add
		for (int j = 0; j < chainSet.size(); j++)
		{
			if(chainSet[j] == chainCheck) // found chainCheck in chainSet, don't add
			{
				newChain = false;
				break;
			}
		}
		if (newChain) // chainCheck not in chainSet
		{
			chainSet.append(chainCheck);
		}
	}
	
	// analyse chains
	for (int i = 0; i < chainSet.size(); i++)
	{
		KSquares::Chain foundChain;
		foundChain.lines = chainSet[i];
		foundChain.type = classifyChain(width, height, chainSet[i], lines, &(foundChain.squares));
		foundChain.ownChain = false;
		
		foundChains->append(foundChain);
	}
	
	// cleanup - undo ownChain!
	for (int i = 0; i < ownChains.size(); i++)
	{
		for (int j = 0; j < ownChains[i].size(); j++)
		{
			lines[ownChains[i][j]] = false;
		}
	}
}


QList<int> aiFunctions::getFreeLines(bool *lines, int linesSize)
{
	QList<int> freeLines;
	for (int i = 0; i < linesSize; i++)
	{
		if (!lines[i])
			freeLines.append(i);
	}
	return freeLines;
}

QMap<int, int> aiFunctions::getScoreMap(QList<int> &squareOwners)
{
	QMap<int, int> scores; // index = player id, value = number of squares
	for (int i = 0; i < squareOwners.size(); i++)
	{
		if (scores.contains(squareOwners[i]))
			scores[squareOwners[i]] ++;
		else
			scores[squareOwners[i]] = 1;
	}
	return scores;
}

/**
* Determines which player has the most squares
* @return playerId of player with most squares, -1 if no squares are drawn, -2 if draw, -3 if sth went wrong
*/
int aiFunctions::getLeader(QList<int> &squareOwners)
{
	QMap<int, int> scores; // index = player id, value = number of squares
	scores = getScoreMap(squareOwners);
	//kDebug() << "score map: " << scores;
	if (scores.contains(-1) && scores.keys().size() == 1) // no squares are drawn
		return -1;
	int bestId = -3;
	int bestScore = 0;
	bool draw = false;
	for (int i = 0; i < scores.keys().size(); i++)
	{
		if (scores.keys()[i] == -1) // square not taken
			continue;
		if (scores[scores.keys()[i]] == bestScore)
			draw = true;
		if (scores[scores.keys()[i]] > bestScore)
		{
			draw = false;
			bestId = scores.keys()[i];
			bestScore = scores[scores.keys()[i]];
		}
	}
	if (bestId == -3)
		kDebug() << "sth went wrong when calculating the board leader!";
	if (draw)
		return -2;
	return bestId;
}

QString aiFunctions::boardToString(bool *lines, int linesSize, int width, int height)
{
	QString ret = "\n+";
	
	for (int i = 0; i < linesSize; i++)
	{
		KSquares::Direction iDirection = lineDirection(width, height, i);
		KSquares::Direction nextDirection = lineDirection(width, height, i+1);
		if (iDirection != nextDirection && nextDirection == KSquares::VERTICAL)
		{
			//ret.append("\n");
		}
		if (iDirection == KSquares::HORIZONTAL)
		{
			ret.append(lines[i] ? "--+" : "  +");
		}
		else
		{
			ret.append(lines[i] ? "|  " : "   ");
		}
		if (iDirection != nextDirection)
		{
			ret.append(nextDirection == KSquares::HORIZONTAL ? "\n+" : "\n");
		}
	}
	
	ret.replace("  \n", "\n");
	return ret;
}

QString aiFunctions::boardToString(bool *lines) const
{
  return boardToString(lines, linesSize, width, height);
}

QString aiFunctions::linelistToString(const QList<int> list, int linesSize, int width, int height)
{
  bool lines[linesSize];
  for (int i = 0; i < linesSize; i++)
  {
    if (list.contains(i))
    {
      lines[i] = true;
    }
    else
    {
      lines[i] = false;
    }
  }
  return boardToString(lines, linesSize, width, height);
}

QString aiFunctions::linelistToString(const QList<int> list) const
{
  return linelistToString(list, linesSize, width, height);
}

void printVisitedSquares(bool *squares, int width, int height)
{
  QString board = "";
  for (int i = 0; i < width * height; i++)
  {
    board.append(i % width == 0?"\n":"");
    board.append(squares[i]?"x":"o");
  }
  kDebug() << "Visited squares: " << board;
}

void printSquares(QList<int> squares, int width, int height)
{
  QString board ="";
  for (int i = 0; i < width * height; i++)
  {
    board.append(i % width == 0?"\n":"");
    board.append(squares.contains(i) ? "x":"o");
  }
  kDebug() << "Squares: " << board;
}

// @param chain: chain lines
// @return 0: long chain, 1: short chain, 2: loop chain, -1: no chain
KSquares::ChainType aiFunctions::classifyChain(int width, int height, const QList<int> &chain, bool *lines)
{
	QList<int> squares;
	return classifyChain(width, height, chain, lines, &squares);
}

KSquares::ChainType aiFunctions::classifyChain(int width, int height, const QList<int> &chain, bool *lines, QList<int> *squares)
{
  if (chain.size() <= 0)
  {
    kDebug() << "ERROR: classifyChain called with no chain lines";
    return KSquares::CHAIN_UNKNOWN;
  }
  
  // get all squares of the chain
  for (int i = 0; i < chain.size(); i++)
  {
    if (lines[chain[i]])
    {
      kDebug() << "ERROR: classifyChain called with incorrect chain parameter (line " << chain[i] << " is drawn!)";
      return KSquares::CHAIN_UNKNOWN;
    }
    QList<int> curSquares = squaresFromLine(width, height, chain[i]);
    for (int j = 0; j < curSquares.size(); j++)
    {
      if (squares->contains(curSquares[j]))
        continue;
      if (countBorderLines(width, height, curSquares[j], lines) < 2)
        continue;
      squares->append(curSquares[j]);
    }
  }
  //printSquares(squares, width, height);
  
  // no squares -> no chain
  if (squares->size() < 1)
  {
    return KSquares::CHAIN_UNKNOWN;
  }
  
  // look for square loops
  QStack<int> squareQueue;
  QList<int> squareVisited;
  QList<int> linesVisited;
  
  squareQueue.push(squares->at(0));
  while (squareQueue.size() > 0)
  {
    int curSquare = squareQueue.pop();
    
    // has square already been visited?
    if (squareVisited.contains(curSquare))
    {
      //kDebug() << "visiting an square that has already been visited! (" << curSquare << ", queue = " << squareVisited << ")";
      return KSquares::CHAIN_LOOP;
    }
    squareVisited.append(curSquare);
    
    // find connected squares for curSquare
    int curLines[4];
    linesFromSquare(width, height, curLines, curSquare);
    for (int i = 0; i < 4; i++)
    {
      int curLine = curLines[i];
      // check if curLine is part of chain (can't be drawn due to previous check)
      if (!chain.contains(curLine))
        continue;
      
      if (linesVisited.contains(curLine))
        continue;
      linesVisited.append(curLine);
      
      // get the bordering squares of curLine
      QList<int> curLineSquares = squaresFromLine(width, height, curLine);
      for (int j = 0; j < curLineSquares.size(); j++)
      {
        // check if the square isn't the one we are expanding
        if (curSquare == curLineSquares[j]) 
          continue;
        if (countBorderLines(width, height, curLineSquares[j], lines) < 2)
          continue;
        squareQueue.push(curLineSquares[j]);
      }
    }
  }
  
  // did we visit all squares?
  if (squares->size() != squareVisited.size())
  {
    kDebug() << "ERROR: didn't visit all squares (squares cnt = " << squares->size() << ", squares visited cnt = " << squareVisited.size() << ")";
    return KSquares::CHAIN_UNKNOWN;
  }
  
  // how many chains did we visit? two or less = short chain
  if (squareVisited.size() <= 2)
  {
    return KSquares::CHAIN_SHORT;
  }
  
  // long chain
  return KSquares::CHAIN_LONG; 
}

KSquares::ChainType aiFunctions::classifyChain(const QList<int> &chain, bool *lines) const
{
	return classifyChain(width, height, chain, lines);
}

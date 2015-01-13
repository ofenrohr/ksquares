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


QList<KSquares::LSConnection > aiFunctions::getConnectedSquares(aiBoard::Ptr board, int square)
{
	QList<KSquares::LSConnection > connectedSquares;
	
	int squareLines[4];
	aiFunctions::linesFromSquare(board->width, board->height, squareLines, square);
	for (int i = 0; i < 4; i++)
	{
		if (board->lines[squareLines[i]])
			continue;
		QList<int> lineSquares = aiFunctions::squaresFromLine(board->width, board->height, squareLines[i]);
		for (int j = 0; j < lineSquares.size(); j++)
		{
			if (lineSquares[j] == square)
				continue;
			KSquares::LSConnection connectedSquare(squareLines[i], lineSquares[j]);
			connectedSquares.append(connectedSquare);
		}
	}
	
	return connectedSquares;
}


bool aiFunctions::squareConnectedToJoint(aiBoard::Ptr board, QMap<int, int> &squareValences, int square, bool checkJointInCycle)
{
	int squareLines[4];
	aiFunctions::linesFromSquare(board->width, board->height, squareLines, square);
	for (int i = 0; i < 4; i++)
	{
		if (board->lines[squareLines[i]])
			continue;
		QList<int> lineSquares = aiFunctions::squaresFromLine(board->width, board->height, squareLines[i]);
		if (getGroundConnections(board, square).size() >= 1)
		{
			kDebug() << "square " << square << " connected to ground joint";
			return true;
		}
		for (int j = 0; j < lineSquares.size(); j++)
		{
			if (squareValences[lineSquares[j]] < 2)
			{
				if (checkJointInCycle && jointInCycle(board, lineSquares[j], square, squareValences))
					continue;
				kDebug() << "square " << square << " connected to inner joint";
				return true;
			}
		}
	}
	return false;
}


// @return list of connections + type of connection: true = ground connection, false = inner joint connection; connections to ground have square value of -1
QList<QPair<KSquares::LSConnection, bool> > aiFunctions::getConnectionsToJoints(aiBoard::Ptr board, QMap<int, int> &squareValences, int square, bool checkJointInCycle)
{
	QList<QPair<KSquares::LSConnection, bool> > ret;
	int squareLines[4];
	aiFunctions::linesFromSquare(board->width, board->height, squareLines, square);
	for (int i = 0; i < 4; i++)
	{
		if (board->lines[squareLines[i]])
			continue;
		QList<int> lineSquares = aiFunctions::squaresFromLine(board->width, board->height, squareLines[i]);
		QList<int> groundConnections = getGroundConnections(board, square);
		for (int j = 0; j < groundConnections.size(); j++)
		{
			kDebug() << "square " << square << " connected to ground joint via line " << groundConnections[j];
			KSquares::LSConnection connection(groundConnections[j], -1);
			ret.append(QPair<KSquares::LSConnection, bool>(connection, true));
		}
		for (int j = 0; j < lineSquares.size(); j++)
		{
			if (squareValences[lineSquares[j]] < 2)
			{
				if (checkJointInCycle && jointInCycle(board, lineSquares[j], square, squareValences))
					continue;
				kDebug() << "square " << square << " connected to inner joint square " << lineSquares[j] << " via line " << squareLines[i];
				KSquares::LSConnection connection(squareLines[i], lineSquares[j]);
				ret.append(QPair<KSquares::LSConnection, bool>(connection, false));
				//return true;
			}
		}
	}
	return ret;
}


// Source: http://stackoverflow.com/questions/1339121/how-to-reverse-a-qlist
template <typename T>
QList<T> aiFunctions::reverseQList( const QList<T> & in ) {
    QList<T> result;
    result.reserve( in.size() ); // reserve is new in Qt 4.7
    std::reverse_copy( in.begin(), in.end(), std::back_inserter( result ) );
    return result;
}


QString chainTypeToString(KSquares::ChainType t)
{
	switch (t)
	{
		case KSquares::CHAIN_SHORT: return "short";
		case KSquares::CHAIN_LONG: return "long";
		case KSquares::CHAIN_LOOP: return "loop";
		case KSquares::CHAIN_UNKNOWN: return "unknown";
		default: return "unknown (switch)";
	}
}

QList<int> aiFunctions::getGroundConnections(aiBoard::Ptr board, int square, bool includeCutConnections)
{
	QList<int> groundConnections;
	int squareLines[4];
	aiFunctions::linesFromSquare(board->width, board->height, squareLines, square);
	for (int i = 0; i < 4; i++)
	{
		if (aiFunctions::squaresFromLine(board->width, board->height, squareLines[i]).size() == 1)
			if (includeCutConnections || !board->lines[squareLines[i]])
				groundConnections.append(squareLines[i]);
	}
	return groundConnections;
}

bool aiFunctions::jointInCycle(aiBoard::Ptr board, int joint, int start, QMap<int, int> &squareValences)
{
	QList<int> squaresVisited;
	
	if (squareValences[start] != 2)
		return false;
	
	if (joint == start)
	{
		kDebug() << "WARNING: jointInCycle called with wrong parameter! joint == start";
		return false;
	}
	
	int square = start;
	bool foundNextSquare = false;
	do
	{
		foundNextSquare = false;
		squaresVisited.append(square);
		
		QList<KSquares::LSConnection > connectedSquares = getConnectedSquares(board, square);
		for (int i = 0; i < connectedSquares.size(); i++)
		{
			if (squaresVisited.contains(connectedSquares[i].square))
				continue;
			if (connectedSquares[i].square == joint && square != start)
				return true;
			if (squareValences[connectedSquares[i].square] == 2)
			{
				square = connectedSquares[i].square;
				foundNextSquare = true;
				break;
			}
		}
	} while (foundNextSquare);
	
	return false;
}

/*
void aiFunctions::findChains(aiBoard::Ptr board, QList<KSquares::Chain> *foundChains)
{
	QMap<int, int> squareValences; // square, valence (WARNING: not really the valence, it's the count of border lines!)
	QList<int> startSquares;
	QList<int> chainSquares; // squares with valence == 2
	
	for (int i = 0; i < board->squareOwners.size(); i++)
	{
		if (board->squareOwners[i] == -1)
		{
			squareValences[i] = aiFunctions::countBorderLines(board->width, board->height, i, board->lines);
			if (squareValences[i] < 2) // joint squares
				startSquares.append(i);
			if (squareValences[i] == 3)
				startSquares.append(i);
			if (squareValences[i] == 2)
				chainSquares.append(i);
		}
	}
	
	while 
}
*/

void aiFunctions::findChains(aiBoard::Ptr board, QList<KSquares::Chain> *foundChains)
{
	/*
	QList<QList<int> > ownChains;
	findOwnChains(board->lines, board->linesSize, board->width, board->height, &ownChains);
	for (int i = 0; i < ownChains.size(); i++)
	{
		KSquares::Chain foundChain;
		foundChain.lines = ownChains[i];
		foundChain.type = classifyChain(board->width, board->height, ownChains[i], board->lines, &(foundChain.squares));
		if (foundChain.type == KSquares::CHAIN_LONG &&
			countBorderLines(board->width, board->height, foundChain.squares[0], board->lines) &&
			countBorderLines(board->width, board->height, foundChain.squares[foundChain.squares.size() - 1], board->lines)
		)
			foundChain.type = KSquares::CHAIN_LOOP;
		foundChain.ownChain = true;
		for (int j = 0; j < ownChains[i].size(); j++)
		{
			board->lines[ownChains[i][j]] = true;
		}
		for (int j = 0; j < foundChain.squares.size(); j++)
		{
			board->squareOwners[foundChain.squares[j]] = -2;
		}
		foundChains->append(foundChain);
	}
	*/
	
	QMap<int, int> squareValences; // square, valence (WARNING: not really the valence, it's the count of border lines!)
	
	// find untaken squares and calculate valence
	QList<int> freeSquares;
	for (int i = 0; i < board->squareOwners.size(); i++)
	{
		if (board->squareOwners[i] == -1)
		{
			squareValences[i] = aiFunctions::countBorderLines(board->width, board->height, i, board->lines);
			if (squareValences[i] < 2) // ignore joints
				continue;
			freeSquares.append(i);
		}
	}
	
	// look for chains
	//QList<QList<int> > capturableChains;
	//QList<QList<int> > uncapturableChains;
	while (freeSquares.size() > 0)
	{
		int square = freeSquares.takeLast();
		kDebug() << "square: " << square;
		/*
		if (squareValences[square] == 2 && squareConnectedToJoint(board, squareValences, square))
		{
			kDebug() << "square connected to joint: " << square;
		}
		*/
		
		if (squareValences[square] == 3 || (squareValences[square] == 2 && squareConnectedToJoint(board, squareValences, square)))
		{
			QList<int> chain;
			bool canCapture = squareValences[square] == 3;
			
			/*
			QList<int> startGroundConnections = getGroundConnections(board, square);
			if (startGroundConnections.size() > 0) // square connected to ground
			{
				chain.append(startGroundConnections);
				//squareValences[square] = squareValences[square] + startGroundConnections.size();
			}
			*/
			
			//bool foundSquare = true;
			int expandingSquare = square;
			QStack<int> squareQueue;
			squareQueue.push(square);
			while (squareQueue.size() > 0)
			{
				expandingSquare = squareQueue.pop();
				//foundSquare = false;
				kDebug() << "expandingSquare: " << expandingSquare;
				// check for ground connections
				QList<int> groundConnections = getGroundConnections(board, expandingSquare);
				for (int i = 0; i < groundConnections.size(); i++)
				{
					kDebug() << "ground connection for square " << expandingSquare << ": " << groundConnections[0];
					chain.append(groundConnections[i]);
				}
				// look for next squares in chain
				QList<KSquares::LSConnection > connectedSquares = getConnectedSquares(board, expandingSquare);
				kDebug() << "connectedSquares: " << connectedSquares;
				for (int i = 0; i < connectedSquares.size(); i++)
				{
					if (chain.contains(connectedSquares[i].line))
						continue;
					
					if (squareValences[connectedSquares[i].square] == 2) // next square continues chain
					{
						chain.append(connectedSquares[i].line);
						squareQueue.push(connectedSquares[i].square);
						freeSquares.removeAll(connectedSquares[i].square); // this square is a part of a single chain and doesn't need to be evaluated as possible start of another chain
						continue;
					}
					
					if (squareValences[connectedSquares[i].square] < 2) // next square is a joint
					{
						int connectedJointSquare = connectedSquares[i].square;
						QList<KSquares::LSConnection> jointConnections = getConnectedSquares(board, connectedJointSquare);
						bool jointReachedBefore = false;
						for (int j = 0; j < jointConnections.size(); j++)
						{
							if (jointConnections[j].square == expandingSquare) // this is the square we're coming from now
								continue;
							
							if (chain.contains(jointConnections[j].line)) // the chain has reached the joint before
								jointReachedBefore = true;
						}
						if (jointReachedBefore) // the joint is part of a cycle and won't stop the chain
						{
							squareQueue.push(connectedJointSquare);
						}
						// add the connection to the joint we're coming from
						chain.append(connectedSquares[i].line);
					}
					/*
					if (squareConnectedToJoint(board, squareValences, expandingSquare) && 
						expandingSquare != square
					)
					{
						chain.append(connectedSquares[i].line);
						kDebug() << "end of chain: " << expandingSquare << ", connectedSquares[i] = (" << connectedSquares[i].line << "|" << connectedSquares[i].square << "), expandingSquare = " << expandingSquare << ", square = " << square;
					}
					else
					{
						chain.append(connectedSquares[i].line);
						expandingSquare = connectedSquares[i].square;
						freeSquares.removeAll(expandingSquare);
						//foundSquare = true;
						//if (squareValences[expandingSquare] >= 2)
							squareQueue.push(expandingSquare);
						kDebug() << "pushing square " << expandingSquare;
					}
					//squareValences[expandingSquare] = squareValences[expandingSquare] + 1;
					//squareValences[connectedSquares[i].square] = squareValences[connectedSquares[i].square] + 1;
					*/
				}
			}
			
			bool canCaptureFromBothEnds = false;
			if (squareValences[expandingSquare] == 3)
			{
				chain = reverseQList(chain);
				if (canCapture)
					canCaptureFromBothEnds = true;
				canCapture = true;
			}
			
			//capturableChains.append(chain);
			KSquares::Chain foundChain;
			foundChain.lines = chain;
			foundChain.type = classifyChain(board->width, board->height, chain, board->lines, &(foundChain.squares)); // TODO: integrate classification!
			if (canCaptureFromBothEnds && foundChain.type == KSquares::CHAIN_LONG)
				foundChain.type = KSquares::CHAIN_LOOP;
			
			if (canCapture)
				foundChain.ownChain = true;
			else
				foundChain.ownChain = false;
			
			//kDebug() << "found chain:" << chain << "squares:" << foundChain.squares << "cap:" << canCapture << "type:" << chainTypeToString(foundChain.type) << " chain: " << linelistToString(chain, board->linesSize, board->width, board->height);
			foundChains->append(foundChain);
		}
	}
	
	// undo the taken own chains
	/*
	for (int i = 0; i < foundChains->size(); i++)
	{
		if (!foundChains->at(i).ownChain)
			continue;
		for (int j = 0; j < foundChains->at(i).lines.size(); j++)
			board->lines[foundChains->at(i).lines[j]] = false;
		for (int j = 0; j < foundChains->at(i).squares.size(); j++)
			board->squareOwners[foundChains->at(i).squares[j]] = -1;
	}
	*/
}

/*
void findChainsOld(bool *lines, int linesSize, int width, int height, QList<KSquares::Chain> *foundChains)
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
*/

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
  
  QMap<int, int> squareReached; // square index, times the square was reached - used for joint squares
  
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
			squareReached[curSquares[j]] ++;
			if (countBorderLines(width, height, curSquares[j], lines) < 2 && squareReached[curSquares[j]] != 2)
        continue;
      squares->append(curSquares[j]);
    }
  }
  squareReached.clear();
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
    //if (countBorderLines(width, height, curSquare, lines) >= 2)
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
				squareReached[curLineSquares[j]] ++;
        if (countBorderLines(width, height, curLineSquares[j], lines) < 2 && squareReached[curLineSquares[j]] != 2)
          continue;
        squareQueue.push(curLineSquares[j]);
      }
    }
  }
  
  // did we visit all squares?
  if (squares->size() != squareVisited.size())
  {
    kDebug() << "ERROR: didn't visit all squares (squares cnt = " << squares->size() << ", squares visited cnt = " << squareVisited.size() << "), board: " << boardToString(lines, toLinesSize(width, height), width, height) << "chain: " << chain << " as board: " << linelistToString(chain, toLinesSize(width, height), width, height);
		printSquares(squareVisited, width, height);
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

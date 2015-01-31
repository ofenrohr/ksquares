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


QList<KSquares::LSConnection > aiFunctions::getConnectedSquares(aiBoard *board, int square)
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


bool aiFunctions::squareConnectedToJoint(aiBoard *board, QMap<int, int> &squareValences, int square, bool checkJointInCycle)
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
			//kDebug() << "square " << square << " connected to ground joint";
			return true;
		}
		for (int j = 0; j < lineSquares.size(); j++)
		{
			if (squareValences[lineSquares[j]] < 2)
			{
				if (checkJointInCycle && jointInCycle(board, lineSquares[j], square, squareValences))
					continue;
				//kDebug() << "square " << square << " connected to inner joint";
				return true;
			}
		}
	}
	return false;
}


// @return list of connections + type of connection: true = ground connection, false = inner joint connection; connections to ground have square value of -1
QList<QPair<KSquares::LSConnection, bool> > aiFunctions::getConnectionsToJoints(aiBoard *board, QMap<int, int> &squareValences, int square, bool checkJointInCycle)
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
			//kDebug() << "square " << square << " connected to ground joint via line " << groundConnections[j];
			KSquares::LSConnection connection(groundConnections[j], -1);
			ret.append(QPair<KSquares::LSConnection, bool>(connection, true));
		}
		for (int j = 0; j < lineSquares.size(); j++)
		{
			if (squareValences[lineSquares[j]] < 2)
			{
				if (checkJointInCycle && jointInCycle(board, lineSquares[j], square, squareValences))
					continue;
				//kDebug() << "square " << square << " connected to inner joint square " << lineSquares[j] << " via line " << squareLines[i];
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

QList<int> aiFunctions::getGroundConnections(aiBoard *board, int square, bool includeCutConnections)
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

bool aiFunctions::jointInCycle(aiBoard *board, int joint, int start, QMap<int, int> &squareValences)
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


void aiFunctions::findChains(aiBoard *board, QList<KSquares::Chain> *foundChains, bool onlyOwnChains)
{
	QMap<int, int> squareValences; // square, valence (WARNING: not really the valence, it's the count of border lines!)
	
	// find untaken squares and calculate valence
	QList<int> freeSquares;
	QList<int> unvisitedSquares;
	QList<int> excludeFromRemovalOnce;
	QList<int> removedOnce;
	QMap<int, int> jointReachedBefore; // square index, times this joint has been reached
	for (int i = 0; i < board->squareOwners.size(); i++)
	{
		if (board->squareOwners[i] == -1)
		{
			squareValences[i] = aiFunctions::countBorderLines(board->width, board->height, i, board->lines);
			if (squareValences[i] < 2) // ignore joints
			{
				jointReachedBefore[i] = 0;
				continue;
			}
			freeSquares.append(i);
		}
	}
	
	// look for chains
	while (freeSquares.size() > 0 || unvisitedSquares.size() > 0)
	{
		int square = -1;
		if (freeSquares.size() > 0)
			square = freeSquares.takeLast();
		else
			square = unvisitedSquares.takeLast();
		//kDebug() << "square: " << square;
		
		if (
			squareValences[square] == 3 || 
			(squareValences[square] == 2 && squareConnectedToJoint(board, squareValences, square)) ||
			(freeSquares.size() == 0 && squareValences[square] == 2) // loop chains don't have a beginning
		)
		{
			QList<int> chain;
			QList<int> chainSquares;
			bool canCapture = squareValences[square] == 3;
			bool passedJoint = false; // joint square is part of loop and connects to another chain (important if that other chain can be captured)
			
			//bool foundSquare = true;
			int expandingSquare = square;
			QList<int> squareQueue;
			squareQueue.append(square);
			int localJointReachedBefore = 0; // times the joint has been reached by this chain
			while (squareQueue.size() > 0)
			{
				expandingSquare = squareQueue.takeLast();
				//foundSquare = false;
				//kDebug() << "expandingSquare: " << expandingSquare;
				// check for ground connections
				QList<int> groundConnections = getGroundConnections(board, expandingSquare);
				for (int i = 0; i < groundConnections.size(); i++)
				{
					//kDebug() << "ground connection for square " << expandingSquare << ": " << groundConnections[0];
					chain.append(groundConnections[i]);
				}
				// look for next squares in chain
				QList<KSquares::LSConnection> connectedSquares = getConnectedSquares(board, expandingSquare);
				//kDebug() << "connectedSquares: " << connectedSquares;
				// order connected squares by valence: joints must be evaluated first
				QList<KSquares::LSConnection> connectedSquaresOrdered;
				for (int i = 0; i < connectedSquares.size(); i++)
				{
					if (squareValences[connectedSquares[i].square] < 2) // joints come first
						connectedSquaresOrdered.prepend(connectedSquares[i]);
					else
						connectedSquaresOrdered.append(connectedSquares[i]);
				}
				connectedSquares = connectedSquaresOrdered;
				// analyse ordered connected squares
				for (int i = 0; i < connectedSquares.size(); i++)
				{
					if (chain.contains(connectedSquares[i].line))
						continue;
					
					if ((squareValences[connectedSquares[i].square] == 3) || // next square is an end of chain
					    (squareValences[connectedSquares[i].square] == 2))   // next square continues chain
					{
						chain.append(connectedSquares[i].line);
						squareQueue.append(connectedSquares[i].square);
						if (excludeFromRemovalOnce.contains(connectedSquares[i].square) && !removedOnce.contains(connectedSquares[i].square))
						{
							excludeFromRemovalOnce.removeAll(connectedSquares[i].square); // this square connects to a joint square and will be the start square of another chain
							removedOnce.append(connectedSquares[i].square);
						}
						else
						{
							freeSquares.removeAll(connectedSquares[i].square); // this square is a part of a single chain and doesn't need to be evaluated as possible start of another chain
						}
						continue;
					}
					
					if (squareValences[connectedSquares[i].square] < 2) // next square is a joint
					{
						int connectedJointSquare = connectedSquares[i].square;
						QList<int> otherChainsReachingJoint;
						if (canCapture)
							jointReachedBefore[connectedJointSquare]++;
						else
							localJointReachedBefore++;
						QList<KSquares::LSConnection> jointConnections = getConnectedSquares(board, connectedJointSquare);
						QList<KSquares::LSConnection> externalJointConnections;
						//bool jointReachedBefore = false;
						for (int j = 0; j < jointConnections.size(); j++)
						{
							if (jointConnections[j].square == expandingSquare) // this is the square we're coming from now
								continue;
							
							bool isExternalConnection = true;
							for (int k = 0; k < foundChains->size(); k++)
							{
								if (!foundChains->at(k).ownChain || !canCapture)
									continue;
								if (foundChains->at(k).lines.contains(jointConnections[j].line))
								{
									isExternalConnection = false;
									otherChainsReachingJoint.append(k);
									//break;
								}
							}
							if (isExternalConnection && chain.contains(jointConnections[j].line)) // the chain has reached the joint before
							{
								//localJointReachedBefore++;
								isExternalConnection = false;
							}
							if (isExternalConnection)
								externalJointConnections.append(jointConnections[j]);
						}
						QList<int> jointGroundConnections = getGroundConnections(board, connectedJointSquare);
						if (squareValences[connectedJointSquare] + 
							jointReachedBefore[connectedJointSquare] + 
							localJointReachedBefore == 3 &&
							externalJointConnections.size() + 
							jointGroundConnections.size() == 1) // the joint can be passed
						{
							squareQueue.prepend(connectedJointSquare);
							passedJoint = true;
							//kDebug() << "passing joint " << connectedJointSquare << " coming from " << expandingSquare << ", foundChains: " << foundChains->size();
							for (int k = 0; k < otherChainsReachingJoint.size(); k++)
							{
								for (int l = foundChains->at(otherChainsReachingJoint[k]).lines.size()-1; l >= 0; l--)
									chain.prepend(foundChains->at(otherChainsReachingJoint[k]).lines[l]);
								for (int l = foundChains->at(otherChainsReachingJoint[k]).squares.size()-1; l >= 0; l--)
									chainSquares.prepend(foundChains->at(otherChainsReachingJoint[k]).squares[l]);
								foundChains->removeAt(otherChainsReachingJoint[k]);
								//kDebug() << "removed foundChain and prepended to new chain";
							}
							// there can be a chain that when completed creates a loop chain which contains the joint.
							// to find that chain the connection to that joint that's not part of the loop chain must be added to freeSquares
							if (externalJointConnections.size() > 0)
								excludeFromRemovalOnce.append(externalJointConnections[0].square);
							else // cutting the ground connection of the joint will create an open loop chain for the next player
							{
								//kDebug() << "no external joint connection, jointGroundConnections: " << jointGroundConnections;
								if (!onlyOwnChains && jointGroundConnections.size() > 0)
								{
									KSquares::Chain createLoopChainChain;
									createLoopChainChain.lines.append(jointGroundConnections[0]);
									createLoopChainChain.ownChain = false;
									createLoopChainChain.type = KSquares::CHAIN_SPECIAL;
									foundChains->append(createLoopChainChain);
								}
							}
						}
						/*
						else
						{
							kDebug() << "info for connectedJointSquare " << connectedJointSquare;
							kDebug() << "coming from: " << expandingSquare;
							kDebug() << "squareValences[connectedJointSquare]: " << squareValences[connectedJointSquare]; 
							kDebug() << "jointReachedBefore[connectedJointSquare]: " << jointReachedBefore[connectedJointSquare];
							kDebug() << "localJointReachedBefore: " << localJointReachedBefore;
							kDebug() << "externalJointConnections.size(): " << externalJointConnections.size();
							kDebug() << "getGroundConnections(board, connectedJointSquare).size(): " << getGroundConnections(board, connectedJointSquare).size();
						}
						*/
						// add the connection to the joint we're coming from
						chain.append(connectedSquares[i].line);
						continue;
					}
				}
				chainSquares.append(expandingSquare);
				unvisitedSquares.removeAll(expandingSquare);
			}
			
			bool canCaptureFromBothEnds = false;
			if (squareValences[expandingSquare] == 3)
			{
				chain = reverseQList(chain);
				chainSquares = reverseQList(chainSquares);
				if (canCapture)
					canCaptureFromBothEnds = true;
				if (!canCapture && passedJoint)
				{
					//kDebug() << "special case where a loop chain connects to a capturable chain";
				}
				else
					canCapture = true;
			}
			
			if (onlyOwnChains && !canCapture)
			{
				continue;
			}
			
			KSquares::Chain foundChain;
			foundChain.lines = chain;
			foundChain.squares = chainSquares;
			QList<int> classifyChainSquares;
			foundChain.type = classifyChain(board->width, board->height, chain, board->lines, &(classifyChainSquares)); // TODO: integrate classification!
			if (canCaptureFromBothEnds && foundChain.type == KSquares::CHAIN_LONG)
				foundChain.type = KSquares::CHAIN_LOOP;
			
			if (canCapture)
				foundChain.ownChain = true;
			else
				foundChain.ownChain = false;
			
			//kDebug() << "found chain:" << chain << "squares:" << foundChain.squares << "cap:" << canCapture << "type:" << chainTypeToString(foundChain.type) << " chain: " << linelistToString(chain, board->linesSize, board->width, board->height);
			foundChains->append(foundChain);
		}
		else
		{
			unvisitedSquares.append(square);
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

KSquares::ChainType aiFunctions::classifyChain(const QList<int> &chain, bool *lines) const
{
	return classifyChain(width, height, chain, lines);
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
	int lastSquareLoopJoint = -1;
	bool isLoop = false;
	
  squareQueue.push(squares->at(0));
  while (squareQueue.size() > 0)
  {
    int curSquare = squareQueue.pop();
    
		//kDebug() << "expanding: " << curSquare;
		
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
				// TODO: not general enough!
        if (countBorderLines(width, height, curLineSquares[j], lines) < 2)
				{
					if (squareReached[curLineSquares[j]] != 2)
					{
						continue;
					}
					else
					{
						lastSquareLoopJoint = curLineSquares[j];
						//kDebug() << "loop joint square: " << curLineSquares[j];
					}
				}
        squareQueue.push(curLineSquares[j]);
      }
    }
    
		if (squareQueue.size() == 0 && lastSquareLoopJoint == curSquare)
			isLoop = true;
  }
  
  // did we visit all squares?
  if (squares->size() != squareVisited.size())
  {
    kDebug() << "ERROR: didn't visit all squares (squares cnt = " << squares->size() << ", squares visited cnt = " << squareVisited.size() << "), board: " << boardToString(lines, toLinesSize(width, height), width, height) << "chain: " << chain << " as board: " << linelistToString(chain, toLinesSize(width, height), width, height);
		printSquares(squareVisited, width, height);
    return KSquares::CHAIN_UNKNOWN;
  }
  
  // chain ended in a loop joint
  if (isLoop)
	{
		return KSquares::CHAIN_LOOP;
	}
  
  // how many chains did we visit? two or less = short chain
  if (squareVisited.size() <= 2)
  {
    return KSquares::CHAIN_SHORT;
  }
  
  // long chain
  return KSquares::CHAIN_LONG; 
}


KSquares::BoardAnalysis aiFunctions::analyseBoard(aiBoard::Ptr board)
{
	KSquares::BoardAnalysis analysis;
	
	// look for capturable chains
	aiFunctions::findChains(board, &(analysis.chains), true);
	
	// sort capturable chains by classification
	for (int i = 0; i < analysis.chains.size(); i++)
	{
		switch (analysis.chains[i].type)
		{
			case KSquares::CHAIN_LONG:
				if (analysis.chains[i].ownChain)
					analysis.capturableLongChains.append(i);
			break;
			case KSquares::CHAIN_LOOP:
				if (analysis.chains[i].ownChain)
					analysis.capturableLoopChains.append(i);
			break;
			case KSquares::CHAIN_SHORT:
				if (analysis.chains[i].ownChain)
					analysis.capturableShortChains.append(i);
			break;
			case KSquares::CHAIN_SPECIAL:
				kDebug() << "ERROR: special own chain!" << analysis.chains[i];
			break;
			case KSquares::CHAIN_UNKNOWN:
			default:
				kDebug() << "WARNING: unknown chain! " << analysis.chains[i];
			break;
		}
		// capture everything that can be captured
		if (analysis.chains[i].ownChain)
			for (int j = 0; j < analysis.chains[i].lines.size(); j++)
				board->doMove(analysis.chains[i].lines[j]);
	}
	
	//kDebug() << "board after capture " << boardToString(board);
	
	// look for chains a second time
	aiFunctions::findChains(board, &(analysis.chainsAfterCapture));
	
	// sort chains by classification
	for (int i = 0; i < analysis.chainsAfterCapture.size(); i++)
	{
		switch (analysis.chainsAfterCapture[i].type)
		{
			case KSquares::CHAIN_LONG:
				if (!analysis.chainsAfterCapture[i].ownChain)
					analysis.openLongChains.append(i);
				else
				{
					kDebug() << "ERROR: capturable chain found when there should be none! chain: " << linelistToString(analysis.chainsAfterCapture[i].lines, board->linesSize, board->width, board->height) << " on board " << boardToString(board);
					for (int j = 0; j < analysis.chains.size(); j++)
						kDebug() << "capture chain: " << linelistToString(analysis.chains[j].lines, board->linesSize, board->width, board->height);
				}
			break;
			case KSquares::CHAIN_LOOP:
				if (!analysis.chainsAfterCapture[i].ownChain)
					analysis.openLoopChains.append(i);
				else
				{
					kDebug() << "ERROR: capturable chain found when there should be none! chain: " << linelistToString(analysis.chainsAfterCapture[i].lines, board->linesSize, board->width, board->height) << " on board " << boardToString(board);
					for (int j = 0; j < analysis.chains.size(); j++)
						kDebug() << "capture chain: " << linelistToString(analysis.chains[j].lines, board->linesSize, board->width, board->height);
				}
			break;
			case KSquares::CHAIN_SHORT:
				if (!analysis.chainsAfterCapture[i].ownChain)
					analysis.openShortChains.append(i);
				else
				{
					kDebug() << "ERROR: capturable chain found when there should be none! chain: " << linelistToString(analysis.chainsAfterCapture[i].lines, board->linesSize, board->width, board->height) << " on board " << boardToString(board);
					for (int j = 0; j < analysis.chains.size(); j++)
						kDebug() << "capture chain: " << linelistToString(analysis.chains[j].lines, board->linesSize, board->width, board->height);
				}
			break;
			case KSquares::CHAIN_SPECIAL:
				if (!analysis.chainsAfterCapture[i].ownChain)
				{
					for (int j = 0; j < analysis.chainsAfterCapture[i].lines.size(); j++)
					{
						analysis.specialLines.append(analysis.chainsAfterCapture[i].lines[j]);
					}
				}
				else
				{
					kDebug() << "ERROR: capturable chain found when there should be none! chain: " << linelistToString(analysis.chainsAfterCapture[i].lines, board->linesSize, board->width, board->height) << " on board " << boardToString(board);
					for (int j = 0; j < analysis.chains.size(); j++)
						kDebug() << "capture chain: " << linelistToString(analysis.chains[j].lines, board->linesSize, board->width, board->height);
				}
			break;
			case KSquares::CHAIN_UNKNOWN:
			default:
				kDebug() << "WARNING: unknown chain! " << analysis.chainsAfterCapture[i].lines;
			break;
		}
	}
	
	// look for safe moves
	//analysis.safeLines = safeMoves(board->width, board->height, board->linesSize, board->lines);
	
	// undo capture moves
	for (int i = 0; i < analysis.chains.size(); i++)
	{
		if (!analysis.chains[i].ownChain)
			continue;
		for (int j = analysis.chains[i].lines.size()-1; j >= 0; j--)
			board->undoMove(analysis.chains[i].lines[j]);
	}
	
	return analysis;
}
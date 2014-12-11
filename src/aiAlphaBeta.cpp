/***************************************************************************
 *   Copyright (C) 2014 by Tom Vincent Peters   <kde@vincent-peters.de>    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "aiAlphaBeta.h"

#include <limits>
#include <KDebug>
#include <cmath>
#include <algorithm>
#include <QElapsedTimer>
#include <QMap>
#include <QPair>

aiAlphaBeta::aiAlphaBeta(int newPlayerId, int newMaxPlayerId, int newWidth, int newHeight, int newLevel) : KSquaresAi(newWidth, newHeight), playerId(newPlayerId), maxPlayerId(newMaxPlayerId), level(newLevel)
{
	width = newWidth;
	height = newHeight;
	linesSize = toLinesSize(width, height);
	lines = new bool[linesSize];
	debug = false;
	maxEvalTime = 0;
	alphabetaTimeout = 5000; // 5 sec timeout
	heuristic = new aiHeuristic(true, true, true);
}

aiAlphaBeta::~aiAlphaBeta()
{
	delete[] lines;
	delete heuristic;
}

int aiAlphaBeta::chooseLine(const QList<bool> &newLines, const QList<int> &newSquareOwners)
{
	if (newLines.size() != linesSize)
	{
		kFatal() << "something went terribly wrong: newLines.size() != linesSize";
	}
	// put lines into local board representation
	for (int i = 0; i < linesSize; ++i) lines[i] = newLines[i];
	// remember square owner table
	squareOwners = newSquareOwners;
	// do the ai stuff:
	kDebug() << "incoming board:" << boardToString(lines, linesSize, width, height);
	
	// do sth smart
	aiBoard::Ptr board = aiBoard::Ptr(new aiBoard(lines, linesSize, width, height, squareOwners, playerId, maxPlayerId));
	
	int line;
	alphabeta(board, 4, &line);
	
	if (line < 0 || line >= linesSize)
	{
		kDebug() << "alphabeta didn't return a correct line: " << line;
		kDebug() << "coosing random valid move";
		QList<int> freeLines = aiFunctions::getFreeLines(lines, linesSize);
		if (freeLines.size() <= 0)
		{
			kDebug() << "no valid lines left!";
			return 0;
		}
		return freeLines.at(qrand() % freeLines.size());
	}
	
	return line;
}

/*
 * Sources:
 * http://en.wikipedia.org/wiki/Minimax#Minimax_algorithm_with_alternate_moves
 * http://www.fierz.ch/strategy1.htm
*/

float aiAlphaBeta::alphabeta(aiBoard::Ptr board, int depth, int *line, int parentNode)
{
	if (line != NULL)
	{
		if (!alphabetaTimer.isValid())
		{
			kDebug() << "starting alphabeta timer";
			alphabetaTimer.start();
		}
		else
		{
			kDebug() << "restarting alphabeta timer";
			alphabetaTimer.restart();
		}
	}
	QList<QList<int> > moveSequences = getMoveSequences(board);
	
	int thisNode = debugNodeCnt;
	debugNodeCnt++;
	if (debug)
	{
		QString boardStr = aiFunctions::boardToString(board->lines, board->linesSize, board->width, board->height).trimmed();
		boardStr.replace(QString("\n"), QString("\\l"));
		//QString debugDot = "";
		debugDot.append("  n");
		debugDot.append(QString::number(thisNode));
		debugDot.append("[label=\"p:");
		debugDot.append(QString::number(board->playerId));
		debugDot.append("\\l");
		debugDot.append(boardStr);
		debugDot.append("\"];\n");
		if (parentNode != -1)
		{
			//kDebug() << debugDot;
			debugDot.append("  n");
			debugDot.append(QString::number(thisNode));
			debugDot.append(" -- n");
			debugDot.append(QString::number(parentNode));
			debugDot.append(";\n");
		}
	}
	
	if (moveSequences.size() == 0) // game is over
	{
		//kDebug() << "terminal node";
		int winner = aiFunctions::getLeader(board->squareOwners);
		if (winner == -2) // draw
			return 0;
		if (winner == playerId)
			return -INFINITY;
		else
			return INFINITY;
	}
	
	bool terminalNode = false;
	if (depth == 0) terminalNode = true;
	if (line == NULL && alphabetaTimer.hasExpired(alphabetaTimeout)) 
	{
		terminalNode = true;
		kDebug() << "alphabeta timeout reached, not going deeper";
	}
	if (terminalNode)
	{
		//kDebug() << "evaluating board:" << boardToString(board->lines, board->linesSize, board->width, board->height);
		int eval = evaluate(board);
		if (debug)
		{
			//kDebug() << "result: " << eval;
			debugDot.append("  e");
			debugDot.append(QString::number(thisNode));
			debugDot.append("[label=\"");
			debugDot.append(QString::number(eval));
			debugDot.append("\\n");
			debugDot.append(QString::number(lastEvalTime));
			debugDot.append(" ms");
			debugDot.append("\"];\n  e");
			debugDot.append(QString::number(thisNode));
			debugDot.append(" -- n");
			debugDot.append(QString::number(thisNode));
			debugDot.append(";\n");
		}
		return eval;
	}
	
	float bestValue = -INFINITY;
	for (int i = 0; i < moveSequences.size(); i++)
	{
		for (int j = 0; j < moveSequences[i].size(); j++)
		{
			board->doMove(moveSequences[i][j]);
		}
		float val = -alphabeta(board, depth - 1, NULL, thisNode);
		for (int j = 0; j < moveSequences[i].size(); j++)
		{
			board->undoMove(moveSequences[i][j]);
		}
		if (val > bestValue)
		{
			bestValue = val;
			if (line != NULL)
				*line = moveSequences[i][0];
		}
	}
	return bestValue;
}

float aiAlphaBeta::evaluate(aiBoard::Ptr board)
{
	QElapsedTimer evalTimer;
	evalTimer.start();
	float ret = heuristic->evaluate(board, playerId);
	long evalTime = evalTimer.elapsed();
	if (evalTime > maxEvalTime)
		evalTime = maxEvalTime;
	return ret;
}

// TODO: move to aiFunctions
/**
 * Finds the squares connected to given square
 * @param board the board to operate on
 * @param square the square in question
 * @return List of pairs (first: line index, second: square index) adjacent to given square
 */
QList<QPair<int, int> > getConnectedSquares(aiBoard::Ptr board, int square)
{
	QList<QPair<int, int> > connectedSquares;
	
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
			QPair<int, int> connectedSquare(squareLines[i], lineSquares[j]);
			connectedSquares.append(connectedSquare);
		}
	}
	
	return connectedSquares;
}

int getMissingLine(aiBoard::Ptr board, int square)
{
	int squareLines[4];
	aiFunctions::linesFromSquare(board->width, board->height, squareLines, square);
	for (int i = 0; i < 4; i++)
	{
		if (board->lines[squareLines[i]])
			continue;
		return squareLines[i];
	}
	return -1;
}

QList<QList<int> > aiAlphaBeta::getMoveSequences(aiBoard::Ptr board)
{
	QList<QList<int> > moveSequences;
	
	QMap<int, int> squareValences; // square, valence
	
	// find untaken squares and calculate valence
	QList<int> freeSquares;
	for (int i = 0; i < board->squareOwners.size(); i++)
	{
		if (board->squareOwners[i] == -1)
		{
			squareValences[i] = countBorderLines(board->width, board->height, i, board->lines);
			freeSquares.append(i);
		}
	}
	
	// look for chains
	QList<QList<int> > capturableChains;
	QList<QList<int> > uncapturableChains;
	while (freeSquares.size() > 0)
	{
		int square = freeSquares.takeLast();
		
		if (squareValences[square] == 3)
		{
			QList<int> capturableChain;
			
			//int nextLine = getMissingLine(board, square);
			int nextSquare = square;
			bool foundSquare = true;
			while (foundSquare)
			{
				foundSquare = false;
				QList<QPair<int, int> > connectedSquares = getConnectedSquares(board, nextSquare);
				for (int i = 0; i < connectedSquares.size(); i++)
				{
					if (capturableChain.contains(connectedSquares[i].first) ||
							squareValences[nextSquare] < 2)
						continue;
					
					capturableChain.append(connectedSquares[i].first);
					nextSquare = connectedSquares[i].second;
					freeSquares.removeAll(nextSquare);
					foundSquare = true;
				}
			}
			
			capturableChains.append(capturableChain);
		}
	}
	
	return capturableChains;
}

QList<QList<int> > getMoveSequencesOld(aiBoard::Ptr board)
{
	QList<int> freeLines = aiFunctions::getFreeLines(board->lines, board->linesSize);
	QMap<int, int> squareValence;
	QList<QPair<int, int> > chainEnds; // line, square
	while (freeLines.size() > 0)
	{
		int line = freeLines.takeLast();
		QList<int> lineSquares = aiFunctions::squaresFromLine(board->width, board->height, line);
		for (int i = 0; i < lineSquares.size(); i++)
		{
			if (squareValence.contains(lineSquares[i]))
				continue;
			squareValence[lineSquares[i]] = aiFunctions::countBorderLines(board->width, board->height, lineSquares[i], board->lines);
			
			if (squareValence[lineSquares[i]] == 3) // square can be captured
			{
				QPair<int, int> chainEnd(line, lineSquares[i]);
				chainEnds.append(chainEnd);
			}
		}
	}
	
	kDebug() << "chainEnds: " << chainEnds;
	
	QList<QList<int> > ret;
	for (int i = 0; i < chainEnds.size(); i++)
	{
		QList<int> lineSequence;
		//lineSequence.append(chainEnds[i].first);
		QPair<int, int> elem = chainEnds[i];
		bool seqDone = false;
		while (!seqDone)
		{
			seqDone = true;
			int line = elem.first;
			int square = elem.second;
			QList<int> lineSquares = aiFunctions::squaresFromLine(board->width, board->height, line);
			for (int j = 0; j < lineSquares.size(); j++)
			{
				if (lineSquares[j] == square)
					continue;
				int squareLines[4];
				aiFunctions::linesFromSquare(board->width, board->height, squareLines, lineSquares[j]);
				for (int k = 0; k < 4; k++)
				{
					if (board->lines[squareLines[k]] || squareLines[k] == line)
						continue;
					if (squareValence[lineSquares[j]] == 2) // chain continues
					{
						seqDone = false;
					}
					elem.first = squareLines[k];
					elem.second = lineSquares[j];
					lineSequence.append(line);
				}
			}
		}
		ret.append(lineSequence);
	}
	return ret;
}

void aiAlphaBeta::setTimeout(long timeout)
{
	alphabetaTimeout = timeout;
}

long aiAlphaBeta::getTimeout()
{
	return alphabetaTimeout;
}

void aiAlphaBeta::setDebug(bool val)
{
	debug = val;
	debugDot = "";
	debugNodeCnt = 0;
}

QString aiAlphaBeta::getDebugDot()
{
	return debugDot;
}

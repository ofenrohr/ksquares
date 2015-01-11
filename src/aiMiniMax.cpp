/***************************************************************************
 *   Copyright (C) 2014 by Tom Vincent Peters   <kde@vincent-peters.de>    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "aiMiniMax.h"

#include <limits>
#include <KDebug>
#include <cmath>
#include <algorithm>
#include <QElapsedTimer>

aiMiniMax::aiMiniMax(int newPlayerId, int newMaxPlayerId, int newWidth, int newHeight, int newLevel) : KSquaresAi(newWidth, newHeight), playerId(newPlayerId), maxPlayerId(newMaxPlayerId), level(newLevel)
{
	width = newWidth;
	height = newHeight;
	linesSize = toLinesSize(width, height);
	lines = new bool[linesSize];
	debug = false;
	maxEvalTime = 0;
	minimaxTimeout = 5000; // 5 sec timeout
	heuristic = new aiHeuristic(true, true, true);
}

aiMiniMax::~aiMiniMax()
{
	delete[] lines;
	delete heuristic;
}

int aiMiniMax::chooseLine(const QList<bool> &newLines, const QList<int> &newSquareOwners)
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
	minimax(board, 4, &line);
	
	if (line < 0 || line >= linesSize)
	{
		kDebug() << "minimax didn't return a correct line: " << line;
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

float aiMiniMax::minimax(aiBoard::Ptr board, int depth, int *line, int parentNode)
{
	if (line != NULL)
	{
		if (!minimaxTimer.isValid())
		{
			kDebug() << "starting minimax timer";
			minimaxTimer.start();
		}
		else
		{
			kDebug() << "restarting minimax timer";
			minimaxTimer.restart();
		}
	}
	QList<int> freeLines = aiFunctions::getFreeLines(board->lines, board->linesSize);
	
	int thisNode = debugNodeCnt;
	debugNodeCnt++;
	if (debug)
	{
		QString boardStr = aiFunctions::boardToString(board->lines, board->linesSize, board->width, board->height).trimmed();
		boardStr.replace(QString("\n"), QString("\\l"));
		//QString debugDot = "";
		debugDot.append("  n");
		debugDot.append(QString::number(thisNode));
		debugDot.append("[shape=box,label=\"p:");
		debugDot.append(QString('A'+board->playerId));
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
	
	if (freeLines.size() == 0) // game is over
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
	if (line == NULL && minimaxTimer.hasExpired(minimaxTimeout)) 
	{
		terminalNode = true;
		kDebug() << "minimax timeout reached, not going deeper";
	}
	if (terminalNode)
	{
		//kDebug() << "evaluating board:" << boardToString(board->lines, board->linesSize, board->width, board->height);
		int eval = evaluate(board);
		if (debug && false)
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
	
	if (playerId == board->playerId)
	{
		float bestValue = -INFINITY;
		for (int i = 0; i < freeLines.size(); i++)
		{
			board->doMove(freeLines[i]);
			float val = minimax(board, depth - 1, NULL, thisNode);
			board->undoMove(freeLines[i]);
			if (val > bestValue)
			{
				bestValue = val;
				if (line != NULL)
					*line = freeLines[i];
			}
		}
		return bestValue;
	}
	else
	{
		float bestValue = INFINITY;
		for (int i = 0; i < freeLines.size(); i++)
		{
			board->doMove(freeLines[i]);
			float val = minimax(board, depth - 1, NULL, thisNode);
			board->undoMove(freeLines[i]);
			if (val < bestValue)
			{
				bestValue = val;
				if (line != NULL)
					*line = freeLines[i];
			}
		}
		return bestValue;
	}
	
	return 0.0;
}

float aiMiniMax::evaluate(aiBoard::Ptr board)
{
	QElapsedTimer evalTimer;
	evalTimer.start();
	//float ret = evaluate2(board);
	float ret = heuristic->evaluate(board, playerId);
	long evalTime = evalTimer.elapsed();
	if (evalTime > maxEvalTime)
		evalTime = maxEvalTime;
	return ret;
}

void aiMiniMax::setTimeout(long timeout)
{
	minimaxTimeout = timeout;
}

long aiMiniMax::getTimeout()
{
	return minimaxTimeout;
}

void aiMiniMax::setDebug(bool val)
{
	debug = val;
	debugDot = "";
	debugNodeCnt = 0;
}

QString aiMiniMax::getDebugDot()
{
	return debugDot;
}

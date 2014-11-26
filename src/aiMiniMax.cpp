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

aiMiniMax::aiMiniMax(int newPlayerId, int newMaxPlayerId, int newWidth, int newHeight, int newLevel) : KSquaresAi(newWidth, newHeight), playerId(newPlayerId), maxPlayerId(newMaxPlayerId), level(newLevel)
{
	width = newWidth;
	height = newHeight;
	linesSize = toLinesSize(width, height);
	lines = new bool[linesSize];
	debug = false;
}

aiMiniMax::~aiMiniMax()
{
	delete[] lines;
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
	aiBoard board(lines, linesSize, width, height, squareOwners, playerId, maxPlayerId);
	
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
 * From: http://en.wikipedia.org/wiki/Minimax#Minimax_algorithm_with_alternate_moves
 * 
function minimax(node, depth, maximizingPlayer)
    if depth = 0 or node is a terminal node
        return the heuristic value of node
    if maximizingPlayer
        bestValue := -∞
        for each child of node
            val := minimax(child, depth - 1, FALSE)
            bestValue := max(bestValue, val)
        return bestValue
    else
        bestValue := +∞
        for each child of node
            val := minimax(child, depth - 1, TRUE)
            bestValue := min(bestValue, val)
        return bestValue

(* Initial call for maximizing player *)
minimax(origin, depth, TRUE)
*/

float aiMiniMax::minimax(aiBoard &board, int depth, int *line, int parentNode)
{
	QList<int> freeLines = aiFunctions::getFreeLines(board.lines, board.linesSize);
	
	int thisNode = debugNodeCnt;
	debugNodeCnt++;
	if (debug)
	{
		QString boardStr = aiFunctions::boardToString(board.lines, board.linesSize, board.width, board.height).trimmed();
		boardStr.replace(QString("\n"), QString("\\l"));
		//QString debugDot = "";
		debugDot.append("  n");
		debugDot.append(QString::number(thisNode));
		debugDot.append("[label=\"p:");
		debugDot.append(QString::number(board.playerId));
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
		int winner = aiFunctions::getLeader(board.squareOwners);
		if (winner == -2) // draw
			return 0;
		if (winner == playerId)
			return -INFINITY;
		else
			return INFINITY;
	}
	
	if (depth == 0)
	{
		//kDebug() << "evaluating board:" << boardToString(board.lines, board.linesSize, board.width, board.height);
		int eval = evaluate(board);
		if (debug)
		{
			//kDebug() << "result: " << eval;
			debugDot.append("  e");
			debugDot.append(QString::number(thisNode));
			debugDot.append("[label=\"");
			debugDot.append(QString::number(eval));
			debugDot.append("\"];\n  e");
			debugDot.append(QString::number(thisNode));
			debugDot.append(" -- n");
			debugDot.append(QString::number(thisNode));
			debugDot.append(";\n");
		}
		return eval;
	}
	
	if (playerId == board.playerId)
	{
		float bestValue = -INFINITY;
		for (int i = 0; i < freeLines.size(); i++)
		{
			// TODO: enable more than 2 player game!
			board.doMove(freeLines[i]);
			float val = minimax(board, depth - 1, NULL, thisNode);
			board.undoMove(freeLines[i]);
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
			// TODO: enable more than 2 player game!
			board.doMove(freeLines[i]);
			float val = minimax(board, depth - 1, NULL, thisNode);
			board.undoMove(freeLines[i]);
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

float aiMiniMax::evaluate(aiBoard &board)
{
	int squaresCnt = 0;
	QList<QList<int> > chains;
	QList<QSet<int> > chainSet;
	
	squaresCnt = aiFunctions::findOwnChains(board.lines, board.linesSize, board.width, board.height, &chains);
	
	// analyze chains
	int shortChainCnt = 0; // chains <= 2 lines
	int longChainCnt = 0; // exploitable chains
	int loopChainCnt = 0; // also exploitable, but more costly
	QList<QList<int> > shortChains;
  QList<QList<int> > longChains;
  QList<QList<int> > loopChains;
	
	for (int i = 0; i < chains.size(); i++)
	{
		QList<int> chain = chains[i];
    int classification = aiFunctions::classifyChain(board.width, board.height, chain, board.lines);
    //kDebug() << "analysing chain " << chain << ": " << classification;
		switch (classification)
		{
      case 0: 
        longChainCnt++;
        longChains.append(chain);
      break;
      case 1:
        shortChainCnt++;
        shortChains.append(chain);
      break;
      case 2:
        loopChainCnt++;
        loopChains.append(chain);
      break;
      default:
        kDebug() << "unknown chain type " << classification;
    }
	}
	//kDebug() << "short chains:" << shortChainCnt << ", long chains: " << longChainCnt << ", loop chains: " << loopChainCnt << "ownSquaresCnt: " << ownSquaresCnt;
	int score = 0;
	int enemyScore = 0;
	QMap<int, int> scores = aiFunctions::getScoreMap(board.squareOwners);
	if (scores.contains(playerId))
		score = scores[playerId];
	for (int i = 0; i <= board.maxPlayerId; i++)
	{
		if (i == playerId)
			continue;
		if (scores.contains(i))
			enemyScore += scores[i];
	}
	
	// long chain rule: dots + long chains % 2 even for A, odd for B
	int dots = (board.width + 1) * (board.height + 1);
	int lcr = (dots + longChainCnt) % 2 ? -dots : dots;
	return - squaresCnt - enemyScore + score;
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

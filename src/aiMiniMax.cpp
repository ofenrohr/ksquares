/***************************************************************************
 *   Copyright (C) 2014 by Tom Vincent Peters   <kde@vincent-peters.de>    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "aiMiniMax.h"

#include <KDebug>
#include <cmath>

aiMiniMax::aiMiniMax(int newPlayerId, int newWidth, int newHeight, int newLevel) : KSquaresAi(newWidth, newHeight), playerId(newPlayerId), level(newLevel)
{
	width = newWidth;
	height = newHeight;
	linesSize = toLinesSize(width, height);
	lines = new bool[linesSize];
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
	aiBoard::Ptr board = aiBoard::Ptr(new aiBoard(lines, linesSize, width, height, squareOwners));
	
	return 0;
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

float aiMiniMax::minimax(aiBoard::Ptr board, int linesSize, int depth, int playerId, int ownPlayerId, int *line)
{
	QList<int> freeLines = aiFunctions::getFreeLines(board->lines, board->linesSize);
	
	if (freeLines.size() == 0) // game is over
	{
		int winner = aiFunctions::getLeader(board->squareOwners);
		if (winner == -2) // draw
			return 0;
		if (winner == ownPlayerId)
			return -INFINITY;
		else
			return INFINITY;
	}
	
	if (depth == 0)
		return evaluate(board);
	
	if (playerId == ownPlayerId)
	{
		float bestValue = -INFINITY;
		for (int i = 0; i < freeLines.size(); i++)
		{
			// TODO: enable more than 2 player game!
			board->doMove(freeLines[i], 1 - playerId);
			float val = minimax(board, linesSize, depth - 1, 1 - playerId, ownPlayerId, NULL);
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
			// TODO: enable more than 2 player game!
			board->doMove(freeLines[i], 1 - playerId);
			float val = minimax(board, linesSize, depth - 1, 1 - playerId, ownPlayerId, NULL);
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
	return 0.0;
}
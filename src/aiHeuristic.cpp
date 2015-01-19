/***************************************************************************
 *   Copyright (C) 2014 by Tom Vincent Peters   <kde@vincent-peters.de>    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "aiHeuristic.h"

#include "aifunctions.h"

#include <KDebug>
#include <algorithm>


aiHeuristic::aiHeuristic()
{
	enableSquaresCnt = true;
	enableScores = true;
	enableLongChainRule = true;
	chainsAnalysed = false;
	debug = false;
}

aiHeuristic::aiHeuristic(bool squaresCnt, bool scores, bool longChainRule)
	: enableSquaresCnt(squaresCnt),
	  enableScores(scores),
	  enableLongChainRule(longChainRule)
{
	chainsAnalysed = false;
	debug = false;
}

void aiHeuristic::reset()
{
	chainsAnalysed = false;
}

void aiHeuristic::setDebug(bool d)
{
	debug = d;
}

float aiHeuristic::evaluate(aiBoard::Ptr board, int ownPlayerId)
{
	playerId = ownPlayerId >= 0 ? ownPlayerId : board->playerId;
	reset();
	float result = 0.0;
	
	if (enableSquaresCnt)
		result += evalSquaresCnt(board);
	
	if (enableScores)
		result += evalScores(board);
	
	if (enableLongChainRule)
		result += evalLongChainRule(board);
	
	return result;
}

void aiHeuristic::analyseChains(aiBoard::Ptr board)
{
	if (chainsAnalysed)
		return;
	
	//board->analyseBoard();
	analysis = aiFunctions::analyseBoard(board);
	chainsAnalysed = true;
}

float aiHeuristic::evalSquaresCnt(aiBoard::Ptr board)
{
	analyseChains(board);
	
	int ownSquaresCnt = 0;
	
	for (int i = 0; i < analysis.chains.size(); i++)
	{
		if (analysis.chains[i].ownChain)
			ownSquaresCnt += analysis.chains[i].squares.size();
	}
	
	return ownSquaresCnt;
}

float aiHeuristic::evalScores(aiBoard::Ptr board)
{
	int score = 0;
	int enemyScore = 0;
	QMap<int, int> scores = aiFunctions::getScoreMap(board->squareOwners);
	if (scores.contains(playerId))
		score = scores[playerId];
	for (int i = 0; i <= board->maxPlayerId; i++)
	{
		if (i == playerId)
			continue;
		if (scores.contains(i))
			enemyScore += scores[i];
	}
	
	return score - enemyScore;
}

float aiHeuristic::evalLongChainRule(aiBoard::Ptr board)
{
	analyseChains(board);
	
	int dots = (board->width + 1) * (board->height + 1);
	int lcr = (dots + analysis.openLongChains.size()) % 2 != board->playerId ? -dots : dots;
	
	if (debug)
	{
		kDebug() << "evaluation of board " << aiFunctions::boardToString(board);
		kDebug() << "  dots = " << dots;
		kDebug() << "  long chains = " << analysis.openLongChains.size();
		kDebug() << "  playerId = " << board->playerId;
		kDebug() << "  lcr = " << lcr;
	}
	
	return lcr;
}

void aiHeuristic::setAnalysis(KSquares::BoardAnalysis &a)
{
	chainsAnalysed = true;
	analysis = a;
}
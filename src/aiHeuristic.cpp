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
}

aiHeuristic::aiHeuristic(bool squaresCnt, bool scores, bool longChainRule)
	: enableSquaresCnt(squaresCnt),
	  enableScores(scores),
	  enableLongChainRule(longChainRule)
{
}

void aiHeuristic::reset()
{
	chainsAnalyzed = false;
// 	ownSquaresCnt = 0;
// 	shortChainCnt = 0;
// 	longChainCnt = 0;
// 	loopChainCnt = 0;
// 	ownChains.clear();
// 	shortChains.clear();
// 	longChains.clear();
// 	loopChains.clear();
}

float aiHeuristic::evaluate(aiBoard::Ptr board, int ownPlayerId)
{
	playerId = ownPlayerId;
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
	if (chainsAnalyzed)
		return;
	
	//board->analyseBoard();
	analysis = aiFunctions::analyseBoard(board);
}

float aiHeuristic::evalSquaresCnt(aiBoard::Ptr board)
{
	analyseChains(board);
	
	int ownSquaresCnt = 0;
	
	//for (int i = 0; i < analysis.capturableLongChains.size(); i++)
	
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
	int lcr = (dots + analysis.capturableLongChains.size() + analysis.capturableLoopChains.size()) % 2 == board->playerId ? -dots : dots;
	
	return lcr;
}

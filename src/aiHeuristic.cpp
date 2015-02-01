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
#include "boardAnalysis.h"

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
	float div = 0.0;
	
	if (enableSquaresCnt)
	{
		div += 1.0;
		result += evalSquaresCnt(board);
	}
	
	if (enableScores)
	{
		div += 1.0;
		result += evalScores(board);
	}
	
	if (enableLongChainRule)
	{
		div += 1.0;
		result += evalLongChainRule(board);
	}
	
	if (div == 0.0) return 0;
	
	return result / div;
}

void aiHeuristic::analyseChains(aiBoard::Ptr board)
{
	if (chainsAnalysed)
		return;
	
	//board->analyseBoard();
	analysis = BoardAnalysisFunctions::analyseBoard(board);
	chainsAnalysed = true;
}

float aiHeuristic::evalSquaresCnt(aiBoard::Ptr board)
{
	analyseChains(board);
	
	int ownSquaresCnt = 0;
	
	QList<int> capturableChains;
	capturableChains.append(analysis.capturableLongChains);
	capturableChains.append(analysis.capturableLoopChains);
	capturableChains.append(analysis.capturableShortChains);
	for (int i = 0; i < capturableChains.size(); i++)
	{
		ownSquaresCnt += analysis.chains[capturableChains[i]].squares.size();
	}
	
	return (float)ownSquaresCnt / (float)(board->width * board->height);
}

float aiHeuristic::evalScores(aiBoard::Ptr board)
{
	int score = 0;
	int enemyScore = 0;
	QMap<int, int> scores = aiFunctions::getScoreMap(board->squareOwners);
	if (scores.contains(board->playerId))
		score = scores[board->playerId];
	for (int i = 0; i <= board->maxPlayerId; i++)
	{
		if (i == board->playerId)
			continue;
		if (scores.contains(i))
			enemyScore += scores[i];
	}
	
	return (float)score - (float)enemyScore / (float)(board->width * board->height);
}

float aiHeuristic::evalLongChainRule(aiBoard::Ptr board)
{
	analyseChains(board);
	
	if (analysis.safeLines.size() > 0)
		return 0;
	
	int dots = (board->width + 1) * (board->height + 1);
	
	// check if the enemy made a loony move
	if (analysis.capturableLongChains.size() > 0)
		return 1;
	if (analysis.capturableLoopChains.size() > 0)
		return 1;
	for (int i = 0; i < analysis.capturableShortChains.size(); i++)
	{
		if (analysis.chains[analysis.capturableShortChains[i]].squares.size() > 1)
			return 1;
	}
	int lcr = (dots + analysis.openLongChains.size()) % 2 != board->playerId ? -1 : 1;
	
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
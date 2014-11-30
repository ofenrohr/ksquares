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
	ownSquaresCnt = 0;
	shortChainCnt = 0;
	longChainCnt = 0;
	loopChainCnt = 0;
	ownChains.clear();
	shortChains.clear();
	longChains.clear();
	loopChains.clear();
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
	
	// find opened chains
	ownSquaresCnt = aiFunctions::findOwnChains(board->lines, board->linesSize, board->width, board->height, &ownChains);
	for (int i = 0; i < ownChains.size(); i++)
	{
		for (int j = 0; j < ownChains[i].size(); j++)
		{
			board->lines[ownChains[i][j]] = true;
		}
	}
	
	// find chains
	QList<int> freeLines = aiFunctions::getFreeLines(board->lines, board->linesSize);
	QList<QList<int> > chains;
	QList<QList<int> > chainList;
	QList<QList<int> > chainSet;
	
	while (!freeLines.isEmpty())
	{
		int line = freeLines.takeLast();
		board->lines[line] = true;
		chains.clear();
		aiFunctions::findOwnChains(board->lines, board->linesSize, board->width, board->height, &chains);
		
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
		
		board->lines[line] = false;
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
		QList<int> chain = chainSet[i];
    int classification = aiFunctions::classifyChain(board->width, board->height, chain, board->lines);
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
				kDebug() << "board: " << aiFunctions::boardToString(board->lines, board->linesSize, board->width, board->height);
				kDebug() << "chain: " << aiFunctions::linelistToString(chain, board->linesSize, board->width, board->height);
    }
	}
	
	// cleanup - undo ownChain!
	for (int i = 0; i < ownChains.size(); i++)
	{
		for (int j = 0; j < ownChains[i].size(); j++)
		{
			board->lines[ownChains[i][j]] = false;
		}
	}
}

float aiHeuristic::evalSquaresCnt(aiBoard::Ptr board)
{
	analyseChains(board);
	
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
	int lcr = (dots + longChainCnt) % 2 == board->playerId ? -dots : dots;
	
	return lcr;
}

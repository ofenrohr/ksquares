/***************************************************************************
 *   Copyright (C) 2014 by Tom Vincent Peters   <kde@vincent-peters.de>    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef AIHEURISTIC_H
#define AIHEURISTIC_H

#include "aiBoard.h"

/**
 * Used to evaluate a dots and boxes board.
 */
class aiHeuristic
{
public:
	aiHeuristic();
	aiHeuristic(bool squaresCnt, bool scores, bool longChainRule);
	
	void reset();
	float evaluate(aiBoard::Ptr board, int ownPlayerId);
	
protected:
	int playerId;
	
	bool enableSquaresCnt;
	bool enableScores;
	bool enableLongChainRule;
	
	float evalSquaresCnt(aiBoard::Ptr board);
	float evalScores(aiBoard::Ptr board);
	float evalLongChainRule(aiBoard::Ptr board);
	
	void analyseChains(aiBoard::Ptr board);
	bool chainsAnalyzed;
	int ownSquaresCnt; // squares in opened chain
	int shortChainCnt; // chains <= 2 lines
	int longChainCnt; // exploitable chains
	int loopChainCnt; // also exploitable, but more costly
	QList<QList<int> > ownChains;
	QList<QList<int> > shortChains;
  QList<QList<int> > longChains;
  QList<QList<int> > loopChains;
};

#endif

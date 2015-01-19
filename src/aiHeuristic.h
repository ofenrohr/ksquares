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
	float evaluate(aiBoard::Ptr board, int ownPlayerId = -1);
	void setAnalysis(KSquares::BoardAnalysis &a);
	void setDebug(bool d);
	
protected:
	int playerId;
	
	bool enableSquaresCnt;
	bool enableScores;
	bool enableLongChainRule;
	
	float evalSquaresCnt(aiBoard::Ptr board);
	float evalScores(aiBoard::Ptr board);
	float evalLongChainRule(aiBoard::Ptr board);
	
	void analyseChains(aiBoard::Ptr board);
	bool chainsAnalysed;
	KSquares::BoardAnalysis analysis;
	
	bool debug;
};

#endif

/***************************************************************************
 *   Copyright (C) 2015 by Tom Vincent Peters    <kde@vincent-peters.de>   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef BOARD_ANALYSIS_H
#define BOARD_ANALYSIS_H

#include "aistructs.h"
#include "aiBoard.h"

class BoardAnalysisFunctions
{
	public:
		/**
		 * Looks for chains on the board and returns chains and valid move sequences
		 */
		static KSquares::BoardAnalysis analyseBoard(aiBoard::Ptr board);
		static KSquares::BoardAnalysis analyseBoard(aiBoard::Ptr board, QList<int> &lineSortList);
		/**
		 * Generates the double dealing sequence for the given chain.
		 * @return the double dealing variant of the given chain. if the chain is short or unknown an empty list will be returned
		 */
		static QList<int> getDoubleDealingSequence(KSquares::Chain &chain);
		/**
		 * Two lines at each corner of the board are equivalent.
		 * This returns one line per corner if both corner lines aren't drawn.
		 */
		static QList<int> ignoreCornerLines(aiBoard::Ptr board);
		/**
		 * Generates a move list. Chains are returned twice if applicable:
		 * one sequence with chain fully taken and one with double dealing.
		 */
		//QSharedPointer<QList<QList<int> > > getMoveSequences(aiBoard::Ptr board, KSquares::BoardAnalysis &analysis, bool *isEndgame = NULL);
		static QSharedPointer<QList<QList<int> > > getMoveSequences(aiBoard::Ptr board, KSquares::BoardAnalysis &analysis, QList<int> &lineSortList, bool *isEndgame = NULL);
};

#endif
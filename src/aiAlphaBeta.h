/***************************************************************************
 *   Copyright (C) 2014 by Tom Vincent Peters   <kde@vincent-peters.de>    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef AIALPHABETA_H
#define AIALPHABETA_H

#include "aicontroller.h"
#include "aifunctions.h"
#include "aiBoard.h"
#include "aiHeuristic.h"

#include <QString>
#include <QElapsedTimer>

#include <cmath>


class aiAlphaBeta : public KSquaresAi
{
	public:
		aiAlphaBeta(int newPlayerId, int newMaxPlayerId, int newWidth, int newHeight, int newLevel);
		~aiAlphaBeta();
		
		int chooseLine(const QList<bool> &newLines, const QList<int> &newSquareOwners);
		QString getName() { return "alphabeta"; }
		
		void setDepth(int d) { searchDepth = d; }
		float alphabeta(aiBoard::Ptr board, int depth, int *line, float alpha = -INFINITY, float beta = INFINITY, int parentNode = -1);
		/**
		 * Calls evaluation from aiHeuristic
		 */
		float evaluate(aiBoard::Ptr board);
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
		static QList<QList<int> > getMoveSequences(aiBoard::Ptr board, KSquares::BoardAnalysis &analysis);
		
		void setTimeout(long timeout);
		long getTimeout();
		
		void setDebug(bool val);
		QString getDebugDot();
		
	protected:
		/// The ID of the player this AI belongs to
		int playerId;
		/// number of players - 1
		int maxPlayerId;
		/// board width in squares
		int width;
		/// board height in squares
		int height;
		/// The strength of the ai
		int level;
		/// number of lines on board
		//int linesSize;
		/// List of the owners of each square
		QList<int> squareOwners;
		/// Array of the lines on the board
		bool *lines;
		/// used to analyse boards
		aiHeuristic *heuristic;
		/// search depth
		int searchDepth;
		
		/// enable debugging
		bool debug;
		QString debugDot;
		int debugNodeCnt;
		long maxEvalTime;
		long lastEvalTime;
		QElapsedTimer alphabetaTimer;
		long alphabetaTimeout;
};

#endif

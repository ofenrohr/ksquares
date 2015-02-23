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
#include "board.h"
#include "aiBoard.h"
#include "aiHeuristic.h"

#include <QString>
#include <QElapsedTimer>
#include <QHash>
#include <QPair>
#include <QSharedPointer>

#include <cmath>


class aiAlphaBeta : public KSquaresAi
{
	public:
		aiAlphaBeta(int newPlayerId, int newMaxPlayerId, int newWidth, int newHeight, int newLevel, int thinkTime = 5000);
		~aiAlphaBeta();
		
		int chooseLine(const QList<bool> &newLines, const QList<int> &newSquareOwners, const QList<Board::Move> &lineHistory);
		QString getName() { return "alphabeta"; }
		virtual bool tainted() { return false; }
		virtual long lastMoveTime() { return 0; }
		
		void setDepth(int d) { searchDepth = d; }
		float alphabeta(aiBoard::Ptr board, int depth, int *line, float alpha = -INFINITY, float beta = INFINITY/*, int parentNode = -1*/);
		float alphabetaIterativeDeepening(aiBoard::Ptr board, int depth, int *line);
		/**
		 * Calls evaluation from aiHeuristic
		 */
		float evaluate(aiBoard::Ptr board);
		/**
		 * Gets board analysis. Uses a QMultiHash to store previous analsyis.
		 */
		KSquares::BoardAnalysis getAnalysis(aiBoard::Ptr board);
		/**
		 * set time in ms until alphabeta aborts search
		 */
		void setTimeout(long timeout);
		long getTimeout();
		
		void setDebug(bool val);
		void setDebugDepth(int d);
		void setDebugEvalOnly(bool e);
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
		///
		QList<int> linePool;
		/// circular sorting map for board lines (see j. k. barker and r. e. korf - solving dots-and-boxes)
		QList<int> lineSortList;
		/// map of previous board analysis
		QHash<aiBoard::Ptr, QPair<bool *, KSquares::BoardAnalysis> > *analysisHash;
		/// remember how many turns ai played
		int turn;
		
		/// enable debugging
		bool debug;
		int debugDepth;
		QString debugDot;
		int debugNodeCnt;
		bool debugEvalOnly;
		bool timerHasExpiredBefore;
		long maxEvalTime;
		long lastEvalTime;
		QElapsedTimer alphabetaTimer;
		long alphabetaTimeout;
};

#endif

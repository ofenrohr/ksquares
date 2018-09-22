/***************************************************************************
 *   Copyright (C) 2006 by Matthew Williams    <matt@milliams.com>         *
 *   Copyright (C) 2014 by Tom Vincent Peters  <kde@vincent-peters.de>     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef AICONTROLLER_H
#define AICONTROLLER_H

#include <QList>
#include <QSharedPointer>
#include <QObject>
#include <QThread>
#include <alphaDots/ModelInfo.h>
#include <gsl/gsl_rng.h>
#include "aifunctions.h"
#include "boardAnalysis.h"
#include "board.h"

#include "KSquaresAI.h"

/**
 * Class that is used to create and call all AIs available in KSquares.
 */
class aiController : public QObject
{
    Q_OBJECT
	public:
		typedef QSharedPointer<aiController> Ptr;
		
		/**
		 * Create a new AI controller
		 *
		 * @param newPlayerId ID of the player
		 * @param newLines list of the lines which are drawn
		 * @param newSquareOwners list of squares and their owners
		 * @param newWidth height of the current gameboard
		 * @param newHeight width of the current gameboard
		 * @param newLevel level of the ai
		 */
		aiController(int newPlayerId, int newMaxPlayerId, int newWidth, int newHeight, int newLevel, int thinkTime = 5000,
					 QString model = QStringLiteral("BasicStrategy"), bool gpu = false);
		
		~aiController();
		
		/**
		 * Choses where to draw the line:
		 * Creates a list of all the squares which are surrounded by 3 lines and if the list isn't empty, randomly picks one of them.
		 * Otherwise, creates a list of all the squares which are surrounded by 1 or 2 lines and if the list isn't empty, randomly chooses one of them.
		 * Otherwise, randomly chooses a square which is surrounded by three lines.
		 *
		 * @return The index of the line from "QVector<bool> lines"
		 */
		int chooseLine(const QList<bool> &newLines, const QList<int> &newSquareOwners, const QList<Board::Move> &lineHistory);
		/**
		 * Finds lines that can be filled without causing squares to be surrounded by 3 lines as a result.
		 * @param safeMovesLeft number of safe moves that can be performed after those returned by the function are drawn (note: the number is valid only for a certain sequence, for other sequences they could either be more or less)
		 * 
		 * @return the list of lines that can be safely drawn
		 */
		static QList<int> autoFill(int safeMovesLeft, int width, int height);
		
		KSquaresAi::Ptr getAi();
		static int getMaxAiLevel() { return 13; }
		
		long lastMoveTime() {return lastTurnTime;}

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
		KSquaresAi::Ptr ai;
		/// time in ms for ai to come up with move
		int aiThinkTime;
		/// time logging
		long lastTurnTime;
		/// name of alphaDots model
		AlphaDots::ModelInfo alphaDotsModel;
		/// alphaDots active?
		bool alphaDotsActive;
		/// allow gpu usage
		bool useGPU;
};


#endif // KSQUARES_H

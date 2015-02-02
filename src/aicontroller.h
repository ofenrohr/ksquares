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
#include "aifunctions.h"
#include "boardAnalysis.h"
#include "board.h"

/**
 * TODO: move this comment to aiEasyMediumHard and write correct comment.
 * @short AI Controller for KSquares
 *
 * When playing a game of squares there are a number of stages the game goes through:
 * @li The random line placement stage. Where players are just placing lines while trying to not complete the third side of any squares
 * @li Next players will try to only draw the third side of a square if it will only give the opponent the minimum amount of points
 * @li The more advanced player will, at the end of a large run of squares leave a small area at the end, forcing the opponent to take only that small section, leaving another large area open to him.
 * @li The even more advanced player will fight for control over the game. This means that he will count the chains forming in the last phase of the "random line" game phase and thus make sure that he will be the one who gets the first long chain. This works like a Nim game somehow.
 * Currently, the first three points are implemented.
 * 
 * @author Matt Williams <matt@milliams.com>
 * @author Tom Vincent Peters <kde@vincent-peters.de>
 */

class KSquaresAi : public aiFunctions, public BoardAnalysisFunctions
{
	public:
		typedef QSharedPointer<KSquaresAi> Ptr;
		KSquaresAi(int w, int h) : aiFunctions(w, h) {}
		virtual ~KSquaresAi() {}
		// call constructor with width, height, playerId, aiLevel
		//virtual ~KSquaresAi() = 0; 
		virtual int chooseLine(const QList<bool> &newLines, const QList<int> &newSquareOwners) = 0;
		virtual QString getName() = 0;
};

class aiController : public QObject
{
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
		aiController(int newPlayerId, int newMaxPlayerId, int newWidth, int newHeight, int newLevel, int thinkTime = 5000);
		
		~aiController();
		
		/**
		 * Choses where to draw the line:
		 * Creates a list of all the squares which are surrounded by 3 lines and if the list isn't empty, randomly picks one of them.
		 * Otherwise, creates a list of all the squares which are surrounded by 1 or 2 lines and if the list isn't empty, randomly chooses one of them.
		 * Otherwise, randomly chooses a square which is surrounded by three lines.
		 *
		 * @return The index of the line from "QVector<bool> lines"
		 */
		int chooseLine(const QList<bool> &newLines, const QList<int> &newSquareOwners);
		/**
		 * Finds lines that can be filled without causing squares to be surrounded by 3 lines as a result.
		 * @param safeMovesLeft number of safe moves that can be performed after those returned by the function are drawn (note: the number is valid only for a certain sequence, for other sequences they could either be more or less)
		 * 
		 * @return the list of lines that can be safely drawn
		 */
		static QList<int> autoFill(int safeMovesLeft, int width, int height);
		
		KSquaresAi::Ptr getAi();
		static int getMaxAiLevel() { return 5; }

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
};

// see http://qt-project.org/doc/qt-4.8/qthread.html#details
class aiControllerWorker : public QObject
{
	Q_OBJECT
	QThread workerThread;
	aiController::Ptr aicontroller;
	QList<bool> lines;
	QList<int> squares;
	
	public:
		aiControllerWorker(aiController::Ptr aic, const QList<bool> &newLines, const QList<int> &newSquareOwners)
		{
			aicontroller = aic;
			lines = newLines;
			squares = newSquareOwners;
		}

	public slots:
		void process()
		//void chooseLine(const QList<bool> &newLines, const QList<int> &newSquareOwners)
		{
			int line = aicontroller->chooseLine(lines, squares);
			emit lineChosen(line);
			emit finished();
		}

	signals:
		void lineChosen(const int &result);
		void finished();
};

#endif // KSQUARES_H

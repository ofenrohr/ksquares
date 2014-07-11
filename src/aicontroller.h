/***************************************************************************
 *   Copyright (C) 2006 by Matthew Williams    <matt@milliams.com>         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef AICONTROLLER_H
#define AICONTROLLER_H

#include <QList>
#include "aifunctions.h"

/**
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

class aiController : public aiFunctions
{
	public:
		/**
		 * Create a new AI controller
		 *
		 * @param newPlayerId ID of the player
		 * @param newLines list of the lines which are drawn
		 * @param newSquareOwners list of squares and their owners
		 * @param newWidth height of the current gameboard
		 * @param newHeight width of the current gameboard
		 */
		aiController(int newPlayerId, const QList<bool> &newLines, const QList<int> &newSquareOwners, int newWidth, int newHeight);
		
		~aiController();
		
		/**
		 * Choses where to draw the line:
		 * Creates a list of all the squares which are surrounded by 3 lines and if the list isn't empty, randomly picks one of them.
		 * Otherwise, creates a list of all the squares which are surrounded by 1 or 2 lines and if the list isn't empty, randomly chooses one of them.
		 * Otherwise, randomly chooses a square which is surrounded by three lines.
		 *
		 * @return The index of the line from "QVector<bool> lines"
		 */
		int chooseLine() const;
		/**
		 * Finds lines that can be filled without causing squares to be surrounded by 3 lines as a result.
		 * @param safeMovesLeft number of safe moves that can be performed after those returned by the function are drawn (note: the number is valid only for a certain sequence, for other sequences they could either be more or less)
		 * 
		 * @return the list of lines that can be safely drawn
		 */
		QList<int> autoFill(int safeMovesLeft);

	protected:
		/**
		 * @return list of moves that are safe (squares surrounded by 2 lines are avoided)
		 */
		QList<int> safeMoves() const;
		/**
		 * @param choiceList list of indices (of lines) which have squares next to them with two lines drawn (relates to @ref lines )
		 *
		 * @return list of indices (of lines) which would be the least damaging in the short term
		 */
		QList<int> chooseLeastDamaging(const QList<int> &choiceList) const;
		
		/// List of the owners of each square
		QList<int> squareOwners;
		/// List of which lines are complete
		int linesSize;
		bool *lines;
		/// The ID of the player this AI belongs to
		int playerId;
};

#endif // KSQUARES_H

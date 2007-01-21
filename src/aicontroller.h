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

namespace KSquares {enum Direction {HORIZONTAL, VERTICAL};}

/**
 * @short AI Controller for KSquares
 *
 * When playing a game of squares there are a number of stages the game goes through:
 * @li The random line placement stage. Where players are just placing lines while trying to not complete the third side of any squares
 * @li Next players will try to only draw the third side of a square if it will only give the opponent the minimum amount of points
 * @li Finally, the more advanced player will, at the end of a large run of squares leave a small area at the end, forcing the opponent to take only that small section, leaving another large area open to him.
 * Currently, the first two points are implemented.
 * 
 * @author Matt Williams <matt@milliams.com>
 */

class aiController
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
		aiController(int newPlayerId, QList<bool> newLines, QList<int> newSquareOwners, int newWidth, int newHeight);
		/**
		 * Choses where to draw the line:
		 * Creates a list of all the squares which are surrounded by 3 lines and if the list isn't empty, randomly picks one of them.
		 * Otherwise, creates a list of all the squares which are surrounded by 1 or 2 lines and if the list isn't empty, randomly chooses one of them.
		 * Otherwise, randomly chooses a square which is surrounded by three lines.
		 *
		 * @return The index of the line from "QVector<bool> lines"
		 */
		int chooseLine();

	protected:
		/**
		 * @param choiceList list of indices (of lines) which have squares next to them with two lines drawn (relates to @ref lines )
		 *
		 * @return list of indices (of lines) which would be the least damaging in the short term
		 */
		QList<int> chooseLeastDamaging(QList<int> choiceList);
		/**
		 * @param squareIndex the index of the square (relates to @ref squareOwners )
		 * @param linesList the linesList you want to work from
		 *
		 * @return the number of lines currently drawn around a specific square
		 */
		int countBorderLines(int squareIndex, QList<bool> linesList) const;
		/**
		 * @param lineIndex the index of the line (relates to @ref lines )
		 *
		 * @return the (one or two) squares abutting a line
		 */
		QList<int> squaresFromLine(int lineIndex) const;
		/**
		 * @param squareIndex the index of the square (relates to @ref squareOwners )
		 *
		 * @return the indices of the four lines surrounding the square
		 */
		QList<int> linesFromSquare(int squareIndex) const;
		/**
		 * @param lineIndex the index of the line (relates to @ref lines )
		 *
		 * @return the direction of the line
		 */
		KSquares::Direction lineDirection(int lineIndex) const;
		
		/// List of the owners of each square
		QList<int> squareOwners;
		/// List of which lines are complete
		QList<bool> lines;
		/// The ID of the player this AI belongs to
		int playerId;
		/// Width of the game board
		int width;
		/// Height of the game board
		int height;
};

#endif // KSQUARES_H

/***************************************************************************
 *   Copyright (C) 2006 by Tom Vincent Peters   <kde@vincent-peters.de>    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef AIFUNCTIONS_H
#define AIFUNCTIONS_H

#include <QList>
#include "board.h"

// TODO: use static functions, provide non-static functions with less parameters
class aiFunctions {
	public:
		aiFunctions(int w, int h);
	protected:
		/**
		 * @param squareIndex the index of the square (relates to @ref squareOwners )
		 * @param linesList the linesList you want to work from
		 *
		 * @return the number of lines currently drawn around a specific square
		 */
		int countBorderLines(int squareIndex, const bool *linesList) const;
		/**
		 * @param sidesOfSquare output parameter: the indices of the four lines surrounding the square
		 * @param squareIndex the index of the square (relates to @ref squareOwners )
		 * @param linesList the linesList you want to work from
		 *
		 * @return the number of lines currently drawn around a specific square
		 */
		int countBorderLines(int *sidesOfSquare, int squareIndex, const bool *linesList) const;
		/**
		 * @param lineIndex the index of the line (relates to @ref lines )
		 *
		 * @return the (one or two) squares abutting a line
		 */
		QList<int> squaresFromLine(int lineIndex) const;
		/**
		 * @param sidesOfSquare output parameter: the indices of the four lines surrounding the square
		 * @param squareIndex the index of the square (relates to @ref squareOwners )
		 */
		void linesFromSquare(int *sidesOfSquare, int squareIndex) const;
		/**
		 * @param lineIndex the index of the line (relates to @ref lines )
		 *
		 * @return the direction of the line
		 */
		KSquares::Direction lineDirection(int lineIndex) const;
		
		/*=========================================*/
		// TODO: add doc
		QList<int> findLinesCompletingBoxes(int linesSize, const bool *lines) const;
		/**
		 * @return list of moves that are safe (squares surrounded by 2 lines are avoided)
		 */
		QList<int> safeMoves(int linesSize, const bool *lines) const;
		/**
		 * Finds chains on board
		 * @param lines lines of the board
		 * @param width width of board in boxes
		 * @param height height of board in boxes
		 * @param ownChains returns the chains the current player can score
		 */
		void findOwnChains(const QList<int> *lines, int linesSize, int width, int height, QList<QList<int> > *ownChains);
		/**
		 * @param chainLines list of lines the chain is made up of
		 * @return 0: long chain, 1: short chain, 2: loop chain
		 */
		int classifyChain(const QList<int> chainLines);
		
		/// Width of the game board
		int width;
		/// Height of the game board
		int height;
};

#endif // AIFUNCTIONS_H
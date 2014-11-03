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
#include <QString>
#include <QSharedPointer>
#include "board.h"

// TODO: use static functions, provide non-static functions with less parameters
class aiBoard {
	public:
		typedef QSharedPointer<aiBoard> Ptr;
		bool *lines_;
		int linesSize_; 
		int width_;
		int height_;
};

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
		static int countBorderLines(int width, int height, int squareIndex, const bool *linesList);
		/**
		 * @param sidesOfSquare output parameter: the indices of the four lines surrounding the square
		 * @param squareIndex the index of the square (relates to @ref squareOwners )
		 * @param linesList the linesList you want to work from
		 *
		 * @return the number of lines currently drawn around a specific square
		 */
		int countBorderLines(int *sidesOfSquare, int squareIndex, const bool *linesList) const;
		static int countBorderLines(int width, int height, int *sidesOfSquare, int squareIndex, const bool *linesList);
		/**
		 * @param lineIndex the index of the line (relates to @ref lines )
		 *
		 * @return the (one or two) squares abutting a line
		 */
		QList<int> squaresFromLine(int lineIndex) const;
		static QList<int> squaresFromLine(int width, int height, int lineIndex);
		/**
		 * @param sidesOfSquare output parameter: the indices of the four lines surrounding the square
		 * @param squareIndex the index of the square (relates to @ref squareOwners )
		 */
		void linesFromSquare(int *sidesOfSquare, int squareIndex) const;
		static void linesFromSquare(int width, int height, int *sidesOfSquare, int squareIndex);
		/**
		 * @param lineIndex the index of the line (relates to @ref lines )
		 *
		 * @return the direction of the line
		 */
		KSquares::Direction lineDirection(int lineIndex) const;
		static KSquares::Direction lineDirection(int width, int height, int lineIndex);
		
		/*=========================================*/
		// TODO: add doc
		QList<int> findLinesCompletingBoxes(int linesSize, const bool *lines) const;
		static QList<int> findLinesCompletingBoxes(int width, int height, int linesSize, const bool *lines);
		/**
		 * @return list of moves that are safe (squares surrounded by 2 lines are avoided)
		 */
		QList<int> safeMoves(int linesSize, const bool *lines) const;
		static QList<int> safeMoves(int width, int height, int linesSize, const bool *lines);
		/**
		 * Finds chains on board that can be taken by the player to move
		 * @param lines lines of the board
		 * @param width width of board in boxes
		 * @param height height of board in boxes
		 * @param ownChains returns the chains the current player can score
		 */
		void findOwnChains(bool *lines, int linesSize, int width, int height, QList<QList<int> > *ownChains) const;
		/**
		 * @param chainLines list of lines the chain is made up of
		 * @return 0: long chain, 1: short chain, 2: loop chain
		 */
		int classifyChain(const QList<int> chainLines);
		
		/* Debugging */
		static QString boardToString(bool *lines, int linesSize, int width, int height);
		
		/// Width of the game board
		int width;
		/// Height of the game board
		int height;
};

#endif // AIFUNCTIONS_H
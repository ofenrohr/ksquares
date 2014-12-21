/***************************************************************************
 *   Copyright (C) 2014 by Tom Vincent Peters   <kde@vincent-peters.de>    *
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
#include <QMap>
#include <QPair>

#include "board.h"
#include "aiBoard.h"

namespace KSquares
{
	enum ChainType {CHAIN_SHORT, CHAIN_LONG, CHAIN_LOOP, CHAIN_UNKNOWN};
	typedef struct Chain_t
	{
		QList<int> lines;
		QList<int> squares;
		ChainType type;
		bool ownChain;
	} Chain;
}


class aiFunctions 
{
	public:
		aiFunctions(int w, int h);
		
		static int toLinesSize(int w, int h);
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
		 * @param linesSize number of lines on the board
		 * @param width width of board in boxes
		 * @param height height of board in boxes
		 * @param ownChains returns the chains the current player can score
     * @return number of squares taken
		 */
		static int findOwnChains(bool *lines, int linesSize, int width, int height, QList<QList<int> > *ownChains);
		/**
		 * Checks if a square is connected to a joint square (either ground or a square with 0 or 1 lines drawn)
		 */
		static bool squareConnectedToJoint(aiBoard::Ptr board, QMap<int, int> &squareValences, int square);
		/**
		 * Finds the squares connected to given square
		 * @param board the board to operate on
		 * @param square the square in question
		 * @return List of pairs (first: line index, second: square index) adjacent to given square
		 */
		static QList<QPair<int, int> > getConnectedSquares(aiBoard::Ptr board, int square);
		/**
		 * Reverses a QList.
		 */
		template <typename T> static QList<T> reverseQList(const QList<T> & in);
		/**
		 * Returns all lines of a square that are connected to ground. If includeCutConnections is set drawn lines will be included
		 */
		static QList<int> getGroundConnections(aiBoard::Ptr board, int square, bool includeCutConnections = false);
		/**
		 * Find chains on board.
		 * WARNING: closed long chains will be classified as loop chains because a 4 square sacrifice is required for double dealing.
		 * @param board the board to analyze
		 * @param ownChains returns the chains on the board
		 */
		static void findChains(aiBoard::Ptr board, QList<KSquares::Chain> *chains);
		/**
		 * Classifies a given chain as short, long or loop chain.
		 * @param chain list of lines the chain is made up of
     * @param lines lines of the board
		 * @return 0: long chain, 1: short chain, 2: loop chain, -1: no chain
		 */
		KSquares::ChainType classifyChain(const QList<int> &chain, bool *lines) const;
		static KSquares::ChainType classifyChain(int width, int height, const QList<int> &chain, bool *lines);
		static KSquares::ChainType classifyChain(int width, int height, const QList<int> &chain, bool *lines, QList<int> *squares);
		/**
		 * TODO: move to aiBoard
		 * Gets lines that are not drawn
		 * @param lines lines of the board
		 * @param linesSize number of lines on the board
		 */
		static QList<int> getFreeLines(bool *lines, int linesSize);
		/**
		 * TODO
		 * @return index = player id, value = number of squares
		 */
		static QMap<int, int> getScoreMap(QList<int> &squareOwners);
		/**
		 * TODO: move to aiBoard
		 * Determines which player has the most squares
		 * @return playerId of player with most squares, -1 if no squares are drawn, -2 if draw
		 */
		static int getLeader(QList<int> &squareOwners);
		
		/* Debugging */
		static QString boardToString(bool *lines, int linesSize, int width, int height);
    QString boardToString(bool *lines) const;
    static QString linelistToString(const QList<int> list, int linesSize, int width, int height);
    QString linelistToString(const QList<int> list) const;
		
	protected:
		/// Width of the game board
		int width;
		/// Height of the game board
		int height;
    /// Size of list of which lines are complete
    int linesSize;
};

#endif // AIFUNCTIONS_H
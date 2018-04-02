/***************************************************************************
 *   Copyright (C) 2014 by Tom Vincent Peters    <kde@vincent-peters.de>   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef BOARD_H
#define BOARD_H

// qt
#include <QObject>
#include <QPoint>
#include <QPair>
#include <QDebug>


namespace KSquares {enum Direction {HORIZONTAL, VERTICAL};}

class Board : public QObject
{
	Q_OBJECT

	public:
		Board();
		Board(int numOfPlayers, int width, int height);
	
		typedef struct Move_t
		{
			int line;
			int player;
			QList<int> squares;
			
			friend QDebug operator<<(QDebug dbg, const Board::Move_t &m) { dbg.nospace() << "Move(line: " << m.line << ", player: " << m.player << ", squares: " << m.squares << ")"; return dbg.maybeSpace(); }
        } Move;

		/**
		 * resets the board
		 */
		void reset();

		/**
		 * @param index index of the line to add
		 * @param nextPlayer true if the line ends the turn of the current player
		 * @param boardFilled true if the last line was added
		 * @param completedSquares list of completed squares
		 * @return true if line was added, false otherwise
		 */
		bool addLine(int lineIndex, bool *nextPlayer, bool *boardFilled, QList<int> *completedSquares);

		/**
		 * Undo last move.
		 * @param nextPlayer index of current player after undo
		 * @return true if one or more lines were undone
		 */
		bool undo(int *nextPlayer);
		
		/**
		 * @return true if the line completes at least one square, false otherwise
		 */
		bool lineCompletesSquare(int lineIndex, QList<int> *completedSquares);
		
		/**
		 * @param p1 Point 1 of the line (one dot)
		 * @param p2 Point 2 of the line (second dot)
		 * @return -1 if the points don't specify a valid line, index of line otherwise
		 */
		int pointsToIndex(QPoint p1, QPoint p2);
		
		/**
		 * @param p1 Point 1 of the line (one dot)
		 * @param p2 Point 2 of the line (second dot)
		 * @param w width of the board in boxes
		 * @param h height of the board in boxes
		 * @return -1 if the points don't specify a valid line, index of line otherwise
		 */
		static int pointsToIndex(QPoint p1, QPoint p2, int w, int h);

		/**
		 * @param lineIndex the index of the line
		 * @param p1 the first resulting point of the line
		 * @param p2 the second resulting point of the line
		 * @return true if conversion was successful, false otherwise
		 */
		bool indexToPoints(const int lineIndex, QPoint *p1, QPoint *p2);
		
		/**
		 * @param lineIndex the index of the line
		 * @param p1 the first resulting point of the line
		 * @param p2 the second resulting point of the line
		 * @param width board width in boxes
		 * @param height board height in boxes
		 * @return true if conversion was successful, false otherwise
		 */
		static bool indexToPoints(const int lineIndex, QPoint *p1, QPoint *p2, const int width, const int height, const bool invert = true);
		/**
		 * @param a first point on dots and boxes board (upper / left point)
		 * @param b second piont on dots and boxes board (lower / right point)
		 * @param w board width in boxes
		 * @param h board height in boxes
		 */
		static QPair<QPoint, QPoint> pointsToCoins(QPoint a, QPoint b, int w, int h);
		
		/**
		 * @param a first coin on strings & coins board (upper / left coin)
		 * @param b second coin on strings & coins board (lower / right coin)
		 * @param w board width in coins
		 * @param h board height in coins
		 */
		static QPair<QPoint, QPoint> coinsToPoints(QPoint a, QPoint b, int w, int h);
		
		/**
		 * @param sidesOfSquare output parameter: the indices of the four lines surrounding the square
		 * @param squareIndex the index of the square (relates to @ref squareOwners )
		 */
		void linesFromSquare(QList<int> *sidesOfSquare, int squareIndex) const;
		
		/**
		 * @param lineIndex the index of the line (relates to @ref lines )
		 * @return the (one or two) squares abutting a line
		 */
		QList<int> squaresFromLine(int lineIndex) const;
		
		/**
		 * @param sidesOfSquare output parameter: the indices of the four lines surrounding the square
		 * @param squareIndex the index of the square (relates to @ref squareOwners )
		 * @param linesList the linesList you want to work from
		 *
		 * @return the number of lines currently drawn around a specific square
		 */
		int countBorderLines(QList<int> *sidesOfSquare, int squareIndex, const QList<bool> *linesList) const;
		
		/**
		 * @param squareIndex the index of the square (relates to @ref squareOwners )
		 * @param linesList the linesList you want to work from
		 *
		 * @return the number of lines currently drawn around a specific square
		 */
		int countBorderLines(int squareIndex, const QList<bool> *linesList) const;
		
		/**
		 * @param lineIndex the index of the line (relates to @ref lines )
		 *
		 * @return the direction of the line
		 */
		KSquares::Direction lineDirection(int lineIndex) const;
		static KSquares::Direction lineDirection(int lineIndex, const int width);
		
		/**
		 * @param playerIndex the index of the player
		 */
		void setCurrentPlayer(int playerIndex);
		
		//getters
		/**
		 * @return the table of currently owned squares
		 */
		const QList<int> squares() const {return squareOwnerTable_;}
		/**
		 * @return the list of lines
		 */
		const QList<bool> lines() const {return lineList_;}
		/**
		 * @return the width of the game board
		 */
		int width() const {return width_;}
		/**
		 * @return the height of the game board
		 */
		int height() const {return height_;}
		/**
		 * @return the id of the current player
		 */
		int currentPlayer() const {return currentPlayerId_;}
        /**
         * @return the history of drawn lines
         */
        QList<Move> getLineHistory() const {return lineHistory_;}
        /**
		 * @return number of players
		 */
		int getNumOfPlayers() const {return numOfPlayers_;}

		QString toString() const;

		//setters
		void setNumOfPlayers(int numOfPlayers) {numOfPlayers_ = numOfPlayers;}
		void setSize(int width, int height);
	protected:
		///Number of players in this game
		int numOfPlayers_;
		///Id of the current player
		int currentPlayerId_;
		///Width of the game board
		int width_;
		/// Height of the game board
		int height_;
		///List of the squares and their owners
		QList<int> squareOwnerTable_;
		///List of the lines and whether they're drawn
		QList<bool> lineList_;

    ///History of drawn lines
    QList<Move> lineHistory_;
};
#endif // BOARD_H

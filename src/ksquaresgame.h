/***************************************************************************
 *   Copyright (C) 2006 by Matthew Williams    <matt@milliams.com>         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef KSQUARESGAME_H
#define KSQUARESGAME_H

#include <QObject>
#include <QVector>
#include <QList>

#include "ksquaresplayer.h"
#include "board.h"

class QColor;

/**
 * @short The game controller
 *
 * Keeps charge of the game. Everything you'd expect really.
 * 
 * - Create one instance of this class which will last the whole time the program is running.
 * 
 * - In order to start a (new) game just call @ref createGame() with the appropriate arguments.
 * 
 * - Once the game is started, play passes to the first player. @ref takeTurnSig() will be emitted to allow you to chose how the go should be taken (AI controller or 'click' from a View).
 * 
 * - However the turn is taken, to make the move, the @ref addLineToIndex() function must be called. This will emit the @ref drawLine() signal to allow you to draw the line on the board with the correct colours.
 * 
 * - If the player completed a square, @ref drawSquare() will then be emitted to allow you to draw the completed square with the correct colour.
 * 
 * - If the player gets another go, @ref takeTurnSig() will be emitted again. If not, play will pass to the next player and @ref takeTurnSig() will be emitted for them.
 * 
 * - If a player completes the scoreboard, @ref gameOver() will be emitted with the full list of players to allow you to construct a score board.
 * 
 * - All variables will remain in the state they were at the end of the game until @ref createGame() is called again.
 * 
 * @author Matt Williams <matt@milliams.com>
 */

class KSquaresGame : public QObject
{
	Q_OBJECT
	
	public:
		///Constructor
		KSquaresGame();
		~KSquaresGame();
		
		/**
		 * Creates a list of players
		 * 
		 * @param cnt number of players
		 * @param isHuman list of (non)human players
		 * @return list of players
		 */
		static QVector<KSquaresPlayer> createPlayers(int cnt, QList<int> isHuman);
		/**
		 * Create a new game
		 * 
		 * @param startPlayers list of the players in the game
		 * @param startWidth the width of the game board
		 * @param startHeight the height of the game board
		 */
		void createGame(const QVector<KSquaresPlayer> &startPlayers, int startWidth, int startHeight);
		/**
		 * Starts the game
		 */
		void start() {gameInProgress = true;}
		/**
		 * Stops the game
		 */
		void stop() {gameInProgress = false;}
		/**
		 * Status of the game
		 */
		bool isRunning() {return gameInProgress;}
		/**
		 * Externally determined player switch, for network game
		 */
		void switchPlayer();
		/**
		 * @return the id of the current player. 0 >= id \< number of players
		 */
		int currentPlayerId() const {return board_.currentPlayer();}
		/**
		 * @return the current player
		 */
		KSquaresPlayer* currentPlayer() {return &players[currentPlayerId()];}
		/**
		 * @return the game board
		 */
		Board* board() {return &board_;}
		/**
		 * @return list of players
		 */
		QVector<KSquaresPlayer> getPlayers() {return players;}

	public Q_SLOTS:
		/**
		 * @param index the index of the line to add
		 */
		bool addLineToIndex(int index);

	protected:
		/**
		 * Moves play control to the next player, looping round when necessary
		 * 
		 * @return the Id of the player who's turn just started
		 */
		int nextPlayer();
		///Sets lots of things to zero, clears lists etc.
		void resetEverything();
		
		///A line was drawn, see if the player gets another go
		void tryEndGo();
		///Scans the board to see if any new squares were completed
		void checkForNewSquares();
		/**
		 * A player completed a square. Emits the lineDrawn() signal. Checks to see if the game is over.
		 *
		 * @param index the index of the square which was completed
		 * @param playerIndex the index of the player who completed the square
		 */
		void playerSquareComplete(int index, int playerIndex);
		
		// Static throughout each game
		Board board_;
		
		// Updated as the game progresses
		///List of all the players in the game
		QVector<KSquaresPlayer> players;
		
		// Probably changes every go
		/// is there currently a game in progress
		bool gameInProgress;
		/// last line added
		int lastLine;
	signals:
		///A player's turn has started. This allows you to use AI/networking etc.
		void takeTurnSig(KSquaresPlayer*);	//emit the new curent player
		///emitted when the game board is completed. Allows you to construct a scoreboard
		void gameOver(const QVector<KSquaresPlayer> &);	//for scoreboard purposes
		///Emits the index and colour of the line
		void drawLine(int,QColor);	//int == lineList index
		///Emits the index and colour of the square
		void drawSquare(int,QColor);	//int == squareOwnerTable index
		///Emitted when the last move in a series is played by the AI
		void highlightMove(int);
};

#endif // KSQUARESGAME_H

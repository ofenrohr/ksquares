/***************************************************************************
 *   Copyright (C) 2006 by Matthew Williams    <matt@milliams.com>         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "ksquaresgame.h"

#include <KDebug>

//generated
#include "settings.h"

KSquaresGame::KSquaresGame()
{
	kDebug() << "Constructing Game";
	gameInProgress = false;
}

KSquaresGame::~KSquaresGame()
{
	kDebug() << "Destroying game";
	gameInProgress = false;
}


QVector<KSquaresPlayer> KSquaresGame::createPlayers(int cnt, QList<int> isHuman)
{
  //create players
  QVector<KSquaresPlayer> playerList;
  for(int i=0; i<cnt; i++)
  {
    QColor color;
    switch(i)
    {
      case 0: //Red
        color = QColor(191,3,3); //or darker: (156,15,15);
        break;
      case 1: //Blue
        color = QColor(0,67,138); //or darker: (0,49,110);
        break;
      case 2: //Green
        color = QColor(0,137,44); //or darker: (0,110,41);
        break;
      case 3: //Yellow
        color = QColor(243,195,0); //or darker: (227,173,0);
        break;
      default:
        kError() << "KSquaresGame::playerSquareComplete(); currentPlayerId() != 0|1|2|3";
    }
    playerList.append(KSquaresPlayer(Settings::playerNames().at(i), color, isHuman.at(i)));
  }
  
  return playerList;
}

void KSquaresGame::createGame(const QVector<KSquaresPlayer> &startPlayers, int startWidth, int startHeight)
{
	resetEverything();	//reset everything
	kDebug() << "Creating Game with" << startPlayers.size() << "player(s)";
	
	//BEGIN Initialisation
	board_.setSize(startWidth, startHeight);
	board_.setNumOfPlayers(startPlayers.size());
	for(int i=0; i < startPlayers.size(); i++)
	{
		players.append(startPlayers[i]);
	}
	//END Initialisation
	
	kDebug() << "Game Starting";
	
	emit takeTurnSig(currentPlayer());
}

void KSquaresGame::resetEverything()
{
	kDebug() << "Game Values Resetting";
	board_.reset();
	players.resize(0);
	gameInProgress = false;
	lastLine = -1;
}

void KSquaresGame::addLineToIndex(int index)
{
	kDebug() << "KSquaresGame::addLineToIndex";
	bool nextPlayer;
	bool boardFilled;
	QList<int> completedSquares;
	int drawingPlayerIndex = currentPlayerId();
	
	// try to add the line
	if (!board_.addLine(index, &nextPlayer, &boardFilled, &completedSquares))
	{
		kDebug() << "Warning: tryied to add invalid / already taken line with index " << index;
		return;
	}
	kDebug() << "added line. index: " << index << ", next player: " << nextPlayer << ", board filled: " << boardFilled << ", completed squares count: " << completedSquares.size();
	// draw the line
	emit drawLine(index, Settings::lineColor());
	// draw the completed squares
	for (int i = 0; i < completedSquares.size(); i++)
	{
		players[drawingPlayerIndex].incScore();
		emit drawSquare(completedSquares.at(i), players.at(drawingPlayerIndex).colour());
	}
	// check if game is over
	if (boardFilled && gameInProgress)
	{
		if (gameInProgress)
		{
			emit gameOver(players);
		}
		gameInProgress = false;
	}
	
	if (nextPlayer)
	{
		// the player's turn is over
		if (!players.at(drawingPlayerIndex).isHuman())
		{
			emit highlightMove(index);
		}
		if (gameInProgress)
		{
			emit takeTurnSig(currentPlayer());
		}
	}
	else
	{
		if (gameInProgress)
		{
			emit takeTurnSig(currentPlayer());
		}
	}
}

#include "ksquaresgame.moc"

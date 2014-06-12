/***************************************************************************
 *   Copyright (C) 2006 by Matthew Williams    <matt@milliams.com>         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "ksquaresgame.h"

#include <kdebug.h>

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
	
	connect(&board_, SIGNAL(squareComplete(int, int)), this, SLOT(playerSquareComplete(int, int)));
	//nextPlayer();
}

/*
void KSquaresGame::switchPlayer()
{
	//anotherGo = false;
	currentPlayerId() >= (players.size()-1) ? i_currentPlayerId = 0 : i_currentPlayerId++;
}
*/

/*
int KSquaresGame::nextPlayer()
{
	//anotherGo = false;	//just to reset the variable
	currentPlayerId() >= (players.size()-1) ? i_currentPlayerId = 0 : i_currentPlayerId++;
	kDebug()<< "- Moving to next player:" << currentPlayer()->name() << "(" << currentPlayerId() << ")";
	kDebug() << "-";
	emit takeTurnSig(currentPlayer());
	
	return currentPlayerId();
}
*/


void KSquaresGame::playerSquareComplete(int index, int playerIndex)
{
	kDebug() << "- - " << currentPlayer()->name() << "(" << currentPlayerId() << ") has completed a square";
	//anotherGo = true;
	
	//board_.squares()[index] = currentPlayerId();	//add square to index
	emit drawSquare(index, players.at(playerIndex).colour());
	players[playerIndex].incScore();
	
	/*
	int totalPoints=0;
	for (int i=0; i < players.size(); i++)
	{
		totalPoints += players.at(i).score();
	}
	kDebug() << "- - Square Completed";
	if (totalPoints >= board_.width()*board_.height())	//if the board is full
	{
		kDebug() << "Game Over";
		gameInProgress = false;
		emit gameOver(players);
	}
	*/
}
/*
void KSquaresGame::tryEndGo()
{
	kDebug() << "- - Trying to end go";
	if (anotherGo)
	{
		if(gameInProgress)
		{
			kDebug() << "- - - Having another go";
			kDebug() << "-";
			anotherGo = false;
			emit takeTurnSig(currentPlayer());
		}
	}
	else
	{
		kDebug() << "- - - Go ending";
		if (!currentPlayer()->isHuman())
		{
			emit highlightMove(lastLine);
		}
		nextPlayer();
	}
}
*/

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
	if (!board_.addLine(index, &nextPlayer, &boardFilled, &completedSquares))
	{
		return;
	}
	if (boardFilled && gameInProgress)
	{
		gameInProgress = false;
		emit gameOver(players);
	}
	emit drawLine(index, Settings::lineColor());
	if (nextPlayer)
	{
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
		for (int i = 0; i < completedSquares.size(); i++)
		{
			emit drawSquare(completedSquares.at(i), players.at(drawingPlayerIndex).colour());
		}
		if (gameInProgress)
		{
			emit takeTurnSig(currentPlayer());
		}
	}
	/*
	if (board_.lines()[index] == true)	//if there is already a line
	{
		kWarning() << "KSquaresGame::addLineToIndex():" 
				   << "trying to add line already there!";
		return;
	}
	board_.lines()[index] = true;
	lastLine = index;
	
	emit drawLine(index, Settings::lineColor());
	
	if (gameInProgress)
		checkForNewSquares();
	*/
}

/*
 * TODO: this should be done in the board
void KSquaresGame::checkForNewSquares()
{
	for(int i=0; i < (board_.width()*board_.height()); i++)	//cycle through every box..
	{
		if (board_.squares().at(i) == -1)	//..checking it if there is no current owner
		{
			//indices of the lines surrounding the box; Correlates to "QVector<bool> lineList"
			int index1 = (i/board_.width()) * ((2*board_.width()) + 1) + (i%board_.width());
			int index2 = index1 + board_.width();
			int index3 = index2 + 1;
			int index4 = index3 + board_.width();
			//cout << index1 << "," << index2 << "," << index3 << "," << index4 << " - " << lineList.size() << endl;
			if (board_.lines().at(index1) && board_.lines().at(index2) && board_.lines().at(index3) && board_.lines().at(index4))
			{
				kDebug() << "- - Square" << i << "completed.";
				playerSquareComplete(i);
			}
		}
	}
	//emit lineDrawnSig();
	tryEndGo();
}
*/

#include "ksquaresgame.moc"

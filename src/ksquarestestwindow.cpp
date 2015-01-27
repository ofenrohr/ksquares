/***************************************************************************
 *   Copyright (C) 2014 by Tom Vincent Peters   <kde@vincent-peters.de>    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

//header
#include "ksquarestestwindow.h"

//qt
#include <QTimer>

//kde
#include <KApplication>
#include <KStatusBar>
#include <KActionCollection>
#include <kdebug.h>
#include <KLocale>
#include <kstandardgameaction.h>

//generated
#include "settings.h"

//classes
#include "gameboardview.h"
#include "ksquaresio.h"

KSquaresTestWindow::KSquaresTestWindow() : KXmlGuiWindow(), m_view(new GameBoardView(this)), m_scene(0)
{
	sGame = new KSquaresGame();
	connect(sGame, SIGNAL(takeTurnSig(KSquaresPlayer*)), this, SLOT(playerTakeTurn(KSquaresPlayer*)));
	connect(sGame, SIGNAL(gameOver(QVector<KSquaresPlayer>)), this, SLOT(gameOver(QVector<KSquaresPlayer>)));

	m_view->setRenderHints(QPainter::Antialiasing);
	m_view->setFrameStyle(QFrame::NoFrame);
	m_view->setDisabled(true);
	setCentralWidget(m_view);

	KStandardGameAction::quit(kapp, SLOT(quit()), actionCollection());
	setupGUI();

	statusBar()->insertPermanentItem(i18n("Current Player"), 0);
	statusBar()->show();
}

void KSquaresTestWindow::gameNew()
{
	//create players
	QVector<KSquaresPlayer> playerList;
	for(int i=0; i<2; i++)
	{
		QColor color;
		switch(i)
		{
			case 0:
				color = Qt::red;
				break;
			case 1:
				color = Qt::blue;
				break;
			case 2:
				color = Qt::green;
				break;
			case 3:
				color = Qt::yellow;
				break;
			default:
				kError() << "KSquaresGame::playerSquareComplete(); currentPlayerId() != 0|1|2|3";
		}
		playerList.append(KSquaresPlayer(i18n("Player %1", i+1), color, false));
	}

	int width = 6;
	int height = 6;
	
	//create physical board
	GameBoardScene* temp = m_scene;
	m_scene = new GameBoardScene(width, height);

	m_view->setScene(m_scene);
	delete temp;

	m_view->setBoardSize();	//refresh board zooming

	// create AI players
	aiController::Ptr aic0(new aiController(0, 1, width, height, 2));
	aiController::Ptr aic1(new aiController(0, 1, width, height, 3));
	aiList.clear();
	aiList.append(aic0);
	aiList.append(aic1);
	
	//start game etc.
	sGame->createGame(playerList, width, height);
	connect(m_scene, SIGNAL(lineDrawn(int)), sGame, SLOT(addLineToIndex(int)));
	connect(sGame, SIGNAL(drawLine(int,QColor)), m_scene, SLOT(drawLine(int,QColor)));
	connect(sGame, SIGNAL(highlightMove(int)), m_scene, SLOT(highlightLine(int)));
	connect(sGame, SIGNAL(drawSquare(int,QColor)), m_scene, SLOT(drawSquare(int,QColor)));

	sGame->start();
	
	playerTakeTurn(sGame->currentPlayer());
}

void KSquaresTestWindow::playerTakeTurn(KSquaresPlayer* currentPlayer)
{
	statusBar()->changeItem(currentPlayer->name(), 0);
	QTimer::singleShot(200, this, SLOT(aiChooseLine()));
}

void KSquaresTestWindow::aiChooseLine()
{
	aiController::Ptr ai = aiList[sGame->currentPlayerId()];
	sGame->addLineToIndex(ai->chooseLine(sGame->board()->lines(), sGame->board()->squares()));
}

void KSquaresTestWindow::gameOver(const QVector<KSquaresPlayer> & playerList)
{
	kDebug() << "Game Over";
	kDebug() << "score hard ai: " << playerList[0].score();
	kDebug() << "score alphabeta ai: " << playerList[1].score();
	QTimer::singleShot(1000, this, SLOT(gameNew()));
}

#include "ksquarestestwindow.moc"

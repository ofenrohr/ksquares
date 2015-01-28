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

	m_view->setRenderHints(QPainter::Antialiasing);
	m_view->setFrameStyle(QFrame::NoFrame);
	m_view->setDisabled(true);
	setCentralWidget(m_view);

	KStandardGameAction::quit(kapp, SLOT(quit()), actionCollection());
	setupGUI();

	statusBar()->insertPermanentItem(i18n("Current Player"), 0);
	statusBar()->insertPermanentItem(i18n("Results"), 1);
	statusBar()->show();
	
	resultStr = "Hard: 0, AlphaBeta: 0";
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
		playerList.append(KSquaresPlayer(i==0?"Hard":"AlphaBeta", color, false));
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
	connect(sGame, SIGNAL(takeTurnSig(KSquaresPlayer*)), this, SLOT(playerTakeTurn(KSquaresPlayer*)));
	connect(sGame, SIGNAL(gameOver(QVector<KSquaresPlayer>)), this, SLOT(gameOver(QVector<KSquaresPlayer>)));

	sGame->start();
	
	playerTakeTurn(sGame->currentPlayer());
}

void KSquaresTestWindow::playerTakeTurn(KSquaresPlayer* currentPlayer)
{
	kDebug() << "playerTakeTurn";
	statusBar()->changeItem(currentPlayer->name(), 0);
	statusBar()->changeItem(resultStr, 1);
	aiChooseLine();
}

void KSquaresTestWindow::aiChooseLine()
{
	kDebug() << "aiChooseLine";
	// https://mayaposch.wordpress.com/2011/11/01/how-to-really-truly-use-qthreads-the-full-explanation/
	QThread* thread = new QThread;
	aiControllerWorker *worker = new aiControllerWorker(aiList[sGame->currentPlayerId()], sGame->board()->lines(), sGame->board()->squares());
	worker->moveToThread(thread);
	//connect(worker, SIGNAL(error(QString)), this, SLOT(errorString(QString)));
	connect(thread, SIGNAL(started()), worker, SLOT(process()));
	connect(worker, SIGNAL(finished()), thread, SLOT(quit()));
	connect(worker, SIGNAL(finished()), worker, SLOT(deleteLater()));
	connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
	connect(worker, SIGNAL(lineChosen(int)), this, SLOT(aiChoseLine(int)));
	thread->start();
}

void KSquaresTestWindow::aiChoseLine(const int &line)
{
	kDebug() << "aiChoseLine";
	sGame->addLineToIndex(line);
}

void KSquaresTestWindow::gameOver(const QVector<KSquaresPlayer> & playerList)
{
	kDebug() << "Game Over";
	kDebug() << "score hard ai: " << playerList[0].score();
	kDebug() << "score alphabeta ai: " << playerList[1].score();
	if (playerList[0].score() > playerList[1].score())
		results[0]++;
	else
		results[1]++;
	resultStr = "Hard: " + QString::number(results[0]) + ", AlphaBeta: " + QString::number(results[1]);
	QTimer::singleShot(1000, this, SLOT(gameNew()));
}

#include "ksquarestestwindow.moc"

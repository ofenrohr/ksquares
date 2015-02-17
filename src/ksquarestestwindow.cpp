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
	thread = NULL;

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
	
	outstandingChooseLineCalls = 0;
	firstSetup = true;
}

KSquaresTestWindow::~KSquaresTestWindow()
{
	delete sGame;
	if (thread != NULL)
		delete thread;
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
		playerList.append(KSquaresPlayer(i==0?"QDab":"AlphaBeta", color, false));
	}

	int width = 5;
	int height = 5;
	
	//create physical board
	GameBoardScene* temp = m_scene;
	m_scene = new GameBoardScene(width, height);

	m_view->setScene(m_scene);
	delete temp;

	m_view->setBoardSize();	//refresh board zooming

	// create AI players
	aiList.clear();
	aiController::Ptr aic0(new aiController(0, 1, width, height, 6, 10000));
	aiController::Ptr aic1(new aiController(1, 1, width, height, 3, 10000));
	aiList.append(aic0);
	aiList.append(aic1);
	
	//start game etc.
	sGame->createGame(playerList, width, height);
	if (firstSetup)
	{
		firstSetup = false;
		//connect(m_scene, SIGNAL(lineDrawn(int)), sGame, SLOT(addLineToIndex(int)));
		connect(sGame, SIGNAL(gameOver(QVector<KSquaresPlayer>)), this, SLOT(gameOver(QVector<KSquaresPlayer>)));
		connect(sGame, SIGNAL(takeTurnSig(KSquaresPlayer*)), this, SLOT(playerTakeTurn(KSquaresPlayer*)));
	}
	connect(sGame, SIGNAL(drawLine(int,QColor)), m_scene, SLOT(drawLine(int,QColor)));
	//connect(sGame, SIGNAL(highlightMove(int)), m_scene, SLOT(highlightLine(int)));
	connect(sGame, SIGNAL(drawSquare(int,QColor)), m_scene, SLOT(drawSquare(int,QColor)));

	sGame->start();
	
	playerTakeTurn(sGame->currentPlayer());
}

void KSquaresTestWindow::playerTakeTurn(KSquaresPlayer* currentPlayer)
{
	kDebug() << "playerTakeTurn";
	statusBar()->changeItem(currentPlayer->name(), 0);
	statusBar()->changeItem(resultStr, 1);
	outstandingChooseLineCalls++;
	kDebug() << "calling aiChooseLine";
	aiChooseLine();
}

void KSquaresTestWindow::aiChooseLine()
{
	outstandingChooseLineCalls--;
	kDebug() << "aiChooseLine (outstanding calls: " << outstandingChooseLineCalls << ")";
	if (!sGame->isRunning())
	{
		kDebug() << "ERROR: game not running, not choosing any line!";
		return;
	}
	
	if (thread != NULL) {
		//thread->quit();
		//kDebug() << "waiting for previous thread to exit, isFinished: " << thread->isFinished();
		if (!thread->isFinished())
		{
			//thread->quit();
			kDebug() << "rescheduling aiChooseLine";
			outstandingChooseLineCalls++;
			QTimer::singleShot(150, this, SLOT(aiChooseLine()));
			return;
		}
	} else {
	// https://mayaposch.wordpress.com/2011/11/01/how-to-really-truly-use-qthreads-the-full-explanation/
		thread = new QThread;
	}
	aiControllerWorker *worker = new aiControllerWorker(aiList[sGame->currentPlayerId()], sGame->board()->lines(), sGame->board()->squares(), sGame->board()->getLineHistory());
	worker->moveToThread(thread);
	//connect(worker, SIGNAL(error(QString)), this, SLOT(errorString(QString)));
	connect(thread, SIGNAL(started()), worker, SLOT(process()));
	connect(worker, SIGNAL(finished()), thread, SLOT(quit()));
	connect(worker, SIGNAL(finished()), worker, SLOT(deleteLater()));
	//connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
	connect(worker, SIGNAL(lineChosen(int)), this, SLOT(aiChoseLine(int)));
	thread->start();
	/*
	int line = aiList[sGame->currentPlayerId()]->chooseLine(sGame->board()->lines(), sGame->board()->squares(), sGame->board()->getLineHistory());
	sGame->addLineToIndex(line);
	*/
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

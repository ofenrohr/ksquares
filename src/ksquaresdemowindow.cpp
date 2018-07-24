/***************************************************************************
 *   Copyright (C) 2006 by Matthew Williams    <matt@milliams.com>         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

//header
#include "ksquaresdemowindow.h"

//qt
#include <QTimer>

//kde
#include <QApplication>
#include <QStatusBar>
#include <KActionCollection>
#include <QDebug>
#include <KLocalizedString>
#include <kstandardgameaction.h>

//generated
#include "settings.h"

//classes
#include "gameboardview.h"
#include "aiControllerWorker.h"

KSquaresDemoWindow::KSquaresDemoWindow() : KXmlGuiWindow(), m_view(new GameBoardView(this)), m_scene(0)
{
    sGame = new KSquaresGame();
    connect(sGame, &KSquaresGame::takeTurnSig, this, &KSquaresDemoWindow::playerTakeTurn);
    connect(sGame, &KSquaresGame::gameOver, this, &KSquaresDemoWindow::gameOver);

    m_view->setRenderHints(QPainter::Antialiasing);
    m_view->setFrameStyle(QFrame::NoFrame);
    m_view->setDisabled(true);
    setCentralWidget(m_view);

    KStandardGameAction::quit(qApp, SLOT(quit()), actionCollection());
    setupGUI();

    //QT5 statusBar()->insertPermanentItem(i18n("Current Player"), 0);
    statusBar()->show();
}

void KSquaresDemoWindow::gameNew()
{
	//create players
	QVector<KSquaresPlayer> playerList;
	for(int i=0; i<4; i++) {
		QColor color;
		switch(i) {
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
				qCritical() << "KSquaresGame::playerSquareComplete(); currentPlayerId() != 0|1|2|3";
		}
		playerList.append(KSquaresPlayer(i18n("Player %1", i+1), color, false));

		int aiLevel=0;
		switch(i)
		{
            // TODO: migrate new ai Settings
                /*
			case 0: aiLevel = Settings::playerOneAi(); break;
			case 1: aiLevel = Settings::playerTwoAi(); break;
			case 2: aiLevel = Settings::playerThreeAi(); break;
			case 3: aiLevel = Settings::playerFourAi(); break;
                 */
		}
		ais.append(aiController::Ptr(new aiController(i, 3, 15, 10, aiLevel)));

	}

    //create physical board
    GameBoardScene *temp = m_scene;
    m_scene = new GameBoardScene(15, 10, false);

    m_view->setScene(m_scene);
    delete temp;

    m_view->setBoardSize(); //refresh board zooming

    //start game etc.
    sGame->createGame(playerList, 15, 10);
    connect(m_scene, &GameBoardScene::lineDrawn, sGame, &KSquaresGame::addLineToIndex);
    connect(sGame, &KSquaresGame::drawLine, m_scene, &GameBoardScene::drawLine);
    connect(sGame, &KSquaresGame::highlightMove, m_scene, &GameBoardScene::highlightLine);
    connect(sGame, &KSquaresGame::drawSquare, m_scene, &GameBoardScene::drawSquare);

    sGame->start();
	playerTakeTurn(sGame->currentPlayer());
}

void KSquaresDemoWindow::playerTakeTurn(KSquaresPlayer *currentPlayer)
{
    //QT5 statusBar()->changeItem(currentPlayer->name(), 0);
    QTimer::singleShot(200, this, &KSquaresDemoWindow::aiChooseLine);
}

void KSquaresDemoWindow::aiChooseLine()
{
	//sGame->addLineToIndex(ai.chooseLine(sGame->board()->lines(), sGame->board()->squares()));

	// https://mayaposch.wordpress.com/2011/11/01/how-to-really-truly-use-qthreads-the-full-explanation/
	QThread* thread = new QThread;
	aiControllerWorker *worker = new aiControllerWorker(ais.at(sGame->currentPlayerId()), sGame->board()->lines(), sGame->board()->squares(), sGame->board()->getLineHistory());
	worker->moveToThread(thread);
	//connect(worker, SIGNAL(error(QString)), this, SLOT(errorString(QString)));
	connect(thread, SIGNAL(started()), worker, SLOT(process()));
	connect(worker, SIGNAL(finished()), thread, SLOT(quit()));
	connect(worker, SIGNAL(finished()), worker, SLOT(deleteLater()));
	connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
	connect(worker, SIGNAL(lineChosen(int)), this, SLOT(aiChoseLine(int)));
	thread->start();

    //aiController ai(sGame->currentPlayerId(), sGame->lines(), sGame->squares(), sGame->boardWidth(), sGame->boardHeight());
    //sGame->addLineToIndex(ai.chooseLine());
}

void KSquaresDemoWindow::aiChoseLine(const int &line)
{
    sGame->addLineToIndex(line);
}

void KSquaresDemoWindow::gameOver(const QVector<KSquaresPlayer> & /*playerList*/)
{
    //qDebug() << "Game Over";
    QTimer::singleShot(1000, this, &KSquaresDemoWindow::gameNew);
}


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

//qjson
#include <qjson/parser.h>
#include <qjson/serializer.h>

//generated
#include "settings.h"

//classes
#include "gameboardview.h"
#include "ksquaresio.h"

KSquaresTestWindow::KSquaresTestWindow() : KXmlGuiWindow(), m_view(new GameBoardView(this)), m_scene(0)
{
	initTestSetup();
	saveStatus();
	
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
	
	resultStr = "results";
	
	outstandingChooseLineCalls = 0;
	firstSetup = true;
}

KSquaresTestWindow::~KSquaresTestWindow()
{
	delete sGame;
	if (thread != NULL)
		delete thread;
}

void KSquaresTestWindow::saveStatus()
{
	QVariantList setupList;
	for (int i = 0; i < testSetups.size(); i++)
		setupList << testSetups[i].toQVariant();
	QVariantList resultList;
	for (int i = 0; i < testResults.size(); i++)
		resultList << testResults[i].toQVariant();
	QVariantMap statusMap;
	statusMap["setups"] = setupList;
	statusMap["results"] = resultList;
	
	QJson::Serializer serializer;
	bool ok;
	QByteArray json = serializer.serialize(setupList, &ok);

	if (ok) {
		kDebug() << "Setup as json: " << json;
	} else {
		kDebug() << "Something went wrong:" << serializer.errorMessage();
	}

}

QVariant AITestSetup::toQVariant()
{
	QVariantMap map;
	map["levelP1"] = levelP1;
	map["levelP2"] = levelP2;
	map["timeout"] = timeout;
	QVariantMap boardSizeMap;
	boardSizeMap["width"] = boardSize.x();
	boardSizeMap["height"] = boardSize.y();
	map["boardSize"] = boardSizeMap;
	return map;
}

void AITestSetup::fromQVariant(QVariant var)
{
	QVariantMap map = var.toMap();
	levelP1 = map["levelP1"].toInt();
	levelP2 = map["levelP2"].toInt();
	timeout = map["timeout"].toInt();
	boardSize = QPoint();
	boardSize.setX(map["boardSize"].toMap()["width"].toInt());
	boardSize.setY(map["boardSize"].toMap()["height"].toInt());
}

QVariant AITestResult::toQVariant()
{
	QVariantMap map;
	map["setup"] = setup.toQVariant();
	QVariantList moveList;
	for (int i = 0; i < moves.size(); i++)
		moveList << moves[i];
	map["moves"] = moveList;
	QVariantList timeP1List;
	for (int i = 0; i < timeP1.size(); i++)
		timeP1List << timeP1[i];
	map["timeP1"] = timeP1List;
	QVariantList timeP2List;
	for (int i = 0; i < timeP2.size(); i++)
		timeP2List << timeP2[i];
	map["timeP2"] = timeP2List;
	map["taintedP1"] = taintedP1;
	map["taintedP2"] = taintedP2;
	map["scoreP1"] = scoreP1;
	map["scoreP2"] = scoreP2;
	return map;
}

void AITestResult::fromQVariant(QVariant map)
{
	
}

void KSquaresTestWindow::initTestSetup()
{
	testSetups.clear();
	
	for (int i = 0; i < 5; i++)
	{
		AITestSetup setup;
		setup.levelP1 = KSquares::AI_EASY;
		setup.levelP2 = KSquares::AI_MEDIUM;
		setup.timeout = 5000;
		setup.boardSize = QPoint(5,5);
		testSetups.append(setup);
	}
	for (int i = 0; i < 5; i++)
	{
		AITestSetup setup;
		setup.levelP1 = KSquares::AI_MEDIUM;
		setup.levelP2 = KSquares::AI_EASY;
		setup.timeout = 5000;
		setup.boardSize = QPoint(5,5);
		testSetups.append(setup);
	}
	for (int i = 0; i < 5; i++)
	{
		AITestSetup setup;
		setup.levelP1 = KSquares::AI_MEDIUM;
		setup.levelP2 = KSquares::AI_HARD;
		setup.timeout = 5000;
		setup.boardSize = QPoint(5,5);
		testSetups.append(setup);
	}
	for (int i = 0; i < 5; i++)
	{
		AITestSetup setup;
		setup.levelP1 = KSquares::AI_HARD;
		setup.levelP2 = KSquares::AI_MEDIUM;
		setup.timeout = 5000;
		setup.boardSize = QPoint(5,5);
		testSetups.append(setup);
	}
	for (int i = 0; i < 5; i++)
	{
		AITestSetup setup;
		setup.levelP1 = KSquares::AI_HARD;
		setup.levelP2 = KSquares::AI_VERYHARD;
		setup.timeout = 5000;
		setup.boardSize = QPoint(5,5);
		testSetups.append(setup);
	}
	for (int i = 0; i < 5; i++)
	{
		AITestSetup setup;
		setup.levelP1 = KSquares::AI_VERYHARD;
		setup.levelP2 = KSquares::AI_HARD;
		setup.timeout = 5000;
		setup.boardSize = QPoint(5,5);
		testSetups.append(setup);
	}
	
	// external ai
	QList<KSquares::AILevel> externalAIs;
	externalAIs << KSquares::AI_DABBLE << KSquares::AI_QDAB << KSquares::AI_KNOX;
	for (int e = 0; e < externalAIs.size(); e++)
	{
		for (int i = 0; i < 10; i++)
		{
			AITestSetup setup;
			setup.levelP1 = KSquares::AI_VERYHARD;
			setup.levelP2 = externalAIs[e];
			setup.timeout = 5000;
			setup.boardSize = QPoint(5,5);
			testSetups.append(setup);
		}
		for (int i = 0; i < 10; i++)
		{
			AITestSetup setup;
			setup.levelP1 = externalAIs[e];
			setup.levelP2 = KSquares::AI_VERYHARD;
			setup.timeout = 5000;
			setup.boardSize = QPoint(5,5);
			testSetups.append(setup);
		}
	}
	
	/*
	QList<int> timeouts;
	timeouts << 5 << 10 << 30;
	
	QList<KSquares::AILevel> aiLevelList;
	aiLevelList << KSquares::AI_EASY << KSquares::AI_MEDIUM << KSquares::AI_HARD << KSquares::AI_VERYHARD << KSquares::AI_DABBLE << KSquares::AI_QDAB << KSquares::AI_KNOX;
	
	QList<QPoint> boardSizes;
	boardSizes << QPoint(5,5);
	
	long maxRequiredTime = 0;
	
	for (int t = 0; t < timeouts.size(); t++)
	{	
		for (int a1 = 0; a1 < aiLevelList.size(); a1++)
		{
			for (int a2 = 0; a2 < aiLevelList.size(); a2++)
			{
				if (a1 == a2)
					continue;
				for (int b = 0; b < boardSizes.size(); b++)
				{
					for (int i = 0; i < 20 - t*5; i++)
					{
						AITestSetup setup;
						setup.levelP1 = aiLevelList[a1];
						setup.levelP2 = aiLevelList[a2];
						setup.timeout = timeouts[t] * 1000;
						setup.boardSize = boardSizes[b];
						testSetups.append(setup);
						
						maxRequiredTime += timeouts[t] * aiFunctions::toLinesSize(boardSizes[b].x(), boardSizes[b].y());
					}
				}
			}
		}
	}
	
	kDebug() << "generated " << testSetups.size() << " test setups";
	kDebug() << "maximum required time: " << maxRequiredTime;
	*/
}

void KSquaresTestWindow::gameNew()
{
	if (testSetups.size() <= 0)
	{
		kDebug() << "no more testSetups, exit application";
		QCoreApplication::quit();
	}
	currentSetup = testSetups.takeFirst();
	currentResult = AITestResult();
	
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

	int width = currentSetup.boardSize.x();
	int height = currentSetup.boardSize.y();
	
	//create physical board
	GameBoardScene* temp = m_scene;
	m_scene = new GameBoardScene(width, height);

	m_view->setScene(m_scene);
	delete temp;

	m_view->setBoardSize();	//refresh board zooming

	// create AI players
	aiList.clear();
	aiController::Ptr aic0(new aiController(0, 1, width, height, currentSetup.levelP1, currentSetup.timeout));
	aiController::Ptr aic1(new aiController(1, 1, width, height, currentSetup.levelP2, currentSetup.timeout));
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
	
	if (aiList[sGame->currentPlayerId()]->getAi()->tainted())
	{
		kDebug() << "ERROR: game is tainted! aborting";
		gameOver(sGame->getPlayers());
		return;
	}
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
	currentResult.moves.append(line);
	
}

void KSquaresTestWindow::gameOver(const QVector<KSquaresPlayer> & playerList)
{
	kDebug() << "Game Over";
	kDebug() << "score p1 ai: " << playerList[0].score();
	kDebug() << "score p2 ai: " << playerList[1].score();
	resultStr = "Remaining games: " + QString::number(testSetups.size());
	
	currentResult.setup = currentSetup;
	currentResult.taintedP1 = aiList[0].getAi().tainted();
	currentResult.taintedP1 = aiList[1].getAi().tainted();
	currentResult.scoreP1 = playerList[0].score();
	currentResult.scoreP2 = playerList[1].score();
	
	QTimer::singleShot(1000, this, SLOT(gameNew()));
}

#include "ksquarestestwindow.moc"

/***************************************************************************
 *   Copyright (C) 2015 by Tom Vincent Peters   <kde@vincent-peters.de>    *
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
#include <QFile>
#include <QTextStream>
#include <QJsonDocument>
#include <QDebug>

//kde
//#include <KApplication>
//#include <KStatusBar>
#include <KActionCollection>
//#include <KLocale>
#include <kstandardgameaction.h>

//qjson
//#include <qjson/parser.h>
//#include <qjson/serializer.h>
#include <klocalizedstring.h>
#include <sstream>

//generated
#include "settings.h"

//classes
#include "gameboardview.h"
#include "ksquaresio.h"
#include "aiControllerWorker.h"

KSquaresTestWindow::KSquaresTestWindow(bool doFullTest) : KXmlGuiWindow(), m_view(new GameBoardView(this)), m_scene(0)
{
	testSetups = QList<AITestSetup>();
	testResults = QList<AITestResult>();

	fullTest = doFullTest;
	
	initTest();
	
	sGame = new KSquaresGame();
	thread = NULL;

	m_view->setRenderHints(QPainter::Antialiasing);
	m_view->setFrameStyle(QFrame::NoFrame);
	m_view->setDisabled(true);
	setCentralWidget(m_view);

	KStandardGameAction::quit(qApp, SLOT(quit()), actionCollection());
	setupGUI();

	//statusBar()->addPermanentWidget(m_player);
	//statusBar()->insertPermanentItem(i18n("Results"), 1);
	//statusBar()->show();
	
	updateResultStr();
	
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
	statusMap[QStringLiteral("setups")] = setupList;
	statusMap[QStringLiteral("results")] = resultList;

	/*
	QJson::Serializer serializer;
	bool ok;
	QByteArray json = serializer.serialize(statusMap, &ok);

	if (ok) {
		qDebug() << "Setup as json: " << json;
	} else {
		qDebug() << "Something went wrong:" << serializer.errorMessage();
		return;
	}
	 */
	QJsonDocument statusMapJSon = QJsonDocument::fromVariant(statusMap);
	QByteArray json = statusMapJSon.toJson();

	QFile file(QStringLiteral("ksquares-test-status.json"));
	if (!file.open(QIODevice::ReadWrite | QIODevice::Truncate))
	{
		qDebug() << "KSquaresTest error: Can't open file";
		return;
	}
	
	QTextStream outStream(&file);
	outStream << json;
	file.close();
}

bool KSquaresTestWindow::loadStatus()
{
	QFile file(QStringLiteral("ksquares-test-status.json"));
	if (!file.open(QIODevice::ReadOnly))
	{
		qDebug() << "No previous test status file found";
		return false;
	}

	QJsonParseError jsonParseError;
	QJsonDocument mapJSon = QJsonDocument::fromJson(file.readAll(), &jsonParseError);

	if (jsonParseError.error != QJsonParseError::NoError)
	{
		qDebug() << "parsing failed! json error: " << jsonParseError.errorString();
		return false;
	}
	QVariantMap map = mapJSon.toVariant().toMap();

	/*
	QJson::Parser jsonParser;
	bool parseOk;
	QVariantMap map = jsonParser.parse(&file, &parseOk).toMap();
	
	if (!parseOk)
	{
		qDebug() << "parsing failed! json error: " << jsonParser.errorString();
		return false;
	}
	*/
	
	QVariantList setups = map[QStringLiteral("setups")].toList();
	testSetups.clear();
	for (int i = 0; i < setups.size(); i++)
	{
		AITestSetup setup;
		setup.fromQVariant(setups[i]);
		testSetups.append(setup);
	}
	QVariantList results = map[QStringLiteral("results")].toList();
	testResults.clear();
	for (int i = 0; i < results.size(); i++)
	{
		AITestResult result;
		result.fromQVariant(results[i]);
		testResults.append(result);
	}
	return true;
}


void KSquaresTestWindow::initTest()
{
	testSetups.clear();
	testResults.clear();

	if (!fullTest && loadStatus())
        return;


    for (int i = 0; i < 5; i++)
	{
		AITestSetup setup;
		setup.aiLevelP1 = KSquares::AI_EASY;
		setup.aiLevelP2 = KSquares::AI_CONVNET;
		setup.timeout = 30000;
		setup.boardSize = QPoint(5,5);
		testSetups.append(setup);
	}

    for (int i = 0; i < 5; i++)
	{
		AITestSetup setup;
		setup.aiLevelP1 = KSquares::AI_CONVNET;
		setup.aiLevelP2 = KSquares::AI_EASY;
		setup.timeout = 30000;
		setup.boardSize = QPoint(5,5);
		testSetups.append(setup);
	}

	return;


	for (int i = 0; i < 5; i++)
	{
		AITestSetup setup;
		setup.aiLevelP1 = KSquares::AI_VERYHARD;
		setup.aiLevelP2 = KSquares::AI_HARD;
		setup.timeout = 30000;
		setup.boardSize = QPoint(7,7);
		testSetups.append(setup);
	}
	for (int i = 0; i < 5; i++)
	{
		AITestSetup setup;
		setup.aiLevelP1 = KSquares::AI_HARD;
		setup.aiLevelP2 = KSquares::AI_VERYHARD;
		setup.timeout = 30000;
		setup.boardSize = QPoint(7,7);
		testSetups.append(setup);
	}
	//return;
	for (int i = 0; i < 10; i++)
	{
		AITestSetup setup;
		setup.aiLevelP1 = KSquares::AI_MCTS_B;
		setup.aiLevelP2 = KSquares::AI_HARD;
		setup.timeout = 30000;
		setup.boardSize = QPoint(5,5);
		testSetups.append(setup);
	}
	for (int i = 0; i < 10; i++)
	{
		AITestSetup setup;
		setup.aiLevelP1 = KSquares::AI_HARD;
		setup.aiLevelP2 = KSquares::AI_MCTS_B;
		setup.timeout = 30000;
		setup.boardSize = QPoint(5,5);
		testSetups.append(setup);
	}
	//return;
	for (int i = 0; i < 10; i++)
	{
		AITestSetup setup;
		setup.aiLevelP1 = KSquares::AI_DABBLE;
		setup.aiLevelP2 = KSquares::AI_HARD;
		setup.timeout = 30000;
		setup.boardSize = QPoint(5,5);
		testSetups.append(setup);
	}
	for (int i = 0; i < 10; i++)
	{
		AITestSetup setup;
		setup.aiLevelP1 = KSquares::AI_HARD;
		setup.aiLevelP2 = KSquares::AI_DABBLE;
		setup.timeout = 30000;
		setup.boardSize = QPoint(5,5);
		testSetups.append(setup);
	}
	//return;
	for (int i = 0; i < 10; i++)
	{
		AITestSetup setup;
		setup.aiLevelP1 = KSquares::AI_HARD;
		setup.aiLevelP2 = KSquares::AI_VERYHARD;
		setup.timeout = 30000;
		setup.boardSize = QPoint(5,5);
		testSetups.append(setup);
	}
	for (int i = 0; i < 10; i++)
	{
		AITestSetup setup;
		setup.aiLevelP1 = KSquares::AI_VERYHARD;
		setup.aiLevelP2 = KSquares::AI_HARD;
		setup.timeout = 30000;
		setup.boardSize = QPoint(5,5);
		testSetups.append(setup);
	}
	//return;
	for (int i = 0; i < 5; i++)
	{
		AITestSetup setup;
		setup.aiLevelP1 = KSquares::AI_DABBLENOHASH;
		setup.aiLevelP2 = KSquares::AI_MCTS_B;
		setup.timeout = 30000;
		setup.boardSize = QPoint(6,6);
		testSetups.append(setup);
	}
	for (int i = 0; i < 5; i++)
	{
		AITestSetup setup;
		setup.aiLevelP1 = KSquares::AI_MCTS_B;
		setup.aiLevelP2 = KSquares::AI_DABBLENOHASH;
		setup.timeout = 30000;
		setup.boardSize = QPoint(6,6);
		testSetups.append(setup);
	}
	for (int i = 0; i < 5; i++)
	{
		AITestSetup setup;
		setup.aiLevelP1 = KSquares::AI_DABBLENOHASH;
		setup.aiLevelP2 = KSquares::AI_MCTS_B;
		setup.timeout = 30000;
		setup.boardSize = QPoint(7,7);
		testSetups.append(setup);
	}
	for (int i = 0; i < 5; i++)
	{
		AITestSetup setup;
		setup.aiLevelP1 = KSquares::AI_MCTS_B;
		setup.aiLevelP2 = KSquares::AI_DABBLENOHASH;
		setup.timeout = 30000;
		setup.boardSize = QPoint(7,7);
		testSetups.append(setup);
	}
	
	for (int i = 0; i < 5; i++)
	{
		AITestSetup setup;
		setup.aiLevelP1 = KSquares::AI_VERYHARD;
		setup.aiLevelP2 = KSquares::AI_MCTS_B;
		setup.timeout = 30000;
		setup.boardSize = QPoint(6,6);
		testSetups.append(setup);
	}
	for (int i = 0; i < 5; i++)
	{
		AITestSetup setup;
		setup.aiLevelP1 = KSquares::AI_MCTS_B;
		setup.aiLevelP2 = KSquares::AI_VERYHARD;
		setup.timeout = 30000;
		setup.boardSize = QPoint(6,6);
		testSetups.append(setup);
	}
	for (int i = 0; i < 5; i++)
	{
		AITestSetup setup;
		setup.aiLevelP1 = KSquares::AI_VERYHARD;
		setup.aiLevelP2 = KSquares::AI_MCTS_B;
		setup.timeout = 30000;
		setup.boardSize = QPoint(7,7);
		testSetups.append(setup);
	}
	for (int i = 0; i < 5; i++)
	{
		AITestSetup setup;
		setup.aiLevelP1 = KSquares::AI_MCTS_B;
		setup.aiLevelP2 = KSquares::AI_VERYHARD;
		setup.timeout = 30000;
		setup.boardSize = QPoint(7,7);
		testSetups.append(setup);
	}
	
	for (int i = 0; i < 5; i++)
	{
		AITestSetup setup;
		setup.aiLevelP1 = KSquares::AI_DABBLENOHASH;
		setup.aiLevelP2 = KSquares::AI_HARD;
		setup.timeout = 30000;
		setup.boardSize = QPoint(6,6);
		testSetups.append(setup);
	}
	for (int i = 0; i < 5; i++)
	{
		AITestSetup setup;
		setup.aiLevelP1 = KSquares::AI_HARD;
		setup.aiLevelP2 = KSquares::AI_DABBLENOHASH;
		setup.timeout = 30000;
		setup.boardSize = QPoint(6,6);
		testSetups.append(setup);
	}
	for (int i = 0; i < 5; i++)
	{
		AITestSetup setup;
		setup.aiLevelP1 = KSquares::AI_DABBLENOHASH;
		setup.aiLevelP2 = KSquares::AI_HARD;
		setup.timeout = 30000;
		setup.boardSize = QPoint(7,7);
		testSetups.append(setup);
	}
	for (int i = 0; i < 5; i++)
	{
		AITestSetup setup;
		setup.aiLevelP1 = KSquares::AI_HARD;
		setup.aiLevelP2 = KSquares::AI_DABBLENOHASH;
		setup.timeout = 30000;
		setup.boardSize = QPoint(7,7);
		testSetups.append(setup);
	}
	//return;
	for (int i = 0; i < 5; i++)
	{
		AITestSetup setup;
		setup.aiLevelP1 = KSquares::AI_DABBLENOHASH;
		setup.aiLevelP2 = KSquares::AI_VERYHARD;
		setup.timeout = 30000;
		setup.boardSize = QPoint(6,6);
		testSetups.append(setup);
	}
	for (int i = 0; i < 5; i++)
	{
		AITestSetup setup;
		setup.aiLevelP1 = KSquares::AI_VERYHARD;
		setup.aiLevelP2 = KSquares::AI_DABBLENOHASH;
		setup.timeout = 30000;
		setup.boardSize = QPoint(6,6);
		testSetups.append(setup);
	}
	for (int i = 0; i < 5; i++)
	{
		AITestSetup setup;
		setup.aiLevelP1 = KSquares::AI_HARD;
		setup.aiLevelP2 = KSquares::AI_VERYHARD;
		setup.timeout = 30000;
		setup.boardSize = QPoint(7,7);
		testSetups.append(setup);
	}
	for (int i = 0; i < 5; i++)
	{
		AITestSetup setup;
		setup.aiLevelP1 = KSquares::AI_VERYHARD;
		setup.aiLevelP2 = KSquares::AI_HARD;
		setup.timeout = 30000;
		setup.boardSize = QPoint(7,7);
		testSetups.append(setup);
	}
	for (int i = 0; i < 5; i++)
	{
		AITestSetup setup;
		setup.aiLevelP1 = KSquares::AI_DABBLENOHASH;
		setup.aiLevelP2 = KSquares::AI_VERYHARD;
		setup.timeout = 30000;
		setup.boardSize = QPoint(7,7);
		testSetups.append(setup);
	}
	for (int i = 0; i < 5; i++)
	{
		AITestSetup setup;
		setup.aiLevelP1 = KSquares::AI_VERYHARD;
		setup.aiLevelP2 = KSquares::AI_DABBLENOHASH;
		setup.timeout = 30000;
		setup.boardSize = QPoint(7,7);
		testSetups.append(setup);
	}
	
	//return;
	
	for (int i = 0; i < 10; i++)
	{
		AITestSetup setup;
		setup.aiLevelP1 = KSquares::AI_VERYHARD;
		setup.aiLevelP2 = KSquares::AI_DABBLE;
		setup.timeout = 30000;
		setup.boardSize = QPoint(5,5);
		testSetups.append(setup);
	}
	for (int i = 0; i < 10; i++)
	{
		AITestSetup setup;
		setup.aiLevelP1 = KSquares::AI_DABBLE;
		setup.aiLevelP2 = KSquares::AI_VERYHARD;
		setup.timeout = 30000;
		setup.boardSize = QPoint(5,5);
		testSetups.append(setup);
	}
	for (int i = 0; i < 10; i++)
	{
		AITestSetup setup;
		setup.aiLevelP1 = KSquares::AI_QDAB;
		setup.aiLevelP2 = KSquares::AI_VERYHARD;
		setup.timeout = 30000;
		setup.boardSize = QPoint(5,5);
		testSetups.append(setup);
	}
	for (int i = 0; i < 10; i++)
	{
		AITestSetup setup;
		setup.aiLevelP1 = KSquares::AI_MCTS_A;
		setup.aiLevelP2 = KSquares::AI_MCTS_B;
		setup.timeout = 5000;
		setup.boardSize = QPoint(5,5);
		testSetups.append(setup);
	}
	for (int i = 0; i < 10; i++)
	{
		AITestSetup setup;
		setup.aiLevelP1 = KSquares::AI_MCTS_B;
		setup.aiLevelP2 = KSquares::AI_MCTS_A;
		setup.timeout = 5000;
		setup.boardSize = QPoint(5,5);
		testSetups.append(setup);
	}
	for (int i = 0; i < 10; i++)
	{
		AITestSetup setup;
		setup.aiLevelP1 = KSquares::AI_MCTS_A;
		setup.aiLevelP2 = KSquares::AI_MCTS_C;
		setup.timeout = 5000;
		setup.boardSize = QPoint(5,5);
		testSetups.append(setup);
	}
	for (int i = 0; i < 10; i++)
	{
		AITestSetup setup;
		setup.aiLevelP1 = KSquares::AI_MCTS_C;
		setup.aiLevelP2 = KSquares::AI_MCTS_A;
		setup.timeout = 5000;
		setup.boardSize = QPoint(5,5);
		testSetups.append(setup);
	}
	for (int i = 0; i < 10; i++)
	{
		AITestSetup setup;
		setup.aiLevelP1 = KSquares::AI_MCTS_B;
		setup.aiLevelP2 = KSquares::AI_MCTS_C;
		setup.timeout = 5000;
		setup.boardSize = QPoint(5,5);
		testSetups.append(setup);
	}
	for (int i = 0; i < 10; i++)
	{
		AITestSetup setup;
		setup.aiLevelP1 = KSquares::AI_MCTS_C;
		setup.aiLevelP2 = KSquares::AI_MCTS_B;
		setup.timeout = 5000;
		setup.boardSize = QPoint(5,5);
		testSetups.append(setup);
	}
	//return;
	
	for (int i = 0; i < 10; i++)
	{
		AITestSetup setup;
		setup.aiLevelP1 = KSquares::AI_VERYHARD;
		setup.aiLevelP2 = KSquares::AI_QDAB;
		setup.timeout = 30000;
		setup.boardSize = QPoint(5,5);
		testSetups.append(setup);
	}
	
	
	for (int i = 0; i < 10; i++)
	{
		AITestSetup setup;
		setup.aiLevelP1 = KSquares::AI_QDAB;
		setup.aiLevelP2 = KSquares::AI_HARD;
		setup.timeout = 5000;
		setup.boardSize = QPoint(5,5);
		testSetups.append(setup);
	}
	for (int i = 0; i < 10; i++)
	{
		AITestSetup setup;
		setup.aiLevelP1 = KSquares::AI_HARD;
		setup.aiLevelP2 = KSquares::AI_QDAB;
		setup.timeout = 5000;
		setup.boardSize = QPoint(5,5);
		testSetups.append(setup);
	}
	
	for (int i = 0; i < 10; i++)
	{
		AITestSetup setup;
		setup.aiLevelP1 = KSquares::AI_DABBLE;
		setup.aiLevelP2 = KSquares::AI_HARD;
		setup.timeout = 5000;
		setup.boardSize = QPoint(5,5);
		testSetups.append(setup);
	}
	for (int i = 0; i < 10; i++)
	{
		AITestSetup setup;
		setup.aiLevelP1 = KSquares::AI_HARD;
		setup.aiLevelP2 = KSquares::AI_DABBLE;
		setup.timeout = 5000;
		setup.boardSize = QPoint(5,5);
		testSetups.append(setup);
	}
	
	
	for (int i = 0; i < 10; i++)
	{
		AITestSetup setup;
		setup.aiLevelP1 = KSquares::AI_HARD;
		setup.aiLevelP2 = KSquares::AI_VERYHARD;
		setup.timeout = 30000;
		setup.boardSize = QPoint(6,6);
		testSetups.append(setup);
	}
	for (int i = 0; i < 10; i++)
	{
		AITestSetup setup;
		setup.aiLevelP1 = KSquares::AI_VERYHARD;
		setup.aiLevelP2 = KSquares::AI_HARD;
		setup.timeout = 30000;
		setup.boardSize = QPoint(6,6);
		testSetups.append(setup);
	}
	
	//return;
	
	for (int i = 0; i < 5; i++)
	{
		AITestSetup setup;
		setup.aiLevelP1 = KSquares::AI_EASY;
		setup.aiLevelP2 = KSquares::AI_MEDIUM;
		setup.timeout = 5000;
		setup.boardSize = QPoint(5,5);
		testSetups.append(setup);
	}
	for (int i = 0; i < 5; i++)
	{
		AITestSetup setup;
		setup.aiLevelP1 = KSquares::AI_MEDIUM;
		setup.aiLevelP2 = KSquares::AI_EASY;
		setup.timeout = 5000;
		setup.boardSize = QPoint(5,5);
		testSetups.append(setup);
	}
	for (int i = 0; i < 5; i++)
	{
		AITestSetup setup;
		setup.aiLevelP1 = KSquares::AI_MEDIUM;
		setup.aiLevelP2 = KSquares::AI_HARD;
		setup.timeout = 5000;
		setup.boardSize = QPoint(5,5);
		testSetups.append(setup);
	}
	for (int i = 0; i < 5; i++)
	{
		AITestSetup setup;
		setup.aiLevelP1 = KSquares::AI_HARD;
		setup.aiLevelP2 = KSquares::AI_MEDIUM;
		setup.timeout = 5000;
		setup.boardSize = QPoint(5,5);
		testSetups.append(setup);
	}
	for (int i = 0; i < 5; i++)
	{
		AITestSetup setup;
		setup.aiLevelP1 = KSquares::AI_HARD;
		setup.aiLevelP2 = KSquares::AI_VERYHARD;
		setup.timeout = 5000;
		setup.boardSize = QPoint(5,5);
		testSetups.append(setup);
	}
	for (int i = 0; i < 5; i++)
	{
		AITestSetup setup;
		setup.aiLevelP1 = KSquares::AI_VERYHARD;
		setup.aiLevelP2 = KSquares::AI_HARD;
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
			setup.aiLevelP1 = KSquares::AI_VERYHARD;
			setup.aiLevelP2 = externalAIs[e];
			setup.timeout = 5000;
			setup.boardSize = QPoint(5,5);
			testSetups.append(setup);
		}
		for (int i = 0; i < 10; i++)
		{
			AITestSetup setup;
			setup.aiLevelP1 = externalAIs[e];
			setup.aiLevelP2 = KSquares::AI_VERYHARD;
			setup.timeout = 5000;
			setup.boardSize = QPoint(5,5);
			testSetups.append(setup);
		}
	}
}

QPair<int, int> orderedLevels(int a, int b)
{
	int l1 = a;
	int l2 = b;
	if (l2 < l1)
	{
		l1 = b;
		l2 = a;
	}
	return QPair<int, int>(l1, l2);
}

QString setupType(AITestSetup setup)
{
	QPair<int, int> lvls = orderedLevels(setup.aiLevelP1, setup.aiLevelP2);
	return QString::number(lvls.first) +
			QStringLiteral("-") +
			QString::number(lvls.second) +
			QStringLiteral(":") +
			QString::number(setup.boardSize.x()) +
			QStringLiteral("x") +
			QString::number(setup.boardSize.y()) +
			QStringLiteral("@") +
			QString::number(setup.timeout) +
			QStringLiteral("ms");
}

QString KSquaresTestWindow::latexResultGroup(QList<AITestResult> group)
{
	if (group.size() <= 0)
		return QString();
	
	QPair<int, int> lvls = orderedLevels(group[0].setup.aiLevelP1, group[0].setup.aiLevelP2);
	QPair<int, int> winsP1(0,0); // winsP1.second = number of times Player2 (aiLevelP2) won when going first
	QPair<int, int> winsP2(0,0); // winsP2.first = number of times Player1 (aiLevelP1) won when going second
	QPair<int, int> losesP1(0,0);
	QPair<int, int> losesP2(0,0);
	QPair<int, int> tainted(0,0);
	QPair<unsigned long long, unsigned long long> avgTime(0,0);
	QPair<unsigned long long, unsigned long long> allTime(0,0);
	QPair<unsigned long, unsigned long> timeDiv(0,0);
	
	//QString tmp = "";
	for (int i = 0; i < group.size(); i++)
	{
		AITestResult res = group[i];
		//tmp += "% setup.aiLevelP1 = " + QString::number(res.setup.aiLevelP1) + ", setup.aiLevelP2 = " + QString::number(res.setup.aiLevelP2);
		//tmp += " setup.scoreP1 = " + QString::number(res.scoreP1) + ", setup.scoreP2 = " + QString::number(res.scoreP2) + "\n";
		if (res.setup.aiLevelP1 == lvls.first) // Player 1 is going first (player 1 is res.*P1)
		{
			if (res.taintedP1)
				tainted.first++;
			if (res.taintedP2)
				tainted.second++;
			tainted.first += res.crashesP1;
			tainted.second += res.crashesP2;
			
			if (res.scoreP1 > res.scoreP2)
			{
				winsP1.first++;
				losesP2.second++;
			}
			if (res.scoreP1 < res.scoreP2)
			{
				winsP2.second++;
				losesP1.first++;
			}
			
			for (int i = 0; i < res.timeP1.size(); i++)
			{
				allTime.first += res.timeP1[i];
			}
			timeDiv.first += res.timeP1.size();
			for (int i = 0; i < res.timeP2.size(); i++)
			{
				allTime.second += res.timeP2[i];
			}
			timeDiv.second += res.timeP2.size();
		}
		else // Player 1 is going second (player 1 is res.*P2)
		{
			if (res.taintedP1)
				tainted.second++;
			if (res.taintedP2)
				tainted.first++;
			tainted.second += res.crashesP1;
			tainted.first += res.crashesP2;
			
			if (res.scoreP1 > res.scoreP2)
			{
				winsP1.second++;
				losesP2.first++;
			}
			if (res.scoreP1 < res.scoreP2)
			{
				winsP2.first++;
				losesP1.second++;
			}
			
			for (int i = 0; i < res.timeP1.size(); i++)
			{
				allTime.second += res.timeP1[i];
			}
			timeDiv.second += res.timeP1.size();
			for (int i = 0; i < res.timeP2.size(); i++)
			{
				allTime.first += res.timeP2[i];
			}
			timeDiv.first += res.timeP2.size();
		}
	}
	if (timeDiv.first != 0)
		avgTime.first = allTime.first / timeDiv.first;
	if (timeDiv.second != 0)
		avgTime.second = allTime.second / timeDiv.second;

	QString strret;
	QTextStream ret(&strret);
	ret << "\\begin{table}[H]\n";
	ret << "  \\begin{tabular}{c|cccc}\n";
	ret << "    \\specialcell{Spieler 1\\\\Spieler 2} & \\specialcell{Gewonnen \\\\(Sp. 1 / Sp. 2)} & Spiele & Crashes & $\\emptyset$ Zugzeit (ms) \\\\ \\hline\n";
	ret << "    " << aiFunctions::prettyAiLevel(lvls.first) << " & $" << QString::number(winsP1.first + winsP2.first) << "$ ($" << QString::number(winsP1.first) << "$ / $" << QString::number(winsP2.first) << "$) &  $" << QString::number(group.size()) << "$ & $" << QString::number(tainted.first) << "$ & $" << QString::number(avgTime.first) << "$ \\\\\n";
	ret << "    " << aiFunctions::prettyAiLevel(lvls.second) << " & $" << QString::number(winsP1.second + winsP2.second) << "$ ($" << QString::number(winsP1.second) << "$ / $" << QString::number(winsP2.second) << "$) & $" << QString::number(group.size()) << "$ & $" << QString::number(tainted.second) << "$ & $" << QString::number(avgTime.second) << "$ \\\\\n";
	ret << "  \\end{tabular}\n";
	ret << "  \\caption{Ergebnisse von " << aiFunctions::prettyAiLevel(lvls.first) << " gegen " << aiFunctions::prettyAiLevel(lvls.second) << " auf $" << QString::number(group[0].setup.boardSize.x()) << " \\times " << QString::number(group[0].setup.boardSize.y()) << "$ Spielfeld bei maximal $" << QString::number(group[0].setup.timeout/1000) << "$ Sekunden Bedenkzeit}\n";
	ret << "\\end{table}\n";

	return strret;
}

void KSquaresTestWindow::generateLatexResults()
{
	QMap<QString, QList<AITestResult> > resultGroupsMap;
	for (int i = 0; i < testResults.size(); i++)
	{
		resultGroupsMap[setupType(testResults[i].setup)].append(testResults[i]);
	}
	QList< QList<AITestResult> > resultGroups = resultGroupsMap.values();
	qDebug() << resultGroups.size() << " result groups.";
	
	QString tex = QStringLiteral("\n\n\\newcommand{\\specialcell}[2][c]{%\n");
    tex += QStringLiteral("\\begin{tabular}[#1]{@{}c@{}}#2\\end{tabular}}\n\n");
	for (int i = 0; i < resultGroups.size(); i++)
	{
		tex += latexResultGroup(resultGroups[i]) + QStringLiteral("\n");
	}
	qDebug().noquote().nospace() << tex;
}

void KSquaresTestWindow::gameNew()
{
	generateLatexResults();
	// load test setup
	if (testSetups.size() <= 0)
	{
		qDebug() << "no more testSetups, exit application";
		generateLatexResults();
		QCoreApplication::quit();
		exit(0);
		return;
	}
	
	currentSetup = testSetups.takeFirst();
	currentResult = AITestResult();

	int width = currentSetup.boardSize.x();
	int height = currentSetup.boardSize.y();
	
	// create AI players
	aiList.clear();
	aiController::Ptr aic0(new aiController(0, 1, width, height, currentSetup.aiLevelP1, currentSetup.timeout));
	aiController::Ptr aic1(new aiController(1, 1, width, height, currentSetup.aiLevelP2, currentSetup.timeout));
	aiList.append(aic0);
	aiList.append(aic1);
	
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
				qCritical() << "KSquaresGame::playerSquareComplete(); currentPlayerId() != 0|1|2|3";
		}
		playerList.append(KSquaresPlayer(aiList[i]->getAi()->getName(), color, false));
	}
	
	//create physical board
	GameBoardScene* temp = m_scene;
	m_scene = new GameBoardScene(width, height, true);

	m_view->setScene(m_scene);
	delete temp;

	m_view->setBoardSize();	//refresh board zooming
	
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
	qDebug() << "playerTakeTurn";
	//statusBar()->changeItem(currentPlayer->name(), 0);
	//statusBar()->changeItem(resultStr, 1);
	outstandingChooseLineCalls++;
	qDebug() << "calling aiChooseLine";
	aiChooseLine();
}

void KSquaresTestWindow::aiChooseLine()
{
	outstandingChooseLineCalls--;
	qDebug() << "aiChooseLine (outstanding calls: " << outstandingChooseLineCalls << ")";
	if (!sGame->isRunning())
	{
		qDebug() << "ERROR: game not running, not choosing any line!";
		return;
	}
	
	if (thread != NULL) {
		//thread->quit();
		//qDebug() << "waiting for previous thread to exit, isFinished: " << thread->isFinished();
		if (!thread->isFinished())
		{
			//thread->quit();
			qDebug() << "rescheduling aiChooseLine";
			outstandingChooseLineCalls++;
			QTimer::singleShot(10, this, SLOT(aiChooseLine()));
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
	connect(worker, SIGNAL(lineChosen(int)), this, SLOT(aiChoseLine(int)));
	connect(worker, SIGNAL(finished()), worker, SLOT(deleteLater()));
	//connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
	thread->start();
	/*
	int line = aiList[sGame->currentPlayerId()]->chooseLine(sGame->board()->lines(), sGame->board()->squares(), sGame->board()->getLineHistory());
	sGame->addLineToIndex(line);
	*/
}

void KSquaresTestWindow::aiChoseLine(const int &line)
{
	qDebug() << "aiChoseLine";
	if (sGame->currentPlayerId() == 0)
		currentResult.timeP1.append(aiList[sGame->currentPlayerId()]->lastMoveTime());
	else
		currentResult.timeP2.append(aiList[sGame->currentPlayerId()]->lastMoveTime());
	
	if (aiList[sGame->currentPlayerId()]->getAi()->tainted())
	{
		qDebug() << "ERROR: game is tainted! aborting";
		gameOver(sGame->getPlayers());
		return;
	}
	
	if (!sGame->addLineToIndex(line))
	{
		qDebug() << "ERROR: ai made invalid move! game is tainted! aborting";
		gameOver(sGame->getPlayers());
		return;
	}
	
	currentResult.moves.append(line);
}

void KSquaresTestWindow::gameOver(const QVector<KSquaresPlayer> & playerList)
{
	qDebug() << "Game Over";
	qDebug() << "score p1 ai: " << playerList[0].score();
	qDebug() << "score p2 ai: " << playerList[1].score();
	
	currentResult.setup = currentSetup;
	currentResult.taintedP1 = aiList[0]->getAi()->tainted();
	currentResult.taintedP2 = aiList[1]->getAi()->tainted();
	currentResult.crashesP1 = aiList[0]->getAi()->crashCount();
	currentResult.crashesP2 = aiList[1]->getAi()->crashCount();
	currentResult.scoreP1 = playerList[0].score();
	currentResult.scoreP2 = playerList[1].score();
	
	testResults.append(currentResult);
	
	updateResultStr();
	saveStatus();
	
	QTimer::singleShot(1000, this, SLOT(gameNew()));
}

void KSquaresTestWindow::updateResultStr()
{
	resultStr = QStringLiteral("Remaining games: ") + QString::number(testSetups.size());
}

//#include "ksquarestestwindow.moc"

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
	testSetups = QList<AITestSetup>();
	testResults = QList<AITestResult>();
	
	initTest();
	
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
	statusMap["setups"] = setupList;
	statusMap["results"] = resultList;
	
	QJson::Serializer serializer;
	bool ok;
	QByteArray json = serializer.serialize(statusMap, &ok);

	if (ok) {
		kDebug() << "Setup as json: " << json;
	} else {
		kDebug() << "Something went wrong:" << serializer.errorMessage();
		return;
	}

	QFile file("ksquares-test-status.json");
	if (!file.open(QIODevice::ReadWrite | QIODevice::Truncate))
	{
		kDebug() << "KSquaresTest error: Can't open file";
		return;
	}
	
	QTextStream outStream(&file);
	outStream << json;
	file.close();
}

bool KSquaresTestWindow::loadStatus()
{
	QFile file("ksquares-test-status.json");
	if (!file.open(QIODevice::ReadOnly))
	{
		kDebug() << "No previous test status file found";
		return false;
	}
	
	QJson::Parser jsonParser;
	bool parseOk;
	QVariantMap map = jsonParser.parse(&file, &parseOk).toMap();
	
	if (!parseOk)
	{
		kDebug() << "parsing failed! json error: " << jsonParser.errorString();
		return false;
	}
	
	QVariantList setups = map["setups"].toList();
	testSetups.clear();
	for (int i = 0; i < setups.size(); i++)
	{
		AITestSetup setup;
		setup.fromQVariant(setups[i]);
		testSetups.append(setup);
	}
	QVariantList results = map["results"].toList();
	testResults.clear();
	for (int i = 0; i < results.size(); i++)
	{
		AITestResult result;
		result.fromQVariant(results[i]);
		testResults.append(result);
	}
	return true;
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
	map["crashesP1"] = crashesP1;
	map["crashesP2"] = crashesP2;
	return map;
}

void AITestResult::fromQVariant(QVariant var)
{
	QVariantMap map = var.toMap();
	setup.fromQVariant(map["setup"]);
	moves.clear();
	QVariantList moveList = map["moves"].toList();
	for (int i = 0; i < moveList.size(); i++)
		moves.append(moveList[i].toInt());
	timeP1.clear();
	QVariantList timeP1List = map["timeP1"].toList();
	for (int i = 0; i < timeP1List.size(); i++)
		timeP1.append(timeP1List[i].toInt());
	timeP2.clear();
	QVariantList timeP2List = map["timeP2"].toList();
	for (int i = 0; i < timeP2List.size(); i++)
		timeP2.append(timeP2List[i].toInt());
	taintedP1 = map["taintedP1"].toBool();
	taintedP2 = map["taintedP2"].toBool();
	scoreP1 = map["scoreP1"].toInt();
	scoreP2 = map["scoreP2"].toInt();
	if (map.contains("crashesP1") && map.contains("crashesP2"))
	{
		crashesP1 = map["crashesP1"].toInt();
		crashesP2 = map["crashesP2"].toInt();
	}
	else
	{
		crashesP1 = 0;
		crashesP2 = 0;
	}
}

void KSquaresTestWindow::initTest()
{
	testSetups.clear();
	testResults.clear();
	
	if (loadStatus())
		return;
	
	for (int i = 0; i < 5; i++)
	{
		AITestSetup setup;
		setup.levelP1 = KSquares::AI_DABBLENOHASH;
		setup.levelP2 = KSquares::AI_MCTS_B;
		setup.timeout = 30000;
		setup.boardSize = QPoint(6,6);
		testSetups.append(setup);
	}
	for (int i = 0; i < 5; i++)
	{
		AITestSetup setup;
		setup.levelP1 = KSquares::AI_MCTS_B;
		setup.levelP2 = KSquares::AI_DABBLENOHASH;
		setup.timeout = 30000;
		setup.boardSize = QPoint(6,6);
		testSetups.append(setup);
	}
	for (int i = 0; i < 5; i++)
	{
		AITestSetup setup;
		setup.levelP1 = KSquares::AI_DABBLENOHASH;
		setup.levelP2 = KSquares::AI_MCTS_B;
		setup.timeout = 30000;
		setup.boardSize = QPoint(7,7);
		testSetups.append(setup);
	}
	for (int i = 0; i < 5; i++)
	{
		AITestSetup setup;
		setup.levelP1 = KSquares::AI_MCTS_B;
		setup.levelP2 = KSquares::AI_DABBLENOHASH;
		setup.timeout = 30000;
		setup.boardSize = QPoint(7,7);
		testSetups.append(setup);
	}
	
	for (int i = 0; i < 5; i++)
	{
		AITestSetup setup;
		setup.levelP1 = KSquares::AI_VERYHARD;
		setup.levelP2 = KSquares::AI_MCTS_B;
		setup.timeout = 30000;
		setup.boardSize = QPoint(6,6);
		testSetups.append(setup);
	}
	for (int i = 0; i < 5; i++)
	{
		AITestSetup setup;
		setup.levelP1 = KSquares::AI_MCTS_B;
		setup.levelP2 = KSquares::AI_VERYHARD;
		setup.timeout = 30000;
		setup.boardSize = QPoint(6,6);
		testSetups.append(setup);
	}
	for (int i = 0; i < 5; i++)
	{
		AITestSetup setup;
		setup.levelP1 = KSquares::AI_VERYHARD;
		setup.levelP2 = KSquares::AI_MCTS_B;
		setup.timeout = 30000;
		setup.boardSize = QPoint(7,7);
		testSetups.append(setup);
	}
	for (int i = 0; i < 5; i++)
	{
		AITestSetup setup;
		setup.levelP1 = KSquares::AI_MCTS_B;
		setup.levelP2 = KSquares::AI_VERYHARD;
		setup.timeout = 30000;
		setup.boardSize = QPoint(7,7);
		testSetups.append(setup);
	}
	
	for (int i = 0; i < 5; i++)
	{
		AITestSetup setup;
		setup.levelP1 = KSquares::AI_DABBLENOHASH;
		setup.levelP2 = KSquares::AI_HARD;
		setup.timeout = 30000;
		setup.boardSize = QPoint(6,6);
		testSetups.append(setup);
	}
	for (int i = 0; i < 5; i++)
	{
		AITestSetup setup;
		setup.levelP1 = KSquares::AI_HARD;
		setup.levelP2 = KSquares::AI_DABBLENOHASH;
		setup.timeout = 30000;
		setup.boardSize = QPoint(6,6);
		testSetups.append(setup);
	}
	for (int i = 0; i < 5; i++)
	{
		AITestSetup setup;
		setup.levelP1 = KSquares::AI_DABBLENOHASH;
		setup.levelP2 = KSquares::AI_HARD;
		setup.timeout = 30000;
		setup.boardSize = QPoint(7,7);
		testSetups.append(setup);
	}
	for (int i = 0; i < 5; i++)
	{
		AITestSetup setup;
		setup.levelP1 = KSquares::AI_HARD;
		setup.levelP2 = KSquares::AI_DABBLENOHASH;
		setup.timeout = 30000;
		setup.boardSize = QPoint(7,7);
		testSetups.append(setup);
	}
	return;
	for (int i = 0; i < 5; i++)
	{
		AITestSetup setup;
		setup.levelP1 = KSquares::AI_DABBLENOHASH;
		setup.levelP2 = KSquares::AI_VERYHARD;
		setup.timeout = 30000;
		setup.boardSize = QPoint(6,6);
		testSetups.append(setup);
	}
	for (int i = 0; i < 5; i++)
	{
		AITestSetup setup;
		setup.levelP1 = KSquares::AI_VERYHARD;
		setup.levelP2 = KSquares::AI_DABBLENOHASH;
		setup.timeout = 30000;
		setup.boardSize = QPoint(6,6);
		testSetups.append(setup);
	}
	for (int i = 0; i < 5; i++)
	{
		AITestSetup setup;
		setup.levelP1 = KSquares::AI_HARD;
		setup.levelP2 = KSquares::AI_VERYHARD;
		setup.timeout = 30000;
		setup.boardSize = QPoint(7,7);
		testSetups.append(setup);
	}
	for (int i = 0; i < 5; i++)
	{
		AITestSetup setup;
		setup.levelP1 = KSquares::AI_VERYHARD;
		setup.levelP2 = KSquares::AI_HARD;
		setup.timeout = 30000;
		setup.boardSize = QPoint(7,7);
		testSetups.append(setup);
	}
	for (int i = 0; i < 5; i++)
	{
		AITestSetup setup;
		setup.levelP1 = KSquares::AI_DABBLENOHASH;
		setup.levelP2 = KSquares::AI_VERYHARD;
		setup.timeout = 30000;
		setup.boardSize = QPoint(7,7);
		testSetups.append(setup);
	}
	for (int i = 0; i < 5; i++)
	{
		AITestSetup setup;
		setup.levelP1 = KSquares::AI_VERYHARD;
		setup.levelP2 = KSquares::AI_DABBLENOHASH;
		setup.timeout = 30000;
		setup.boardSize = QPoint(7,7);
		testSetups.append(setup);
	}
	
	return;
	
	for (int i = 0; i < 10; i++)
	{
		AITestSetup setup;
		setup.levelP1 = KSquares::AI_VERYHARD;
		setup.levelP2 = KSquares::AI_DABBLE;
		setup.timeout = 30000;
		setup.boardSize = QPoint(5,5);
		testSetups.append(setup);
	}
	for (int i = 0; i < 10; i++)
	{
		AITestSetup setup;
		setup.levelP1 = KSquares::AI_DABBLE;
		setup.levelP2 = KSquares::AI_VERYHARD;
		setup.timeout = 30000;
		setup.boardSize = QPoint(5,5);
		testSetups.append(setup);
	}
	for (int i = 0; i < 10; i++)
	{
		AITestSetup setup;
		setup.levelP1 = KSquares::AI_QDAB;
		setup.levelP2 = KSquares::AI_VERYHARD;
		setup.timeout = 30000;
		setup.boardSize = QPoint(5,5);
		testSetups.append(setup);
	}
	for (int i = 0; i < 10; i++)
	{
		AITestSetup setup;
		setup.levelP1 = KSquares::AI_MCTS_A;
		setup.levelP2 = KSquares::AI_MCTS_B;
		setup.timeout = 5000;
		setup.boardSize = QPoint(5,5);
		testSetups.append(setup);
	}
	for (int i = 0; i < 10; i++)
	{
		AITestSetup setup;
		setup.levelP1 = KSquares::AI_MCTS_B;
		setup.levelP2 = KSquares::AI_MCTS_A;
		setup.timeout = 5000;
		setup.boardSize = QPoint(5,5);
		testSetups.append(setup);
	}
	for (int i = 0; i < 10; i++)
	{
		AITestSetup setup;
		setup.levelP1 = KSquares::AI_MCTS_A;
		setup.levelP2 = KSquares::AI_MCTS_C;
		setup.timeout = 5000;
		setup.boardSize = QPoint(5,5);
		testSetups.append(setup);
	}
	for (int i = 0; i < 10; i++)
	{
		AITestSetup setup;
		setup.levelP1 = KSquares::AI_MCTS_C;
		setup.levelP2 = KSquares::AI_MCTS_A;
		setup.timeout = 5000;
		setup.boardSize = QPoint(5,5);
		testSetups.append(setup);
	}
	for (int i = 0; i < 10; i++)
	{
		AITestSetup setup;
		setup.levelP1 = KSquares::AI_MCTS_B;
		setup.levelP2 = KSquares::AI_MCTS_C;
		setup.timeout = 5000;
		setup.boardSize = QPoint(5,5);
		testSetups.append(setup);
	}
	for (int i = 0; i < 10; i++)
	{
		AITestSetup setup;
		setup.levelP1 = KSquares::AI_MCTS_C;
		setup.levelP2 = KSquares::AI_MCTS_B;
		setup.timeout = 5000;
		setup.boardSize = QPoint(5,5);
		testSetups.append(setup);
	}
	return;
	
	for (int i = 0; i < 10; i++)
	{
		AITestSetup setup;
		setup.levelP1 = KSquares::AI_VERYHARD;
		setup.levelP2 = KSquares::AI_QDAB;
		setup.timeout = 30000;
		setup.boardSize = QPoint(5,5);
		testSetups.append(setup);
	}
	
	
	for (int i = 0; i < 10; i++)
	{
		AITestSetup setup;
		setup.levelP1 = KSquares::AI_QDAB;
		setup.levelP2 = KSquares::AI_HARD;
		setup.timeout = 5000;
		setup.boardSize = QPoint(5,5);
		testSetups.append(setup);
	}
	for (int i = 0; i < 10; i++)
	{
		AITestSetup setup;
		setup.levelP1 = KSquares::AI_HARD;
		setup.levelP2 = KSquares::AI_QDAB;
		setup.timeout = 5000;
		setup.boardSize = QPoint(5,5);
		testSetups.append(setup);
	}
	
	for (int i = 0; i < 10; i++)
	{
		AITestSetup setup;
		setup.levelP1 = KSquares::AI_DABBLE;
		setup.levelP2 = KSquares::AI_HARD;
		setup.timeout = 5000;
		setup.boardSize = QPoint(5,5);
		testSetups.append(setup);
	}
	for (int i = 0; i < 10; i++)
	{
		AITestSetup setup;
		setup.levelP1 = KSquares::AI_HARD;
		setup.levelP2 = KSquares::AI_DABBLE;
		setup.timeout = 5000;
		setup.boardSize = QPoint(5,5);
		testSetups.append(setup);
	}
	
	
	for (int i = 0; i < 10; i++)
	{
		AITestSetup setup;
		setup.levelP1 = KSquares::AI_HARD;
		setup.levelP2 = KSquares::AI_VERYHARD;
		setup.timeout = 30000;
		setup.boardSize = QPoint(6,6);
		testSetups.append(setup);
	}
	for (int i = 0; i < 10; i++)
	{
		AITestSetup setup;
		setup.levelP1 = KSquares::AI_VERYHARD;
		setup.levelP2 = KSquares::AI_HARD;
		setup.timeout = 30000;
		setup.boardSize = QPoint(6,6);
		testSetups.append(setup);
	}
	
	return;
	
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
	QPair<int, int> lvls = orderedLevels(setup.levelP1, setup.levelP2);
	return QString::number(lvls.first) + "-" + QString::number(lvls.second) + ":" + QString::number(setup.boardSize.x()) + "x" + QString::number(setup.boardSize.y()) + "@" + QString::number(setup.timeout) + "ms";
}

QString prettyAiLevel(int level)
{
	switch (level)
	{
		case KSquares::AI_EASY: return "KSquares (Leicht)";
		case KSquares::AI_MEDIUM: return "KSquares (Mittel)";
		case KSquares::AI_HARD: return "KSquares (Schwer)";
		case KSquares::AI_VERYHARD: return "KSquares ($\\alpha\\beta$)";
		case KSquares::AI_DABBLE: return "Dabble";
		case KSquares::AI_DABBLENOHASH: return "Dabble (NoHash)";
		case KSquares::AI_QDAB: return "QDab";
		case KSquares::AI_KNOX: return "Knox";
		case KSquares::AI_MCTS_A: return "KSquares (MCTS-1)";
		case KSquares::AI_MCTS_B: return "KSquares (MCTS-2)";
		case KSquares::AI_MCTS_C: return "KSquares (MCTS-3)";
		default: return "Unbekannt";
	}
	return "Unbekannt";
}

QString latexResultGroup(QList<AITestResult> group)
{
	if (group.size() <= 0)
		return "";
	
	QPair<int, int> lvls = orderedLevels(group[0].setup.levelP1, group[0].setup.levelP2);
	QPair<int, int> winsP1(0,0); // winsP1.second = number of times Player2 (levelP2) won when going first
	QPair<int, int> winsP2(0,0); // winsP2.first = number of times Player1 (levelP1) won when going second
	QPair<int, int> losesP1(0,0);
	QPair<int, int> losesP2(0,0);
	QPair<int, int> tainted(0,0);
	QPair<int, int> avgTime(0,0);
	QPair<unsigned long long, unsigned long long> allTime(0,0);
	QPair<unsigned long, unsigned long> timeDiv(0,0);
	
	//QString tmp = "";
	for (int i = 0; i < group.size(); i++)
	{
		AITestResult res = group[i];
		//tmp += "% setup.levelP1 = " + QString::number(res.setup.levelP1) + ", setup.levelP2 = " + QString::number(res.setup.levelP2);
		//tmp += " setup.scoreP1 = " + QString::number(res.scoreP1) + ", setup.scoreP2 = " + QString::number(res.scoreP2) + "\n";
		if (res.setup.levelP1 == lvls.first) // Player 1 is going first (player 1 is res.*P1)
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
	
	QString ret = "\\begin{table}[H]\n";
	ret += "  \\begin{tabular}{c|cccc}\n";
	ret += "    \\specialcell{Spieler 1\\\\Spieler 2} & \\specialcell{Gewonnen \\\\(Sp. 1 / Sp. 2)} & Spiele & Crashes & $\\emptyset$ Zugzeit (ms) \\\\ \\hline\n";
	ret += "    "+prettyAiLevel(lvls.first)+" & $"+QString::number(winsP1.first + winsP2.first)+"$ ($"+QString::number(winsP1.first)+"$ / $"+QString::number(winsP2.first)+"$) &  $"+QString::number(group.size())+"$ & $"+QString::number(tainted.first)+"$ & $"+QString::number(avgTime.first)+"$ \\\\\n";
	ret += "    "+prettyAiLevel(lvls.second)+" & $"+QString::number(winsP1.second + winsP2.second)+"$ ($"+QString::number(winsP1.second)+"$ / $"+QString::number(winsP2.second)+"$) & $"+QString::number(group.size())+"$ & $"+QString::number(tainted.second)+"$ & $"+QString::number(avgTime.second)+"$ \\\\\n";
	ret += "  \\end{tabular}\n";
	ret += "  \\caption{Ergebnisse von "+prettyAiLevel(lvls.first)+" gegen "+prettyAiLevel(lvls.second)+" auf $"+QString::number(group[0].setup.boardSize.x())+" \\times "+QString::number(group[0].setup.boardSize.y())+"$ Spielfeld bei maximal $"+QString::number(group[0].setup.timeout/1000)+"$ Sekunden Bedenkzeit}\n";
	ret += "\\end{table}\n";
	
	return ret;
}

void KSquaresTestWindow::generateLatexResults()
{
	QMap<QString, QList<AITestResult> > resultGroupsMap;
	for (int i = 0; i < testResults.size(); i++)
	{
		resultGroupsMap[setupType(testResults[i].setup)].append(testResults[i]);
	}
	QList< QList<AITestResult> > resultGroups = resultGroupsMap.values();
	kDebug() << resultGroups.size() << " result groups.";
	
	QString tex = "\n\n\\newcommand{\\specialcell}[2][c]{%\n";
  tex += "\\begin{tabular}[#1]{@{}c@{}}#2\\end{tabular}}\n\n";
	for (int i = 0; i < resultGroups.size(); i++)
	{
		tex += latexResultGroup(resultGroups[i]) + "\n";
	}
	kDebug() << tex;
}

void KSquaresTestWindow::gameNew()
{
	generateLatexResults();
	// load test setup
	if (testSetups.size() <= 0)
	{
		kDebug() << "no more testSetups, exit application";
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
	aiController::Ptr aic0(new aiController(0, 1, width, height, currentSetup.levelP1, currentSetup.timeout));
	aiController::Ptr aic1(new aiController(1, 1, width, height, currentSetup.levelP2, currentSetup.timeout));
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
				kError() << "KSquaresGame::playerSquareComplete(); currentPlayerId() != 0|1|2|3";
		}
		playerList.append(KSquaresPlayer(aiList[i]->getAi()->getName(), color, false));
	}
	
	//create physical board
	GameBoardScene* temp = m_scene;
	m_scene = new GameBoardScene(width, height);

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
	kDebug() << "aiChoseLine";
	if (sGame->currentPlayerId() == 0)
		currentResult.timeP1.append(aiList[sGame->currentPlayerId()]->lastMoveTime());
	else
		currentResult.timeP2.append(aiList[sGame->currentPlayerId()]->lastMoveTime());
	
	if (aiList[sGame->currentPlayerId()]->getAi()->tainted())
	{
		kDebug() << "ERROR: game is tainted! aborting";
		gameOver(sGame->getPlayers());
		return;
	}
	
	if (!sGame->addLineToIndex(line))
	{
		kDebug() << "ERROR: ai made invalid move! game is tainted! aborting";
		gameOver(sGame->getPlayers());
		return;
	}
	
	currentResult.moves.append(line);
}

void KSquaresTestWindow::gameOver(const QVector<KSquaresPlayer> & playerList)
{
	kDebug() << "Game Over";
	kDebug() << "score p1 ai: " << playerList[0].score();
	kDebug() << "score p2 ai: " << playerList[1].score();
	
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
	resultStr = "Remaining games: " + QString::number(testSetups.size());
}

#include "ksquarestestwindow.moc"

/***************************************************************************
 *   Copyright (C) 2015 by Tom Vincent Peters   <kde@vincent-peters.de>    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef KSQUARESTESTWINDOW_H
#define KSQUARESTESTWINDOW_H

#include <KXmlGuiWindow>

#include <QList>
#include <QPoint>
#include <QVariant>

#include "ksquaresgame.h"
#include "aicontroller.h"

class GameBoardView;
class GameBoardScene;

/**
 * @short Test various AIs.
 *
 * @author Tom Vincent Peters <kde@vincent-peters.de>
 */

class AITestSetup {
public:
	int levelP1;
	int levelP2;
	int timeout;
	QPoint boardSize;
	
	QVariant toQVariant();
	void fromQVariant(QVariant map);
};

class AITestResult {
public:
	AITestSetup setup;
	QList<int> moves;
	QList<int> timeP1;
	QList<int> timeP2;
	bool taintedP1;
	bool taintedP2;
	int scoreP1;
	int scoreP2;
	
	QVariant toQVariant();
	void fromQVariant(QVariant map);
};

class KSquaresTestWindow : public KXmlGuiWindow
{
	Q_OBJECT

	public:
		///Constructor
		KSquaresTestWindow();
		~KSquaresTestWindow();

	public slots:
		void gameNew();
		void aiChoseLine(const int &line);

	private slots:
		void aiChooseLine();
		void playerTakeTurn(KSquaresPlayer* currentPlayer);
		void gameOver(const QVector<KSquaresPlayer> & /*playerList*/);

	private:
		///The game board view
		GameBoardView *m_view;
		///The game scene
		GameBoardScene *m_scene;
		///The game controller
		KSquaresGame* sGame;
		///List of AI players
		QList<aiController::Ptr> aiList;
		///Result map
		QMap<int, int> results;
		QString resultStr;
		
		QThread* thread;
		
		int outstandingChooseLineCalls;
		bool firstSetup;
		
		void saveStatus();
		bool loadStatus();
		void initTest();
		QList<AITestSetup> testSetups;
		QList<AITestResult> testResults;
		AITestSetup currentSetup;
		AITestResult currentResult;
		
		void updateResultStr();
		void generateLatexResults();
};

#endif // KSQUARESDEMOWINDOW_H

/***************************************************************************
 *   Copyright (C) 2006 by Matthew Williams    <matt@milliams.com>         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef KSQUARESWINDOW_H
#define KSQUARESWINDOW_H

#include <QList>
#include <QThread>

#include <KXmlGuiWindow>

#include "ksquaresgame.h"
#include "aicontroller.h"
#include "ui_prefs_ai.h"
#include "ui_prefs_display.h"

class KToggleAction;
class GameBoardView;
class GameBoardScene;

/**
 * @short Mainwindow class
 *
 * Sets actions, controls the creation of the game class and the GameBoardScene
 *
 * @author Matt Williams <matt@milliams.com>
 */

class KSquaresWindow : public KXmlGuiWindow
{
	Q_OBJECT

	public:
		///Constructor
		KSquaresWindow();
		~KSquaresWindow();

	public slots:
		void  showHighscores();
		//void  configureHighscores();
		///Launch the new game dialog and create a new game
		void gameNew();
		///Start a new game with the same settings (read from KConfig Settings)
		void gameReset();
    ///Resize the visible board
    void resetBoard(int width, int height);
    ///Load game from file
    void loadGame();
    ///Save game to file
    void saveGame();
    ///Save game to specific file
    void saveGameAs();
		///Get Level of Ai for player id
		int getAiLevel(int playerId);
		void aiChoseLine(const int &line); // testing only
		
//	signals:
//		void startAi(const QList<bool> &lines, const QList<int> &squares);

	private slots:
		void aiChooseLine(); // testing only
		void initObject();
		void optionsPreferences();
		void playerTakeTurn(KSquaresPlayer* currentPlayer);
		void gameOver(const QVector<KSquaresPlayer> &_playerList);	//when KSquaresGame says the game is over. Display score board

	private:
		enum StatusBarItem
		{
			statusplayer,
			statusnetwork
		};

		//void setupAccel();
		void setupActions();
		Ui::prefs_ai ui_prefs_ai;
		Ui::prefs_display ui_prefs_display;
		///The game board view
		GameBoardView *m_view;
		///The game scene
		GameBoardScene *m_scene;
		///The game controller
		KSquaresGame* sGame;
		// Remember last network move
		int m_lastx1, m_lasty1, m_lastx2, m_lasty2;

		//KToggleAction *m_toolbarAction;
		//KToggleAction *m_statusbarAction;
    void connectSignalsAndSlots();

		// path to the last saved game
		QString savegamePath;

		// ais for players
		QList<aiController::Ptr> ais;
};

#endif // KSQUARESWINDOW_H

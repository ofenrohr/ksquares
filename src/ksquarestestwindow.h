/***************************************************************************
 *   Copyright (C) 2014 by Tom Vincent Peters   <kde@vincent-peters.de>    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef KSQUARESTESTWINDOW_H
#define KSQUARESTESTWINDOW_H

#include <KXmlGuiWindow>

#include "ksquaresgame.h"
#include "aicontroller.h"

class GameBoardView;
class GameBoardScene;

/**
 * @short Mainwindow class for demonstration mode
 *
 * Creates a very simple mainwindow and continuously plays 4 computer players against eachother.
 *
 * @author Matt Williams <matt@milliams.com>
 */

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
};

#endif // KSQUARESDEMOWINDOW_H

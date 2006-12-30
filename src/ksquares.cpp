/***************************************************************************
 *   Copyright (C) 2006 by Matthew Williams    <matt@milliams.com>         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

//header
#include "ksquares.h"

//qt
#include <QStandardItemModel>
#include <QList>
#include <QRectF>

//kde
#include <KDE/KApplication>
#include <KDE/KConfigDialog>
#include <KDE/KStatusBar>
#include <KDE/KStandardAction>
#include <kdebug.h>
#include <KDE/KLocale>
#include <KDE/KCursor>
#include <khighscore.h>
#include <kstandardgameaction.h>

//generated
#include "settings.h"

//classes
#include "aicontroller.h"
#include "gameboardscene.h"

//ui
#include "newgamedialog.h"
#include "scoresdialog.h"

KSquares::KSquares() : KMainWindow(), m_view(new GameBoardView(this)), m_scene(0)
{	
	sGame = new KSquaresGame();
	connect(sGame, SIGNAL(takeTurnSig(KSquaresPlayer*)), this, SLOT(playerTakeTurn(KSquaresPlayer*)));
	connect(sGame, SIGNAL(gameOverSig(QVector<KSquaresPlayer>)), this, SLOT(gameOver(QVector<KSquaresPlayer>)));
	
	setCentralWidget(m_view);
	setupActions();
	statusBar()->insertPermanentItem(i18n("Current Player"), 0);
	statusBar()->show();
	setAutoSaveSettings();
	
	//gameNew();	//uncomment to start a new game on startup
}

KSquares::~KSquares()
{
}

void KSquares::setupActions()
{	
	KStandardGameAction::gameNew(this, SLOT(gameNew()), actionCollection());
	KStandardGameAction::quit(kapp, SLOT(quit()), actionCollection());
	KStandardAction::preferences(this, SLOT(optionsPreferences()), actionCollection());
	setupGUI();
}

void KSquares::gameNew()
{
	//load settings
	NewGameDialog dialog(this);
	
	//create indexed arrays of the widgets
	QList<QLineEdit*> nameLineEditList;
	nameLineEditList.append(dialog.playerOneName);
	nameLineEditList.append(dialog.playerTwoName);
	nameLineEditList.append(dialog.playerThreeName);
	nameLineEditList.append(dialog.playerFourName);
	QList<QCheckBox*> humanCheckBoxList;
	humanCheckBoxList.append(dialog.playerOneHuman);
	humanCheckBoxList.append(dialog.playerTwoHuman);
	humanCheckBoxList.append(dialog.playerThreeHuman);
	humanCheckBoxList.append(dialog.playerFourHuman);
	
	//get settings from file
	for(int i=0; i<=3; i++)
	{
		nameLineEditList.at(i)->setText(Settings::playerNames().at(i));
	}
	for(int i=0; i<=3; i++)
	{
		if (Settings::humanList().at(i) == 2)
			humanCheckBoxList.at(i)->setCheckState(Qt::Checked);
		else
			humanCheckBoxList.at(i)->setCheckState(Qt::Unchecked);
	}
	dialog.spinNumOfPlayers->setValue(Settings::numOfPlayers());
	dialog.spinHeight->setValue(Settings::boardHeight());
	dialog.spinWidth->setValue(Settings::boardWidth());
	
	//run dialog
	dialog.exec();
	
	if (dialog.result() == QDialog::Accepted)
	{
		//save settings
		Settings::setNumOfPlayers(dialog.spinNumOfPlayers->value());
		
		QStringList tempNames;
		for(int i=0; i<=3; i++)
		{
			tempNames.append(nameLineEditList.at(i)->text());
		}
		Settings::setPlayerNames(tempNames);
		
		QList<int> tempHuman;
		for(int i=0; i<=3; i++)
		{
			tempHuman.append(humanCheckBoxList.at(i)->checkState());
		}
		Settings::setHumanList(tempHuman);
		
		Settings::setBoardHeight(dialog.spinHeight->value());
		Settings::setBoardWidth(dialog.spinWidth->value());
		Settings::writeConfig();
		
		//create players
		QVector<KSquaresPlayer> playerList;
		for(int i=0; i<dialog.spinNumOfPlayers->value(); i++)
		{
			playerList.append(KSquaresPlayer(nameLineEditList.at(i)->text(), humanCheckBoxList.at(i)->isChecked()));
		}
		
		//create physical board
		if (!((dialog.spinWidth->value() == 1) and (dialog.spinHeight->value() == 1)))
		{
			GameBoardScene* temp = m_scene;
			m_scene = new GameBoardScene(dialog.spinWidth->value(), dialog.spinHeight->value());
			m_scene->update();
			
			m_view->setScene(m_scene);
			delete temp;
			//m_view->setEnabled(true);
			
			m_view->setBoardSize();	//refresh board zooming
		}
		
		//start game etc.
		sGame->createGame(playerList, dialog.spinWidth->value(), dialog.spinHeight->value());
		sGame->startGame();
		
		connect(m_scene, SIGNAL(lineDrawnSig()), sGame, SLOT(tryEndGo()));
		connect(m_scene, SIGNAL(squareComplete(int)), sGame, SLOT(playerSquareComplete(int)));
		connect(sGame, SIGNAL(setSquareOwnerSig(int,int)), m_scene, SLOT(setSquareOwner(int,int)));
	}
}

void KSquares::gameOver(QVector<KSquaresPlayer> playerList)
{
	ScoresDialog scoresDialog(this);
	
	QStandardItemModel* scoreTableModel = new QStandardItemModel();
	scoreTableModel->setRowCount(playerList.size());
	scoreTableModel->setColumnCount(3);
	scoreTableModel->setHeaderData(0, Qt::Horizontal, i18n("Player Name"));
	scoreTableModel->setHeaderData(1, Qt::Horizontal, i18n("Score"));
	scoreTableModel->setHeaderData(2, Qt::Horizontal, i18n("Score"));
	
	qSort(playerList.begin(), playerList.end(), qGreater<KSquaresPlayer>());
	for(int i = 0; i <  playerList.size(); i++)
	{
		scoreTableModel->setItem(i, 0, new QStandardItem(playerList.at(i).name()));
		
		QString temp;
		temp.setNum(playerList.at(i).score());
		scoreTableModel->setItem(i, 1, new QStandardItem(temp));
		
		qreal score = qreal(playerList.at(i).score()) - ((qreal(Settings::boardWidth())*qreal(Settings::boardHeight())) / (playerList.size()));
		temp.setNum(score);
		scoreTableModel->setItem(i, 2, new QStandardItem(temp));
	}
	
	if(playerList.at(0).isHuman())
	{
		//display a "<name> won! box and add to high scores"
	}
	
	scoresDialog.scoreTable->setModel(scoreTableModel);
	//scoresDialog.scoreTable->adjustSize();
	
	scoresDialog.exec();
}

void KSquares::optionsPreferences()
{
	KConfigDialog *dialog = new KConfigDialog(this, "settings", Settings::self());
	
	QWidget *displaySettingsDlg = new QWidget;
	ui_prefs_display.setupUi(displaySettingsDlg);
	dialog->addPage(displaySettingsDlg, i18n("Display"), "ksquares_display");
	
	QWidget *aiSettingsDlg = new QWidget;
	ui_prefs_ai.setupUi(aiSettingsDlg);
	dialog->addPage(aiSettingsDlg, i18n("AI"), "ksquares_ai");
	
	connect(dialog, SIGNAL(settingsChanged(const QString &)), m_view, SLOT(setBoardSize()));
	dialog->show();
}

void KSquares::playerTakeTurn(KSquaresPlayer* currentPlayer)
{
	//kDebug() << "void KSquares::playerTakeTurn(KSquaresPlayer* currentPlayer)" << endl;
	statusBar()->changeItem(currentPlayer->name(), 0);
	if(currentPlayer->isHuman())
	{
		//kDebug() << "Humans's Turn" << endl;
		//Let the human player interact with the board through the GameBoardView
		
		setCursor(KCursor::arrowCursor());
		//m_view->setEnabled(true);
	}
	else	//AI
	{
		//kDebug() << "AI's Turn" << endl;
		//lock the view to let the AI do it's magic
		setCursor(KCursor::waitCursor());
		//m_view->setEnabled(false);
		
		aiController ai(sGame->currentPlayerId(), m_scene->lines(), m_scene->squareOwners(), m_scene->boardWidth(), m_scene->boardHeight());
		m_scene->addLineToIndex(ai.drawLine());
	}
}

#include "ksquares.moc"

/***************************************************************************
 *   Copyright (C) 2006 by Matthew Williams <matt@milliams.com>            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

//header
#include "ksquareswindow.h"

//qt
#include <QtGui/QStandardItemModel>
#include <QtCore/QTimer>

//kde
#include <KConfigDialog>
#include <KActionCollection>
#include <KDebug>
#include <KLocale>
#include <KScoreDialog>
#include <KHighscore>
#include <KStandardGameAction>
#include <KStatusBar>
#include <KAction>
#include <KFileDialog>

//generated
#include "settings.h"

//classes
#include "aicontroller.h"
#include "gameboardview.h"
#include "ksquaresio.h"

//ui
#include "newgamedialog.h"
#include "scoresdialog.h"

KSquaresWindow::KSquaresWindow() : KXmlGuiWindow(), m_view(new GameBoardView(this)), m_scene(0)
{
	setCentralWidget(m_view);
	QTimer::singleShot(0, this, SLOT(initObject()));
}

KSquaresWindow::~KSquaresWindow()
{
	delete m_view;
	delete m_scene;
	delete sGame;
}

void KSquaresWindow::initObject()
{
	sGame = new KSquaresGame();
	connect(sGame, SIGNAL(takeTurnSig(KSquaresPlayer*)), this, SLOT(playerTakeTurn(KSquaresPlayer*)));
	connect(sGame, SIGNAL(gameOver(QVector<KSquaresPlayer>)), this, SLOT(gameOver(QVector<KSquaresPlayer>)));
	m_view->setRenderHints(QPainter::Antialiasing);
	m_view->setFrameStyle(QFrame::NoFrame);
	setupActions();
	statusBar()->insertPermanentItem(i18n("Current Player"), statusplayer);
	statusBar()->show();
	setAutoSaveSettings();

	gameNew();
}

//void KSquaresWindow::configureHighscores() {KExtHighscore::configure(this);}
void KSquaresWindow::showHighscores()
{
	KScoreDialog ksdialog(KScoreDialog::Name, this);
	ksdialog.addLocalizedConfigGroupName(qMakePair(QByteArray("Easy"), i18n("Easy")));
	ksdialog.addLocalizedConfigGroupName(qMakePair(QByteArray("Medium"), i18n("Medium")));
	ksdialog.addLocalizedConfigGroupName(qMakePair(QByteArray("Hard"), i18n("Hard")));
	ksdialog.exec();
}

void KSquaresWindow::gameNew()
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
	QList<QComboBox*> aiLevelList;
	aiLevelList.append(dialog.playerOneAiLevel);
	aiLevelList.append(dialog.playerTwoAiLevel);
	aiLevelList.append(dialog.playerThreeAiLevel);
	aiLevelList.append(dialog.playerFourAiLevel);

	//get settings from file
	for(int i=0; i<Settings::playerNames().size(); i++)
	{
		nameLineEditList.at(i)->setText(Settings::playerNames().at(i));
	}
	for(int i=0; i<Settings::playerNames().size(); i++)
	{
		if (Settings::humanList().at(i) == 2)
		{
			humanCheckBoxList.at(i)->setCheckState(Qt::Checked);
			aiLevelList.at(i)->setDisabled(true);
		}
		else
		{
			humanCheckBoxList.at(i)->setCheckState(Qt::Unchecked);
			aiLevelList.at(i)->setDisabled(false);
		}
	}
	aiLevelList.at(0)->setCurrentIndex(Settings::playerOneAi());
	aiLevelList.at(1)->setCurrentIndex(Settings::playerTwoAi());
	aiLevelList.at(2)->setCurrentIndex(Settings::playerThreeAi());
	aiLevelList.at(3)->setCurrentIndex(Settings::playerFourAi());

	dialog.spinNumOfPlayers->setValue(Settings::numOfPlayers());
	dialog.spinHeight->setValue(Settings::boardHeight());
	dialog.spinWidth->setValue(Settings::boardWidth());
	if (Settings::quickStart() == 2)
		dialog.quickStartCheck->setCheckState(Qt::Checked);
	else
		dialog.quickStartCheck->setCheckState(Qt::Unchecked);

	//run dialog
	dialog.exec();
	if (dialog.result() == QDialog::Rejected) return;

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
	
	Settings::setPlayerOneAi(aiLevelList.at(0)->currentIndex());
	Settings::setPlayerTwoAi(aiLevelList.at(1)->currentIndex());
	Settings::setPlayerThreeAi(aiLevelList.at(2)->currentIndex());
	Settings::setPlayerFourAi(aiLevelList.at(3)->currentIndex());

	Settings::setBoardHeight(dialog.spinHeight->value());
	Settings::setBoardWidth(dialog.spinWidth->value());
	Settings::setQuickStart(dialog.quickStartCheck->checkState());
	Settings::self()->writeConfig();

	gameReset();
}

void KSquaresWindow::gameReset()
{
	savegamePath = "";
	
  //create players
	QVector<KSquaresPlayer> playerList = KSquaresGame::createPlayers(Settings::numOfPlayers(), Settings::humanList());

  //reset visible board
	resetBoard(Settings::boardWidth(), Settings::boardHeight());

	//start game etc.
	sGame->createGame(playerList, Settings::boardWidth(), Settings::boardHeight());
  connectSignalsAndSlots();

	if (Settings::quickStart() == 2)
	{
    disconnect(sGame, SIGNAL(highlightMove(int)), m_scene, SLOT(highlightLine(int)));
		//This is being done before sGame->start(); to avoid the players cycling
		aiController ai(-1, sGame->board()->lines(), QList<int>(), sGame->board()->width(), sGame->board()->height(),0);
		QList<int> lines = ai.autoFill(8);	//There will be 8 possible safe move for the players
		QListIterator<int> i(lines);
		while (i.hasNext())
		{
			sGame->addLineToIndex(i.next());
		}
    connect(sGame, SIGNAL(highlightMove(int)), m_scene, SLOT(highlightLine(int)));
	}
	sGame->start();
	
	playerTakeTurn(sGame->currentPlayer());
}

void KSquaresWindow::connectSignalsAndSlots()
{
  connect(m_scene, SIGNAL(lineDrawn(int)), sGame, SLOT(addLineToIndex(int)));
  connect(m_scene, SIGNAL(signalMoveRequest(int,int,int,int)), SLOT(slotMoveRequest(int,int,int,int)));
  connect(sGame, SIGNAL(drawLine(int,QColor)), m_scene, SLOT(drawLine(int,QColor)));
  connect(sGame, SIGNAL(highlightMove(int)), m_scene, SLOT(highlightLine(int)));
  connect(sGame, SIGNAL(drawSquare(int,QColor)), m_scene, SLOT(drawSquare(int,QColor)));
}

void KSquaresWindow::resetBoard(int width, int height)
{
  //create physical board
  GameBoardScene* temp = m_scene;
  m_scene = new GameBoardScene(width, height);

  m_view->setScene(m_scene);
  delete temp;

  m_view->setBoardSize(); //refresh board zooming
}

void KSquaresWindow::gameOver(const QVector<KSquaresPlayer> &_playerList)
{
	QVector<KSquaresPlayer> playerList = _playerList;
	qSort(playerList.begin(), playerList.end(), qGreater<KSquaresPlayer>());
	//m_scene->displayScoreTable(playerList);

	ScoresDialog scoresDialog(this);
	scoresDialog.setButtons(KDialog::Close);
	scoresDialog.setDefaultButton(KDialog::Close);
	scoresDialog.setCaption(i18n("Scores"));

	QStandardItemModel* scoreTableModel = new QStandardItemModel(&scoresDialog);
	scoreTableModel->setRowCount(playerList.size());
	scoreTableModel->setColumnCount(2);
	scoreTableModel->setHeaderData(0, Qt::Horizontal, i18n("Player Name"));
	scoreTableModel->setHeaderData(1, Qt::Horizontal, i18n("Completed Squares"));

	for(int i = 0; i <  playerList.size(); i++)
	{
		scoreTableModel->setItem(i, 0, new QStandardItem(playerList.at(i).name()));
		scoreTableModel->item(i, 0)->setEditable(false);

		QString temp;
		temp.setNum(playerList.at(i).score());
		scoreTableModel->setItem(i, 1, new QStandardItem(temp));
		scoreTableModel->item(i, 1)->setEditable(false);
	}

	scoresDialog.scoreTable->setModel(scoreTableModel);
	scoresDialog.scoreTable->resizeColumnsToContents();
	scoresDialog.exec();

	// TODO: enable highscore if second player is human!
	if(playerList.at(0).isHuman() && playerList.size() == 2)
	{
		int score = (int)(static_cast<double>(playerList.at(0).score()) - (static_cast<double>(Settings::boardWidth()*Settings::boardHeight()) / static_cast<double>(playerList.size())));
		
		KScoreDialog ksdialog(KScoreDialog::Name, this);
		switch(Settings::playerTwoAi())
		{
			case 0:
				ksdialog.setConfigGroup(qMakePair(QByteArray("Easy"), i18n("Easy")));
				ksdialog.addLocalizedConfigGroupName(qMakePair(QByteArray("Medium"), i18n("Medium")));
				ksdialog.addLocalizedConfigGroupName(qMakePair(QByteArray("Hard"), i18n("Hard")));
				break;
			case 1:
				ksdialog.setConfigGroup(qMakePair(QByteArray("Medium"), i18n("Medium")));
				ksdialog.addLocalizedConfigGroupName(qMakePair(QByteArray("Easy"), i18n("Easy")));
				ksdialog.addLocalizedConfigGroupName(qMakePair(QByteArray("Hard"), i18n("Hard")));
				break;
			case 2:
				ksdialog.setConfigGroup(qMakePair(QByteArray("Hard"), i18n("Hard")));
				ksdialog.addLocalizedConfigGroupName(qMakePair(QByteArray("Easy"), i18n("Easy")));
				ksdialog.addLocalizedConfigGroupName(qMakePair(QByteArray("Medium"), i18n("Medium")));
				break;
		}
		KScoreDialog::FieldInfo scoreInfo;
		scoreInfo[KScoreDialog::Name]=playerList.at(0).name();
		scoreInfo[KScoreDialog::Score].setNum(score);
		
		if(ksdialog.addScore(scoreInfo, KScoreDialog::AskName))
			ksdialog.exec();
	}
}

void KSquaresWindow::playerTakeTurn(KSquaresPlayer* currentPlayer)
{
	//kDebug() << "void KSquares::playerTakeTurn(KSquaresPlayer* currentPlayer)";
	statusBar()->changeItem(
		QString::fromLatin1("<font color=\"%1\">%2</font>")
			.arg(currentPlayer->colour().name())
			.arg(currentPlayer->name()),
		statusplayer
	);
	if(currentPlayer->isHuman())
	{
		//Let the human player interact with the board through the GameBoardView
		
		setCursor(Qt::ArrowCursor);
		m_scene->enableEvents();
	}
	else	//AI
	{
		//lock the view to let the AI do it's magic
		setCursor(Qt::WaitCursor);
		m_scene->disableEvents();
		
		QTimer::singleShot(200, this, SLOT(aiChooseLine()));
		setCursor(Qt::ArrowCursor);
	}
}

void KSquaresWindow::loadGame()
{
  kDebug() << "loadGame";
  QString filename = KFileDialog::getOpenFileName(KUrl("kfiledialog:///ksquares"), "*.dbl|Dabble savegames\n*.ksq|KSquares savegames");
  if (filename.isEmpty())
  {
    return;
  }
  QList<int> lines;
  KSquaresIO::loadGame(filename, sGame, &lines);
  resetBoard(sGame->board()->width(), sGame->board()->height());
  connectSignalsAndSlots();
  for (int i = 0; i < lines.size()-1; i++)
  {
    sGame->addLineToIndex(lines.at(i));
  }
  sGame->start();
	if (lines.size() > 0)
	{
		sGame->addLineToIndex(lines.at(lines.size()-1));
	}
}

void KSquaresWindow::saveGame() 
{
  kDebug() << "saveGame";
	if (savegamePath.isEmpty())
	{
		saveGameAs();
		return;
	}
	KSquaresIO::saveGame(savegamePath, sGame);
}

void KSquaresWindow::saveGameAs() 
{
  kDebug() << "saveGameAs";
	savegamePath = KFileDialog::getSaveFileName(KUrl("kfiledialog:///ksquares"), "*.dbl|Dabble savegames\n*.ksq|KSquares savegames\n*.tex|LaTeX Picture\n*.sc.tex|LaTeX Strings and Coins Picture");
	if (!savegamePath.isEmpty())
	{
		saveGame();
	}
}

// testing only
void KSquaresWindow::aiChooseLine()
{
	int aiLevel=0;
	switch(sGame->currentPlayerId())
	{
		case 0: aiLevel = Settings::playerOneAi(); break;
		case 1: aiLevel = Settings::playerTwoAi(); break;
		case 2: aiLevel = Settings::playerThreeAi(); break;
		case 3: aiLevel = Settings::playerFourAi(); break;
	}
	aiController ai(sGame->currentPlayerId(), sGame->board()->lines(), sGame->board()->squares(), sGame->board()->width(), sGame->board()->height(), aiLevel);
	sGame->addLineToIndex(ai.chooseLine());
}

void KSquaresWindow::setupActions()
{
	KStandardGameAction::gameNew(this, SLOT(gameNew()), actionCollection());
	KAction *resetGame = KStandardGameAction::restart(this, SLOT(gameReset()), actionCollection());
	resetGame->setStatusTip(i18n("Start a new game with the current settings"));

	KStandardGameAction::highscores(this, SLOT(showHighscores()), actionCollection());
	KStandardGameAction::quit(this, SLOT(close()), actionCollection());
  
  KStandardGameAction::load(this, SLOT(loadGame()), actionCollection());
  KStandardGameAction::save(this, SLOT(saveGame()), actionCollection());
  KStandardGameAction::saveAs(this, SLOT(saveGameAs()), actionCollection());
	
	// Preferences
	KStandardAction::preferences(this, SLOT(optionsPreferences()), actionCollection());

	setupGUI();
}

void KSquaresWindow::optionsPreferences()
{
	KConfigDialog *dialog = new KConfigDialog(this, "settings", Settings::self());

	QWidget *displaySettingsDialog = new QWidget;
	ui_prefs_display.setupUi(displaySettingsDialog);
	dialog->addPage(displaySettingsDialog, i18n("Display"), "preferences-desktop-display");

	//QWidget *aiSettingsDialog = new QWidget;
	//ui_prefs_ai.setupUi(aiSettingsDialog);
	//dialog->addPage(aiSettingsDialog, i18n("Computer Player"), "games-difficult");

	connect(dialog, SIGNAL(settingsChanged(QString)), m_view, SLOT(setBoardSize()));
	dialog->show();
}

#include "ksquareswindow.moc"

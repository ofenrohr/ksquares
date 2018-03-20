//
// Created by ofenrohr on 11.03.18.
//

#include "ModelEvaluation.h"

#include <qdebug.h>
#include <QtCore/QTimer>
#include <alphaDots/ProtobufConnector.h>

using namespace AlphaDots;

ModelEvaluation::ModelEvaluation() : KXmlGuiWindow(), m_view(new QWidget()) {
    modelList = ProtobufConnector::getModelList();
    resultModel = new TestResultModel(this, modelList);
    sGame = new KSquaresGame();
    thread = nullptr;
    connect(sGame, SIGNAL(gameOver(QVector<KSquaresPlayer>)), this, SLOT(gameOver(QVector<KSquaresPlayer>)));
    connect(sGame, SIGNAL(takeTurnSig(KSquaresPlayer*)), this, SLOT(playerTakeTurn(KSquaresPlayer*)));
    createTestSetups();

    QTimer::singleShot(0, this, &ModelEvaluation::initObject);
}

ModelEvaluation::~ModelEvaluation() {
    delete resultModel;
}

void ModelEvaluation::initObject() {
    setupUi(m_view);
    setCentralWidget(m_view);
    setupGUI();

    resultsTable->setModel(resultModel);
    resultsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    m_scene = new GameBoardScene(5,5, true);
    gameView->setScene(m_scene);

    nextGame();
}

void ModelEvaluation::createTestSetups() {
    int gamesPerAi = 10; // should be an even number > 0
    int testBoardWidth = 5;
    int testBoardHeight = 5;
    int timeout = 5000;

    testSetups.clear();

    for (int m = 0; m < modelList.size(); m++) { // all models
        for (int r = 0; r < 3; r++) { // ai easy, medium, hard (r = rule based ai id)
            for (int i = 0; i < gamesPerAi / 2; i++) {
                AITestSetup setup;
                setup.levelP1 = r;
                setup.levelP2 = 3 + m;
                setup.timeout = timeout;
                setup.boardSize = QPoint(testBoardWidth, testBoardHeight);
                testSetups.append(setup);
            }
            for (int i = 0; i < gamesPerAi / 2; i++) {
                AITestSetup setup;
                setup.levelP1 = 3 + m;
                setup.levelP2 = r;
                setup.timeout = timeout;
                setup.boardSize = QPoint(testBoardWidth, testBoardHeight);
                testSetups.append(setup);
            }
        }
    }
}

void ModelEvaluation::loadTestSetup(const AITestSetup &setup) {
    currentSetup = setup;
    lineLog.clear();

	int width = setup.boardSize.x();
	int height = setup.boardSize.y();

    QString modelName = modelList[(setup.levelP1>setup.levelP2?setup.levelP1:setup.levelP2)-3].name();
	// create AI players
	aiList.clear();
	aiController::Ptr aic0(new aiController(0, 1, width, height, setup.levelP1 > 2 ? KSquares::AI_CONVNET : setup.levelP1, setup.timeout, modelName));
	aiController::Ptr aic1(new aiController(1, 1, width, height, setup.levelP2 > 2 ? KSquares::AI_CONVNET : setup.levelP2, setup.timeout, modelName));
	aiList.append(aic0);
	aiList.append(aic1);

	// create players
	QVector<KSquaresPlayer> playerList;
    playerList.append(KSquaresPlayer(QString::number(setup.levelP1), Qt::red, false));
    playerList.append(KSquaresPlayer(QString::number(setup.levelP2), Qt::blue, false));

	// create physical board
	GameBoardScene* temp = m_scene;
	m_scene = new GameBoardScene(width, height, false);
	gameView->setScene(m_scene);
	delete temp;
	gameView->setBoardSize();

    // setup game
	sGame->createGame(playerList, width, height);
	connect(sGame, SIGNAL(drawLine(int,QColor)), m_scene, SLOT(drawLine(int,QColor)));
	connect(sGame, SIGNAL(drawSquare(int,QColor)), m_scene, SLOT(drawSquare(int,QColor)));

    // update info label
    infoLbl->setText(QStringLiteral("<b>Current game</b><br/>\n") +
                     aiName(setup.levelP1) + QStringLiteral(" vs. ") + aiName(setup.levelP2) +
                     QStringLiteral("<br/><br/>\n\n<b>Games left</b><br/>\n") + QString::number(testSetups.size()));

    // start game
	sGame->start();
	playerTakeTurn(sGame->currentPlayer());
}

void ModelEvaluation::playerTakeTurn(KSquaresPlayer *currentPlayer) {
    qDebug() << "calling aiChooseLine";
    aiChooseLine();
}

void ModelEvaluation::aiChooseLine() {
    qDebug() << "aiChooseLine()";
    if (!sGame->isRunning()) {
		qDebug() << "ERROR: game not running, not choosing any line!";
		return;
	}

	if (thread != NULL) {
		if (!thread->isFinished()) {
			qDebug() << "rescheduling aiChooseLine";
			QTimer::singleShot(10, this, SLOT(aiChooseLine()));
			return;
		}
	} else {
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
}

void ModelEvaluation::aiChoseLine(const int &line) {
    qDebug() << "aiChoseLine";

    lineLog.append(line);

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
}

void ModelEvaluation::gameOver(const QVector<KSquaresPlayer> &playerList) {
    qDebug() << "Game Over";
    qDebug() << "score p1 ai: " << playerList[0].score();
    qDebug() << "score p2 ai: " << playerList[1].score();

    AITestResult currentResult;
    currentResult.setup = currentSetup;
    currentResult.taintedP1 = aiList[0]->getAi()->tainted();
    currentResult.taintedP2 = aiList[1]->getAi()->tainted();
    currentResult.crashesP1 = aiList[0]->getAi()->crashCount();
    currentResult.crashesP2 = aiList[1]->getAi()->crashCount();
    currentResult.scoreP1 = playerList[0].score();
    currentResult.scoreP2 = playerList[1].score();
    currentResult.moves = lineLog;

    resultModel->addResult(currentResult);
    resultModel->saveData();

    QTimer::singleShot(1000, this, SLOT(nextGame()));
}

void ModelEvaluation::nextGame() {
    if (testSetups.size() > 0) {
        loadTestSetup(testSetups.takeFirst());
    } else {
        qDebug() << "done";
    }
}

QString ModelEvaluation::aiName(int level) {
    switch (level) {
        case 0:
            return QStringLiteral("Easy");
        case 1:
            return QStringLiteral("Medium");
        case 2:
            return QStringLiteral("Hard");
        default:
            return modelList[level - 3].name();
    }
}

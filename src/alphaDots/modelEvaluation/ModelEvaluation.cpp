//
// Created by ofenrohr on 11.03.18.
//

#include "ModelEvaluation.h"
#include "FastModelEvaluation.h"
#include "aifunctions.h"

#include <QLocale>
#include <QtCore/QTimer>
#include <alphaDots/ProtobufConnector.h>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMessageBox>

using namespace AlphaDots;

ModelEvaluation::ModelEvaluation(QString models, bool fast, int threadCnt, int games) : KXmlGuiWindow(), m_view(new QWidget()) {
    qDebug() << "ModelEvaluation" << models << fast;
    modelList = getModelList(models);
    fastEvaluation = fast;
    threads = threadCnt;
    gamesPerAi = games;
    fastEvaluationHandler = nullptr;
    sGame = new KSquaresGame();
    thread = nullptr;
    qRegisterMetaType<QVector<KSquaresPlayer> >("QVector<KSquaresPlayer>");
    qRegisterMetaType<QVector<int> >("QVector<int>");
    connect(sGame, SIGNAL(gameOver(QVector<KSquaresPlayer>)), this, SLOT(gameOver(QVector<KSquaresPlayer>)));
    connect(sGame, SIGNAL(takeTurnSig(KSquaresPlayer*)), this, SLOT(playerTakeTurn(KSquaresPlayer*)));
    createTestSetups();
    resultModel = new TestResultModel(this, &modelList, gamesPerAi);

    QTimer::singleShot(0, this, &ModelEvaluation::initObject);
}

ModelEvaluation::~ModelEvaluation() {
    delete m_view;
    //delete resultModel;
}

void ModelEvaluation::initObject() {
    qDebug() << "initObject";
    setupUi(m_view);
    setCentralWidget(m_view);
    setupGUI();

    resultsTable->setModel(resultModel);
    resultsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    connect(saveResultsAsBtn, SIGNAL(released()), this, SLOT(saveResultsAs()));

    m_scene = new GameBoardScene(5, 5, true);
    gameView->setScene(m_scene);

    if (!fastEvaluation) {
        nextGame();
    } else {
        qDebug() << "[ModelEvaluation] Starting fast model evaluation...";
        currentGameContainer->setVisible(false);

        if (fastEvaluationHandler != nullptr) {
            // was initObject called more than once?
            QMessageBox::critical(this, tr("ModelEvaluation"), tr("ERROR: fastEvaluationHandler is not null!"));
            return;
        }
        fastEvaluationHandler = new FastModelEvaluation(threads);
        fastEvaluationHandler->startEvaluation(&testSetups, resultModel);
    }
}

QList<ModelInfo> ModelEvaluation::getModelList(QString models) {
    QList<ModelInfo> allModels = ProtobufConnector::getModelList();
    QList<ModelInfo> ret;

    models = models.trimmed();
    if (models.isEmpty()) {
        return allModels;
    }

    QList<QString> selectedModels = models.split(tr(","));
    foreach (QString modelStr, selectedModels) {
        bool foundModel = false;
        foreach (ModelInfo modelInfo, allModels) {
            if (modelStr.trimmed() == modelInfo.name()) {
                ret.append(modelInfo);
                foundModel = true;
                break;
            }
        }
            /*
        if (!foundModel) {
            KSquares::AILevel lvl = aiFunctions::parseAiLevel(modelStr, &foundModel);
            if (foundModel) {
                ModelInfo aiWithoutModel(modelStr, tr(""), tr(""), tr(""), modelStr);
                ret.append(aiWithoutModel);
            }
        }
             */
        if (!foundModel) {
            QMessageBox::critical(this, tr("ModelEvaluation"), tr("unknown model: ") + modelStr);
        }
    }

    return ret;
}

void ModelEvaluation::printModelList() {
    QList<ModelInfo> models = ProtobufConnector::getModelList();
    qDebug() << "================================================================================";
    qDebug() << "Available models:";
    qDebug() << "================================================================================";
    QString modelsStr;
    foreach (ModelInfo model, models) {
        modelsStr.append(model.name());
        modelsStr.append(tr("\n"));
        qDebug().noquote() << model.name();
    }
    qDebug() << "================================================================================";
    QMessageBox::information(nullptr, tr("KSquares - model list"), modelsStr);
}

void ModelEvaluation::createTestSetups() {
    int testBoardWidth = 5;
    int testBoardHeight = 5;
    int timeout = 5000;

    testSetups.clear();

    for (int m = 0; m < modelList.size(); m++) { // all models
        for (int r = 0; r < 3; r++) { // ai easy, medium, hard (r = rule based ai id)
            for (int i = 0; i < gamesPerAi / 2; i++) {
                AITestSetup setup;
                setup.aiLevelP1 = r;
                setup.aiLevelP2 = 3 + m;
                setup.timeout = timeout;
                setup.boardSize = QPoint(testBoardWidth, testBoardHeight);
                setup.modelNameP1 = modelList[m].name();
                setup.modelNameP2 = modelList[m].name();
                bool ok;
                setup.modelAiP1 = (KSquares::AILevel) r;
                setup.modelAiP2 = aiFunctions::parseAiLevel(modelList[m].ai(), &ok);
                if (!ok) {
                    QMessageBox::critical(nullptr, tr("Error"), tr("Creating test setups failed! Unknown model ai!") + modelList[m].ai());
                    return;
                }
                testSetups.append(setup);
            }
            for (int i = 0; i < gamesPerAi / 2; i++) {
                AITestSetup setup;
                setup.aiLevelP1 = 3 + m;
                setup.aiLevelP2 = r;
                setup.timeout = timeout;
                setup.boardSize = QPoint(testBoardWidth, testBoardHeight);
                setup.modelNameP1 = modelList[m].name();
                setup.modelNameP2 = modelList[m].name();
                bool ok;
                setup.modelAiP1 = aiFunctions::parseAiLevel(modelList[m].ai(), &ok);
                setup.modelAiP2 = (KSquares::AILevel) r;
                if (!ok) {
                    QMessageBox::critical(nullptr, tr("Error"), tr("Creating test setups failed! Unknown model ai!") + modelList[m].ai());
                    return;
                }
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

    //QString modelName = modelList[(setup.aiLevelP1>setup.aiLevelP2?setup.aiLevelP1:setup.aiLevelP2)-3].name();
	// create AI players
	aiList.clear();
	aiController::Ptr aic0(new aiController(0, 1, width, height, setup.aiLevelP1 > 2 ? setup.modelAiP1 : setup.aiLevelP1, setup.timeout, setup.modelNameP1));
	aiController::Ptr aic1(new aiController(1, 1, width, height, setup.aiLevelP2 > 2 ? setup.modelAiP2 : setup.aiLevelP2, setup.timeout, setup.modelNameP2));
	aiList.append(aic0);
	aiList.append(aic1);

	// create players
	QVector<KSquaresPlayer> playerList;
    playerList.append(KSquaresPlayer(QString::number(setup.aiLevelP1), Qt::red, false));
    playerList.append(KSquaresPlayer(QString::number(setup.aiLevelP2), Qt::blue, false));

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
    infoLbl->setText(tr("<br/><br/><b>Current game</b><br/>\n") +
                     aiName(setup.aiLevelP1) + tr(" (<span style=\"color: red; background-color: white\">red</span>) vs. ") + aiName(setup.aiLevelP2) +
                     tr(" (<span style=\"color: blue; background-color: white\">blue</span>)<br/><br/>\n\n<b>Games left</b><br/>\n") + QString::number(testSetups.size()));

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
    //resultModel->saveData(tr("ModelEvaluation.csv"));

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
            return tr("Easy");
        case 1:
            return tr("Medium");
        case 2:
            return tr("Hard");
        default:
            return modelList[level - 3].name();
    }
}

void ModelEvaluation::saveResultsAs() {
    QString dest = QFileDialog::getSaveFileName(this, QObject::tr("Save results as"), QObject::tr("ModelEvaluation.csv"), QObject::tr("Comma-separated values (*.csv)"));
    resultModel->saveData(dest);
}

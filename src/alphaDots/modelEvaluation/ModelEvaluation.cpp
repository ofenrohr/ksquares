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
#include <QtCore/QDateTime>
#include <alphaDots/ModelManager.h>
#include <aiControllerWorker.h>
#include <alphaDots/ReportLogger.h>

using namespace AlphaDots;

ModelEvaluation::ModelEvaluation(QString &models, QString &opponentModels, bool fast, int threadCnt, int games,
        QPoint boardSize, bool doQuickStart, QString reportDirectory) :
        KXmlGuiWindow(), m_view(new QWidget()) {
    qDebug() << "ModelEvaluation" << models << fast;
    modelList = getModelList(models);
    opponentModelList = getModelList(opponentModels);
    fastEvaluation = fast;
    threads = threadCnt;
    gamesPerAi = games;
    fastEvaluationHandler = nullptr;
    evaluationRunning = false;
    quickStart = doQuickStart;
    reportDir = reportDirectory;

    QDir rDir(reportDir);
    rDir.mkpath(reportDir);

    sGame = new KSquaresGame();
    thread = nullptr;
    qRegisterMetaType<QVector<KSquaresPlayer> >("QVector<KSquaresPlayer>");
    qRegisterMetaType<QVector<int> >("QVector<int>");
    connect(sGame, SIGNAL(gameOver(QVector<KSquaresPlayer>)), this, SLOT(gameOver(QVector<KSquaresPlayer>)));
    connect(sGame, SIGNAL(takeTurnSig(KSquaresPlayer*)), this, SLOT(playerTakeTurn(KSquaresPlayer*)));

    createTestSetups(boardSize);
    resultModel = new TestResultModel(this, &modelList, &opponentModelList, gamesPerAi);

    if (gamesPerAi % 2 != 0) {
        QMessageBox::critical(this, tr("ModelEvaluation error"), tr("ERROR: games per AI must be a multiple of 2"));
        QCoreApplication::exit(1);
        return;
    }

    QTimer::singleShot(0, this, &ModelEvaluation::initObject);
}

ModelEvaluation::~ModelEvaluation() {
    //AlphaDots::ModelManager::getInstance().stopAll();
    //delete m_view;
    m_view->deleteLater();
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

    startTime = QDateTime::currentDateTime();
    evaluationRunning = true;

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
        connect(fastEvaluationHandler, SIGNAL(evaluationFinished()), this, SLOT(evaluationFinished()));
        fastEvaluationHandler->startEvaluation(&testSetups, resultModel, &modelList, &opponentModelList, quickStart);
    }
}

QList<ModelInfo> ModelEvaluation::getModelList(QString &models) {
    QList<ModelInfo> allModels = ProtobufConnector::getInstance().getModelList();
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
        if (!foundModel) {
            aiFunctions::parseAiLevel(modelStr, &foundModel);
            if (foundModel) {
                ModelInfo aiWithoutModel(modelStr, tr(""), tr(""), tr(""), modelStr);
                ret.append(aiWithoutModel);
            }
        }
        if (!foundModel) {
            QMessageBox::critical(this, tr("ModelEvaluation"), tr("unknown model: ") + modelStr);
        }
    }

    return ret;
}

void ModelEvaluation::printModelList() {
    QList<ModelInfo> models = ProtobufConnector::getInstance().getModelList();
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

void ModelEvaluation::createTestSetups(QPoint boardSize) {
    createTestSetups(testSetups, boardSize, 5000, modelList, opponentModelList, gamesPerAi);
}

void ModelEvaluation::createTestSetups(QList<AITestSetup> &testSetups, QPoint boardSize, int timeout,
        QList<ModelInfo> &modelList, QList<ModelInfo> &opponentModelList, int gamesPerAi) {
    testSetups.clear();
    for (int m = 0; m < modelList.size(); m++) { // all models
        for (int r = 0; r < opponentModelList.size(); r++) {
            for (int i = 0; i < gamesPerAi / 2; i++) {
                bool ok;
                AITestSetup setup;
                setup.aiLevelP1 = -r-1;
                setup.aiLevelP2 = m+1;
                setup.timeout = timeout;
                setup.boardSize = boardSize;
                setup.modelNameP1 = opponentModelList[r].name();
                setup.modelNameP2 = modelList[m].name();
                setup.modelAiP1 = aiFunctions::parseAiLevel(opponentModelList[r].ai(), &ok);
                if (!ok) {
                    QMessageBox::critical(nullptr, tr("Error"), tr("Creating test setups failed! Unknown model ai!") + opponentModelList[r].ai());
                    return;
                }
                setup.modelAiP2 = aiFunctions::parseAiLevel(modelList[m].ai(), &ok);
                if (!ok) {
                    QMessageBox::critical(nullptr, tr("Error"), tr("Creating test setups failed! Unknown model ai!") + modelList[m].ai());
                    return;
                }
                testSetups.append(setup);
            }
            for (int i = 0; i < gamesPerAi / 2; i++) {
                bool ok;
                AITestSetup setup;
                setup.aiLevelP1 = m+1;
                setup.aiLevelP2 = -r-1;
                setup.timeout = timeout;
                setup.boardSize = boardSize;
                setup.modelNameP1 = modelList[m].name();
                setup.modelNameP2 = opponentModelList[r].name();
                setup.modelAiP1 = aiFunctions::parseAiLevel(modelList[m].ai(), &ok);
                if (!ok) {
                    QMessageBox::critical(nullptr, tr("Error"), tr("Creating test setups failed! Unknown model ai!") + modelList[m].ai());
                    return;
                }
                setup.modelAiP2 = aiFunctions::parseAiLevel(opponentModelList[r].ai(), &ok);
                if (!ok) {
                    QMessageBox::critical(nullptr, tr("Error"), tr("Creating test setups failed! Unknown model ai!") + opponentModelList[r].ai());
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
	QString p1l = setup.aiLevelP1 < 0 ? opponentModelList[-setup.aiLevelP1 -1].ai() : modelList[setup.aiLevelP1 -1].ai();
	QString p2l = setup.aiLevelP2 < 0 ? opponentModelList[-setup.aiLevelP2 -1].ai() : modelList[setup.aiLevelP2 -1].ai();
	bool ok = false;
	int p1 = aiFunctions::parseAiLevel(p1l, &ok);
	int p2 = aiFunctions::parseAiLevel(p2l, &ok);
	aiList.clear();
	aiController::Ptr aic0(new aiController(0, 1, width, height, p1, setup.timeout, setup.modelNameP1));
	aiController::Ptr aic1(new aiController(1, 1, width, height, p2, setup.timeout, setup.modelNameP2));
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

	// quick start
	if (quickStart) {
	    qDebug() << "pre-filling board for quick start";
        disconnect(sGame, &KSquaresGame::highlightMove, m_scene, &GameBoardScene::highlightLine);
        autoFillLines = aiController::autoFill(12, sGame->board()->width(), sGame->board()->height());
        for (const auto &line : autoFillLines) {
            sGame->addLineToIndex(line);
        }
        connect(sGame, &KSquaresGame::highlightMove, m_scene, &GameBoardScene::highlightLine);
    }

    // update info label
    infoLbl->setText(tr("<br/><br/><b>Current game</b><br/>\n") +
                     tr("Board size: ") + QString::number(width) + tr(" x ") + QString::number(height) + tr("<br />") +
                     aiName(setup.aiLevelP1) + tr(" (<span style=\"color: red; background-color: white\">red</span>) vs. ") + aiName(setup.aiLevelP2) +
                     tr(" (<span style=\"color: blue; background-color: white\">blue</span>)<br/><br/>\n\n<b>Games left</b><br/>\n") + QString::number(testSetups.size())
    );

    // start game
	sGame->start();
	playerTakeTurn(sGame->currentPlayer());
}

void ModelEvaluation::playerTakeTurn(KSquaresPlayer *) {
    qDebug() << "calling aiChooseLine";
    aiChooseLine();
}

void ModelEvaluation::aiChooseLine() {
    qDebug() << "aiChooseLine()";
    if (!sGame->isRunning()) {
		qDebug() << "ERROR: game not running, not choosing any line!";
		return;
	}

	if (thread != nullptr) {
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
    currentResult.autoFillMoves = autoFillLines;

    resultModel->addResult(currentResult);
    //resultModel->saveData(tr("ModelEvaluation.csv"));

    QTimer::singleShot(1000, this, SLOT(nextGame()));
}

void ModelEvaluation::nextGame() {
    if (!testSetups.empty()) {
        loadTestSetup(testSetups.takeFirst());
    } else {
        qDebug() << "done";
        evaluationFinished();
    }
}

QString ModelEvaluation::aiName(int level) {
    if (level < 0) {
        return opponentModelList[-level - 1].name();
    } else {
        return modelList[level - 1].name();
    }
}

void ModelEvaluation::evaluationFinished() {
    qDebug() << "evaluation finished!";
    evaluationRunning = false;
    endTime = QDateTime::currentDateTime();

    if (reportDir != "") {
        QString datetime = QDateTime::currentDateTime().toString(QObject::tr("yyyy-MM-dd_hh-mm-ss"));
        QString dest = reportDir + "/ModelEvaluationReport-" + datetime + ".md";
        qDebug() << "Saving evaluation report to: " << dest;
        ReportLogger::Ptr report = ReportLogger::Ptr(new ReportLogger(dest));
        writeResultsToReport(report, startTime, endTime, resultModel, threads, evaluationRunning, modelList, opponentModelList);
        QCoreApplication::exit(0);
    }
    ModelManager::getInstance().stopAll();
}

void ModelEvaluation::saveResultsAs() {
    QString datetime = QDateTime::currentDateTime().toString(QObject::tr("yyyy-MM-dd_hh-mm-ss"));
    QString dest = QFileDialog::getSaveFileName(this, QObject::tr("Save results as"), QObject::tr("ModelEvaluationReport-") + datetime + QObject::tr(".md"), QObject::tr("Markdown file (*.md)"));

    ReportLogger::Ptr report = ReportLogger::Ptr(new ReportLogger(dest));
    writeResultsToReport(report, startTime, endTime, resultModel, threads, evaluationRunning, modelList, opponentModelList);
}

void ModelEvaluation::writeResultsToReport(ReportLogger::Ptr &report, QDateTime &startTime, QDateTime &endTime,
        TestResultModel *resultModel, int threads, bool evaluationRunning, QList<ModelInfo> &modelList,
        QList<ModelInfo> &opponentModelList, bool includeArgs, bool includeGIT) {

    // generate a nice markdown grid table...
    // cmd line args
    QStringList allArgs = QCoreApplication::arguments();
    allArgs.removeFirst();
    QString cmdArgs = allArgs.join("\n");
    // git status cmd width
    // generate git status table entry
    QString gitStatus = GIT_STATUS;
    // model list
    QString modelListStr = "";
    for (const auto &model : modelList) {
        modelListStr.append(model.name() + " (" + model.path() + ")\n");
    }
    QString opponentModelListStr = "";
    for (const auto &model : opponentModelList) {
        opponentModelListStr.append(model.name() + " (" + model.path() + ")\n");
    }

    // generate markdown file
    report->log("# Model evaluation report\n");
    report->log("This is an automatically generated report for the model evaluation with KSquares.\n\n");
    report->log("## Configuration\n\n\n");
    QList<QList<QString>> reportTable;
    if (includeArgs) {
        reportTable << (QList<QString>() << "Command line args" << cmdArgs);
    }
    reportTable << (QList<QString>() << "Current time" << QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss") );
    reportTable << (QList<QString>() << "Start time" << startTime.toString("dd.MM.yyyy hh:mm:ss") );
    if (evaluationRunning) {
        reportTable << (QList<QString>() << "End time" << "Evaluation is still running" );
    } else {
        reportTable << (QList<QString>() << "End time" << endTime.toString("dd.MM.yyyy hh:mm:ss") );
    }
    if (includeGIT) {
        reportTable << (QList<QString>() << "GIT branch" << GIT_BRANCH );
        reportTable << (QList<QString>() << "GIT commit" << GIT_COMMIT_HASH );
        reportTable << (QList<QString>() << "GIT status" << gitStatus );
    }
    reportTable << (QList<QString>() << "Threads" << QString::number(threads) );
    reportTable << (QList<QString>() << "Models" << modelListStr);
    reportTable << (QList<QString>() << "Opponents" << opponentModelListStr);
    report->logTable(reportTable);
    report->log("\n");

    report->log("## Results\n\n");
    report->log(resultModel->dataToString(false));
    report->log("\n");

    report->log("## Game histories\n\n");
    report->log("|Match|Result|Board|AutoFill|Lines|\n");
    report->log("|--------|---|---|---|---------------------------------|\n");

    for (const QString &history : resultModel->getHistories()) {
        report->log(history);
        report->log("\n");
    }
    report->log("\n\n");
}

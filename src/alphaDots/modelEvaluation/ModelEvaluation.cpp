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

using namespace AlphaDots;

ModelEvaluation::ModelEvaluation(QString models, bool fast, int threadCnt, int games, QPoint boardSize) :
        KXmlGuiWindow(), m_view(new QWidget()) {
    qDebug() << "ModelEvaluation" << models << fast;
    modelList = getModelList(models);
    fastEvaluation = fast;
    threads = threadCnt;
    gamesPerAi = games;
    fastEvaluationHandler = nullptr;
    evaluationRunning = false;
    sGame = new KSquaresGame();
    thread = nullptr;
    qRegisterMetaType<QVector<KSquaresPlayer> >("QVector<KSquaresPlayer>");
    qRegisterMetaType<QVector<int> >("QVector<int>");
    connect(sGame, SIGNAL(gameOver(QVector<KSquaresPlayer>)), this, SLOT(gameOver(QVector<KSquaresPlayer>)));
    connect(sGame, SIGNAL(takeTurnSig(KSquaresPlayer*)), this, SLOT(playerTakeTurn(KSquaresPlayer*)));
    opponentModelList.clear();
    opponentModelList.append(ModelInfo("Easy", "", "", "", "easy"));
    //opponentModelList.append(ModelInfo("Medium", "", "", "", "medium"));
    //opponentModelList.append(ModelInfo("Hard", "", "", "", "hard"));
    opponentModelList.append(ProtobufConnector::getInstance().getModelByName("AlphaZeroV7"));
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
        fastEvaluationHandler->startEvaluation(&testSetups, resultModel, &modelList, &opponentModelList);
    }
}

QList<ModelInfo> ModelEvaluation::getModelList(QString models) {
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
    int testBoardWidth = boardSize.x();
    int testBoardHeight = boardSize.y();
    int timeout = 5000;

    testSetups.clear();


    for (int m = 0; m < modelList.size(); m++) { // all models
        for (int r = 0; r < opponentModelList.size(); r++) {
            for (int i = 0; i < gamesPerAi / 2; i++) {
                bool ok;
                AITestSetup setup;
                setup.aiLevelP1 = -r-1;
                setup.aiLevelP2 = m+1;
                setup.timeout = timeout;
                setup.boardSize = QPoint(testBoardWidth, testBoardHeight);
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
                setup.boardSize = QPoint(testBoardWidth, testBoardHeight);
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
    ModelManager::getInstance().stopAll();
}

void ModelEvaluation::saveResultsAs() {
    QString datetime = QDateTime::currentDateTime().toString(QObject::tr("yyyy-MM-dd_hh-mm-ss"));
    QString dest = QFileDialog::getSaveFileName(this, QObject::tr("Save results as"), QObject::tr("ModelEvaluationReport-") + datetime + QObject::tr(".md"), QObject::tr("Markdown file (*.md)"));

    QFile outputFile(dest);
    if (!outputFile.open(QIODevice::ReadWrite)) {
        qDebug() << "failed to open output file!";
        return;
    }

    // do a lot of stuff to generate a nice markdown grid table...
    //ExternalProcess git(tr("/usr/bin/git"), QStringList() << "log" << "-1" << "--format=%H"
    // find out maximum line width
    int maxWidth = 72;
    // cmd line args
    QStringList allArgs = QCoreApplication::arguments();
    allArgs.removeFirst();
    QString cmdArgs = allArgs.join(QLatin1Char(' '));
    if (cmdArgs.length() > maxWidth) {
        maxWidth = cmdArgs.length();
    }
    // git status cmd width
    QList<QString> gitStatusLines = tr(GIT_STATUS).split(tr("\n"));
    for (QString l : gitStatusLines) {
        if (l.length() > maxWidth) {
            maxWidth = l.length();
        }
    }
    // generate git status table entry
    QString gitStatus;
    bool first = true;
    for (QString l : gitStatusLines) {
        if (first) {
            first = false;
        } else {
            gitStatus.append(tr("|                 |"));
        }
        gitStatus.append(tr("%1\\ |\n").arg(l, -maxWidth + 2));
    }
    // generate markdown file
    QTextStream outputStream(&outputFile);
    outputStream << "# Model evaluation report\n";
    outputStream << "This is an automatically generated report for the model evaluation with KSquares.\n\n";
    outputStream << "## Configuration\n\n\n";
    outputStream << "+-----------------+" << tr("%1").arg(tr(""), maxWidth, QLatin1Char('-')) << "+\n";
    outputStream << "|Command line args|" << tr("%1").arg(cmdArgs, -maxWidth) << "|\n";
    outputStream << "+-----------------+" << tr("%1").arg(tr(""), maxWidth, QLatin1Char('-')) << "+\n";
    outputStream << "|Current time     |" << tr("%1").arg(QDateTime::currentDateTime().toString(QObject::tr("dd.MM.yyyy hh:mm:ss")), -maxWidth) << "|\n";
    outputStream << "+-----------------+" << tr("%1").arg(tr(""), maxWidth, QLatin1Char('-')) << "+\n";
    outputStream << "|Start time       |" << tr("%1").arg(startTime.toString(QObject::tr("dd.MM.yyyy hh:mm:ss")), -maxWidth) << "|\n";
    outputStream << "+-----------------+" << tr("%1").arg(tr(""), maxWidth, QLatin1Char('-')) << "+\n";
    if (evaluationRunning) {
        outputStream << "|End time         |" << tr("%1").arg(tr("Evaluation is still running"), -maxWidth) << "|\n";
    } else {
        outputStream << "|End time         |" << tr("%1").arg(endTime.toString(QObject::tr("dd.MM.yyyy hh:mm:ss")), -maxWidth) << "|\n";
    }
    outputStream << "+-----------------+" << tr("%1").arg(tr(""), maxWidth, QLatin1Char('-')) << "+\n";
    outputStream << "|GIT branch       |" << tr("%1").arg(tr(GIT_BRANCH), -maxWidth) << "|\n";
    outputStream << "+-----------------+" << tr("%1").arg(tr(""), maxWidth, QLatin1Char('-')) << "+\n";
    outputStream << "|GIT commit       |" << tr("%1").arg(tr(GIT_COMMIT_HASH), -maxWidth) << "|\n";
    outputStream << "+-----------------+" << tr("%1").arg(tr(""), maxWidth, QLatin1Char('-')) << "+\n";
    outputStream << "|GIT status       |" << gitStatus;
    outputStream << "+-----------------+" << tr("%1").arg(tr(""), maxWidth, QLatin1Char('-')) << "+\n";
    outputStream << "|Threads          |" << tr("%1").arg(QString::number(threads), -maxWidth) << "|\n";
    outputStream << "+-----------------+" << tr("%1").arg(tr(""), maxWidth, QLatin1Char('-')) << "+\n";
    outputStream << "\n";

    outputStream << "## Results\n\n";
    outputStream << resultModel->dataToString(false);
    outputStream << "\n";

    outputStream << "## Game histories\n\n";
    outputStream << "|Match|Result|Board|Lines|\n";
    outputStream << "|--------|---|---|---------------------------------|\n";

    for (const QString &history : resultModel->getHistories()) {
        outputStream << history;
        outputStream << "\n";
    }
    outputStream << "\n\n";
}

/***************************************************************************
 *   Copyright (C) 2006 by Matthew Williams    <matt@milliams.com>         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include <KAboutData>
#include <KCrash>
#include <KUser>
#include <QApplication>
#include <KLocalizedString>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <kdelibs4configmigrator.h>
#include <KDBusService>
#include <QtWidgets/QMessageBox>
#include <alphaDots/ModelManager.h>
#include <alphaDots/selfPlay/SelfPlay.h>
#include <alphaDots/ProtobufConnector.h>

#include "ksquareswindow.h"
#include "ksquaresdemowindow.h"
#include "ksquarestestwindow.h"
#include "settings.h"
#include "alphaDots/MLDataGenerator.h"
#include "alphaDots/modelEvaluation/ModelEvaluation.h"
#include "aiAlphaZeroMCTS.h"

static const char description[] =
    I18N_NOOP("Take it in turns to draw lines.\nIf you complete a squares, you get another go.");

static const char version[] = "0.7";

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    qRegisterMetaType< aiBoard::Ptr >("aiBoard::Ptr");

    Kdelibs4ConfigMigrator migrate(i18n("ksquares"));
    migrate.setConfigFiles(QStringList() << i18n("ksquaresrc"));
    migrate.setUiFiles(QStringList() << i18n("ksquaresui.rc"));
    migrate.migrate();
    KLocalizedString::setApplicationDomain("ksquares");
    KAboutData about(i18n("ksquares"), i18n("KSquares"), QLatin1Literal(version), i18n(description),
                     KAboutLicense::GPL, i18n("(C) 2006-2018 Matt Williams, Tom Vincent Peters"));
    about.addAuthor(i18n("Matt Williams"), i18n("Original creator"), i18n("matt@milliams.com"), i18n("http://milliams.com"));
    about.addCredit(i18n("Fela Winkelmolen"), i18n("Many patches and bugfixes"));
    about.addCredit(i18n("Tom Vincent Peters"), i18n("Various AIs, current maintainer"));
    about.setHomepage(i18n("http://games.kde.org/ksquares"));

    QCommandLineParser parser;
    KAboutData::setApplicationData(about);
    KCrash::initialize();
    parser.addVersionOption();
    parser.addHelpOption();
    parser.addOption(QCommandLineOption(QStringList() <<  i18n("demo"), i18n("Run game in demo (autoplay) mode")));
    parser.addOption(QCommandLineOption(QStringList() <<  i18n("test"), i18n("Run AI tests")));
    parser.addOption(QCommandLineOption(QStringList() <<  i18n("full-test"), i18n("Start over all AI tests")));
    parser.addOption(QCommandLineOption(QStringList() <<  i18n("generate"), i18n("Generate training data"), i18n("generate")));
    parser.addOption(QCommandLineOption(QStringList() <<  i18n("show-generate"), i18n("Generate training data")));
    parser.addOption(QCommandLineOption(QStringList() <<  i18n("dataset-generator"),
        i18n("Select dataset type to generate. valid values: firstTry, stageOne, basicStrategy, LSTM, LSTM2, StageTwo"), i18n("dataset-generator")));
    parser.addOption(QCommandLineOption(QStringList() <<  i18n("dataset-width"), i18n("Dataset width in boxes"), i18n("dataset-width")));
    parser.addOption(QCommandLineOption(QStringList() <<  i18n("dataset-height"), i18n("Dataset height in boxes"), i18n("dataset-height")));
    parser.addOption(QCommandLineOption(QStringList() <<  i18n("dataset-dest"), i18n("Dataset destination directory"), i18n("dataset-dest")));
    parser.addOption(QCommandLineOption(QStringList() <<  i18n("model-evaluation"), i18n("Evaluate all models")));
    parser.addOption(QCommandLineOption(QStringList() <<  i18n("model-list"), i18n("List all available models")));
    parser.addOption(QCommandLineOption(QStringList() <<  i18n("models"), i18n("List models to evaluate"), i18n("models")));
    parser.addOption(QCommandLineOption(QStringList() <<  i18n("fast-model-evaluation"), i18n("Run multi-threaded fast evaluation")));
    parser.addOption(QCommandLineOption(QStringList() <<  i18n("threads"), i18n("Number of threads for model evaluation and dataset generation (default: 4)"), i18n("threads")));
    parser.addOption(QCommandLineOption(QStringList() <<  i18n("games"),
        i18n("Number of games played in evaluation against Easy,Medium and Hard each (default slow: 10, default fast: 100)"), i18n("threads")));
    parser.addOption(QCommandLineOption(QStringList() <<  i18n("gpu"), i18n("When set, the model process will be allowed to use the GPU. In combination with model evaluation of more than one model, this can cause problems when more than one model is evaluated.")));
    parser.addOption(QCommandLineOption(QStringList() <<  i18n("self-play"), i18n("Generate training data in self-play")));
    parser.addOption(QCommandLineOption(QStringList() <<  i18n("iterations"), i18n("Number of iterations in self-play"), i18n("iterations")));
    parser.addOption(QCommandLineOption(QStringList() <<  i18n("iteration-size"), i18n("Number of games to play per iteration"), i18n("iteration-size")));
    parser.addOption(QCommandLineOption(QStringList() <<  i18n("initial-model"), i18n("Model to start self-play."), i18n("initial-model")));
    parser.addOption(QCommandLineOption(QStringList() <<  i18n("target-model"), i18n("Model generated by self-play"), i18n("target-model")));
    parser.addOption(QCommandLineOption(QStringList() <<  i18n("debug"), i18n("Print debug output")));
    parser.addOption(QCommandLineOption(QStringList() <<  i18n("debug-mcts"), i18n("Print debug output only for mcts")));
    parser.addOption(QCommandLineOption(QStringList() <<  i18n("batch-prediction"), i18n("Enable batch prediction")));
    parser.addOption(QCommandLineOption(QStringList() <<  i18n("log-dest"), i18n("Destination directory for debug and log files (not really implemented yet, no fun with default values)"), i18n("log-dest")));
    parser.addOption(QCommandLineOption(QStringList() <<  i18n("gpu-training"), i18n("Use GPU for training in self-play")));
    parser.addOption(QCommandLineOption(QStringList() <<  i18n("epochs"), i18n("Number of epochs in self-play training iteration"), i18n("epochs")));
    parser.addOption(QCommandLineOption(QStringList() <<  i18n("upload"), i18n("Upload results of self-play (used in docker image)")));
    parser.addOption(QCommandLineOption(QStringList() <<  i18n("hp-sigma-scale"), i18n("Hyperparameter: StageFour sigma scale for moves left, default: 0.125"), i18n("hp-sigma-scale")));
    parser.addOption(QCommandLineOption(QStringList() <<  i18n("hp-mean-scale"), i18n("Hyperparameter: StageFour mean scale for moves left, default: 0.5"), i18n("hp-mean-scale")));
    parser.addOption(QCommandLineOption(QStringList() <<  i18n("hp-mcts-iterations"), i18n("Hyperparameter: Number of iterations in AlphaZero MCTS, default: 1500"), i18n("hp-mcts-iterations")));
    parser.addOption(QCommandLineOption(QStringList() <<  i18n("hp-mcts-cpuct"), i18n("Hyperparameter: C_puct in AlphaZero MCTS, default: 10"), i18n("hp-mcts-cpuct")));
    parser.addOption(QCommandLineOption(QStringList() <<  i18n("hp-mcts-dirichlet-alpha"), i18n("Hyperparameter: dirichlet alpha in AlphaZero MCTS, default: 0.03"), i18n("hp-mcts-dirichlet-alpha")));
    parser.addOption(QCommandLineOption(QStringList() <<  i18n("hp-no-move-sequences"), i18n("Hyperparameter: set this flag to not use move sequences in AlphaZero MCTS")));
    parser.addOption(QCommandLineOption(QStringList() <<  i18n("board-sizes"), i18n("Board sizes available in self-play mode. Format AxB,CxD e.g. 5x4,4x4. Minimum: 2, Maximum 20"), i18n("board-sizes")));
    parser.addOption(QCommandLineOption(QStringList() <<  i18n("wait-for-training"), i18n("Do not generate new training data while training is running.")));

    about.setupCommandLine(&parser);
    parser.process(app);
    about.processCommandLine(&parser);

    if (parser.unknownOptionNames().size() > 0) {
        QMessageBox::critical(nullptr, i18n("Error"), i18n("unkown command line arguments!"));
        qDebug() << "ERROR: unkown command line arguments: " << parser.unknownOptionNames();
        return 1;
    }

    KDBusService service;

    app.setWindowIcon(QIcon::fromTheme(i18n("ksquares")));

    // default names for players
    KConfigGroup cg(KSharedConfig::openConfig(), "General");
    if (cg.readEntry<bool>("initializeNames", true)) {
        QStringList playerNames;
        playerNames << KUser().property(KUser::FullName).toString();
        playerNames << i18nc("default name of player", "Player %1", 2);
        playerNames << i18nc("default name of player", "Player %1", 3);
        playerNames << i18nc("default name of player", "Player %1", 4);
        Settings::setPlayerNames(playerNames);
        cg.writeEntry("initializeNames", false);
    }

    srand(time(NULL));

    // get number of threads
    int threads = 4;
    if (parser.isSet(i18n("threads"))) {
        bool ok = false;
        int tmp = parser.value(i18n("threads")).toInt(&ok);
        if (ok) {
            threads = tmp;
        }
    }

    // get board width and height
    int boardWidth = 5;
    int boardHeight = 4;
    if (
        parser.isSet(i18n("dataset-width")) &&
        parser.isSet(i18n("dataset-height"))
    ) {
        bool ok2 = false;
        int tmp = parser.value(i18n("dataset-width")).toInt(&ok2);
        if (ok2) {
            boardWidth = tmp;
            tmp = parser.value(i18n("dataset-height")).toInt(&ok2);
            if (ok2) {
                boardHeight = tmp;
            } else {
                qDebug() << "invalid dataset-height value";
            }
        } else {
            qDebug() << "invalid dataset-width value";
        }
    }

    // get number of games for evaluation
    int gamesPerAi_slow = 10;
    int gamesPerAi_fast = 100;
    if (parser.isSet(i18n("games"))) {
        bool ok = false;
        int tmp = parser.value(i18n("games")).toInt(&ok);
        if (ok) {
            gamesPerAi_slow = tmp;
            gamesPerAi_fast = tmp;
        }
    }

    // specific dataset generator requested?
    AlphaDots::DatasetType datasetType = AlphaDots::FirstTry;
    if (parser.isSet(i18n("dataset-generator"))) {
        QString datasetGeneratorParam = parser.value(i18n("dataset-generator")).toLower();
        if (datasetGeneratorParam == i18n("firsttry")) {
            datasetType = AlphaDots::FirstTry;
        } else if (datasetGeneratorParam == i18n("stageone")) {
            datasetType = AlphaDots::StageOne;
        } else if (datasetGeneratorParam == i18n("basicstrategy")) {
            datasetType = AlphaDots::BasicStrategy;
        } else if (datasetGeneratorParam == i18n("lstm")) {
            datasetType = AlphaDots::LSTM;
        } else if (datasetGeneratorParam == i18n("lstm2")) {
            datasetType = AlphaDots::LSTM2;
        } else if (datasetGeneratorParam == i18n("stagetwo")) {
            datasetType = AlphaDots::StageTwo;
        } else if (datasetGeneratorParam == i18n("stagethree")) {
            datasetType = AlphaDots::StageThree;
        } else if (datasetGeneratorParam == i18n("stagefour")) {
            datasetType = AlphaDots::StageFour;
        } else if (datasetGeneratorParam == i18n("stagefournomcts")) {
            datasetType = AlphaDots::StageFourNoMCTS;
        } else {
            QMessageBox::critical(nullptr, i18n("Error"), i18n("ERROR: unknown dataset-generator"));
            return 1;
        }
    }

    // get dataset destination
    QString datasetDest = QDir::currentPath();
    if (parser.isSet(i18n("dataset-dest"))) {
        datasetDest = parser.value(i18n("dataset-dest"));
        qDebug() << "dataset destination: " << datasetDest;
    }

    // allow gpu acceleration?
    if (parser.isSet(i18n("gpu"))) {
        AlphaDots::ModelManager::getInstance().allowGPU(true);
    }

    // self-play iteration count
    int iterationCnt = 50;
    if (parser.isSet(i18n("iterations"))) {
        bool ok = false;
        int tmp = parser.value(i18n("iterations")).toInt(&ok);
        if (ok) {
            iterationCnt = tmp-1;
        } else {
            QMessageBox::warning(nullptr, i18n("Self-Play error"), i18n("Invalid iterations"));
        }
    }

    // self-play iteration size
    int iterationSize = 128;
    if (parser.isSet(i18n("iteration-size"))) {
        bool ok = false;
        int tmp = parser.value(i18n("iteration-size")).toInt(&ok);
        if (ok) {
            iterationSize = tmp;
        } else {
            QMessageBox::warning(nullptr, i18n("Self-Play error"), i18n("Invalid iteration-size"));
        }
    }

    // self-play model names
    QString initialModelName = i18n("AlphaZeroV7");
    if (parser.isSet(i18n("initial-model"))) {
        initialModelName = parser.value(i18n("initial-model"));
    }
    QString targetModelName = i18n("AlphaZeroV10");
    if (parser.isSet(i18n("target-model"))) {
        targetModelName = parser.value(i18n("target-model"));
    }

    // extra debug output
    if (parser.isSet(i18n("debug"))) {
        AlphaDots::ModelManager::getInstance().setDebug(true);
        AlphaDots::aiAlphaZeroMCTS::setDebug(true);
    }

    // extra debug output for mcts
    if (parser.isSet(i18n("debug-mcts"))) {
        AlphaDots::aiAlphaZeroMCTS::setDebug(true);
    }

    // batch predction
    if (parser.isSet(i18n("batch-prediction"))) {
        AlphaDots::ProtobufConnector::getInstance().setBatchPredict(true);
        AlphaDots::ProtobufConnector::getInstance().setBatchSize(threads);
    }

    // log file destination
    QString logDest = QDir::currentPath();
    if (parser.isSet(i18n("log-dest"))) {
        logDest = parser.value(i18n("log-dest"));
        AlphaDots::ModelManager::getInstance().setLogDest(logDest);
    }

    // gpu training in self-play
    bool gpuTraining = false;
    if (parser.isSet(i18n("gpu-training"))) {
        gpuTraining = true;
    }

    // number of epochs in self-play training
    int epochs = 10;
    if (parser.isSet(i18n("epochs"))) {
        bool ok = false;
        int tmp = parser.value(i18n("epochs")).toInt(&ok);
        if (ok) {
            epochs = tmp;
        } else {
            QMessageBox::warning(nullptr, i18n("Self-Play error"), i18n("Invalid epochs argument"));
        }
    }

    // upload models
    bool upload = false;
    if (parser.isSet(i18n("upload"))) {
        upload = true;
    }

    // hp-sigma-scale - hyperparameter for movesLeft distribution (see test/GSLTest.cpp)
    double hpSigmaScale = 0.125;
    if (parser.isSet(i18n("hp-sigma-scale"))) {
        bool ok = false;
        double tmp = parser.value(i18n("hp-sigma-scale")).toDouble(&ok);
        if (ok) {
            hpSigmaScale = tmp;
        } else {
            QMessageBox::warning(nullptr, i18n("Self-Play error"), i18n("Invalid hp-sigma-scale argument"));
        }
    }
    AlphaDots::StageFourDataset::sigmaScale = hpSigmaScale;

    // hp-mean-scale - hyperparameter for movesLeft distribution (see test/GSLTest.cpp)
    double hpMeanScale = 0.5;
    if (parser.isSet(i18n("hp-mean-scale"))) {
        bool ok = false;
        double tmp = parser.value(i18n("hp-mean-scale")).toDouble(&ok);
        if (ok) {
            hpMeanScale = tmp;
        } else {
            QMessageBox::warning(nullptr, i18n("Self-Play error"), i18n("Invalid hp-mean-scale argument"));
        }
    }
    AlphaDots::StageFourDataset::meanScale = hpMeanScale;

    // hp-mcts-iterations - hyperparameter for alphazero mcts
    int hpMCTSIterations = 1500;
    if (parser.isSet(i18n("hp-mcts-iterations"))) {
        bool ok = false;
        int tmp = parser.value(i18n("hp-mcts-iterations")).toInt(&ok);
        if (ok) {
            hpMCTSIterations = tmp;
        } else {
            QMessageBox::warning(nullptr, i18n("Self-Play error"), i18n("Invalid hp-mcts-iterations argument"));
        }
    }
    AlphaDots::aiAlphaZeroMCTS::mcts_iterations = hpMCTSIterations;

    // hp-mcts-cpuct - hyperparameter for alphazero mcts
    double hpMCTSCpuct = 4;
    if (parser.isSet(i18n("hp-mcts-cpuct"))) {
        bool ok = false;
        double tmp = parser.value(i18n("hp-mcts-cpuct")).toDouble(&ok);
        if (ok) {
            hpMCTSCpuct = tmp;
        } else {
            QMessageBox::warning(nullptr, i18n("Self-Play error"), i18n("Invalid hp-mcts-cpuct argument"));
        }
    }
    AlphaDots::aiAlphaZeroMCTS::C_puct = hpMCTSCpuct;

    // hp-mcts-dirichlet-alpha - hyperparameter for alphazero mcts
    double hpMCTSDirichletAlpha = 0.03;
    if (parser.isSet(i18n("hp-mcts-dirichlet-alpha"))) {
        bool ok = false;
        double tmp = parser.value(i18n("hp-mcts-dirichlet-alpha")).toDouble(&ok);
        if (ok) {
            hpMCTSDirichletAlpha = tmp;
        } else {
            QMessageBox::warning(nullptr, i18n("Self-Play error"), i18n("Invalid hp-mcts-dirichlet-alpha argument"));
        }
    }
    AlphaDots::aiAlphaZeroMCTS::dirichlet_alpha = hpMCTSDirichletAlpha;

    // hp-no-move-sequences - hyperparameter for alphazero mcts
    if (parser.isSet(i18n("hp-no-move-sequences"))) {
        AlphaDots::aiAlphaZeroMCTS::use_move_sequences = false;
    }

    // board sizes
    QPoint evaluationBoardSize(5,5);
    QList<QPoint> boardSizes;
    boardSizes << QPoint(4,3) << QPoint(5,4) << QPoint(6,5) << QPoint(7,5) << QPoint(8,8) << QPoint(14,7) <<
                  QPoint(14,14) << QPoint(10,9);
    if (parser.isSet(i18n("board-sizes"))) {
        bool ok = true;
        QList<QPoint> tmp;
        for (const QString &boardSize : parser.value(i18n("board-sizes")).split(i18n(","))) {
            QStringList xy = boardSize.split(i18n("x"));
            if (xy.size() != 2) {
                ok = false;
                break;
            }
            int x = xy.value(0).toInt(&ok);
            if (!ok) {
                break;
            }
            int y = xy.value(1).toInt(&ok);
            if (!ok) {
                break;
            }
            if (x < 2 || x > 20 || y < 2 || y > 20) {
                ok = false;
                break;
            }
            tmp << QPoint(x,y);
        }
        if (!ok || boardSizes.empty()) {
            QMessageBox::warning(nullptr, i18n("Self-Play error"), i18n("Invalid board-sizes argument"));
        } else {
            boardSizes = tmp;
            evaluationBoardSize = boardSizes[0];
        }
    }

    // wait for training to finish?
    bool waitForTraining = false;
    if (parser.isSet(i18n("wait-for-training"))) {
        waitForTraining = true;
    }

    // start things
    if (parser.isSet(i18n("demo"))) {
        KSquaresDemoWindow *demoWindow = new KSquaresDemoWindow;
        demoWindow->show();
        demoWindow->gameNew();
    } else if (parser.isSet(i18n("test"))) {
        KSquaresTestWindow *testWindow = new KSquaresTestWindow;
        testWindow->show();
        testWindow->gameNew();
    } else if (parser.isSet(i18n("full-test"))) {
        KSquaresTestWindow *testWindow = new KSquaresTestWindow(true);
        testWindow->show();
        testWindow->gameNew();
    }  else if (parser.isSet(i18n("generate"))) {
        bool ok = false;
        long exampleCnt = parser.value(i18n("generate")).toLong(&ok);
        qDebug() << "Requested number of samples: " << parser.value(i18n("generate"));

        if (exampleCnt % threads != 0) {
            qDebug() << "ERROR: Number of samples must be divisible by the number of threads!";
            QMessageBox::critical(nullptr, i18n("Error"),
                                  i18n("Number of samples must be divisible by the number of threads!"));
            return 1;
        }

        AlphaDots::MLDataGenerator *dataGenerator=nullptr;
        if (ok) {
            dataGenerator = new AlphaDots::MLDataGenerator(exampleCnt, datasetType, boardWidth, boardHeight, datasetDest, threads);
        } else {
            dataGenerator = new AlphaDots::MLDataGenerator(datasetType, boardWidth, boardHeight);
        }
        dataGenerator->show();
    } else if (parser.isSet(i18n("show-generate"))) {
        AlphaDots::MLDataGenerator *dataGenerator = new AlphaDots::MLDataGenerator(datasetType, boardWidth, boardHeight);
        dataGenerator->show();
    } else if (parser.isSet(i18n("model-evaluation"))) {
        AlphaDots::ModelEvaluation *modelEvaluation = new AlphaDots::ModelEvaluation(parser.value(i18n("models")), false, threads, gamesPerAi_slow, evaluationBoardSize);
        modelEvaluation->show();
    } else if (parser.isSet(i18n("fast-model-evaluation"))) {
        AlphaDots::ModelEvaluation *modelEvaluation = new AlphaDots::ModelEvaluation(parser.value(i18n("models")), true, threads, gamesPerAi_fast, evaluationBoardSize);
        modelEvaluation->show();
    } else if (parser.isSet(i18n("model-list"))) {
        AlphaDots::ModelEvaluation::printModelList();
        return 0;
    } else if (parser.isSet(i18n("self-play"))) {
        if (!parser.isSet(i18n("dataset-generator"))) {
            datasetType = AlphaDots::StageFour;
        }
        AlphaDots::SelfPlay *selfPlay = new AlphaDots::SelfPlay(datasetDest, threads, initialModelName, targetModelName, iterationCnt, iterationSize, logDest, epochs, gpuTraining, datasetType, upload, boardSizes, waitForTraining);
        selfPlay->show();
    } else {
        KSquaresWindow *mainWindow = new KSquaresWindow;
        mainWindow->show();
    }

    return app.exec();
}

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
#include <alphaDots/aiEvaluation/SelfPlay.h>

#include "ksquareswindow.h"
#include "ksquaresdemowindow.h"
#include "ksquarestestwindow.h"
#include "settings.h"
#include "alphaDots/MLDataGenerator.h"
#include "alphaDots/modelEvaluation/ModelEvaluation.h"

static const char description[] =
    I18N_NOOP("Take it in turns to draw lines.\nIf you complete a squares, you get another go.");

static const char version[] = "0.6";

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
                     KAboutLicense::GPL, i18n("(C) 2006-2007 Matt Williams"));
    about.addAuthor(i18n("Matt Williams"), i18n("Original creator and maintainer"), i18n("matt@milliams.com"), i18n("http://milliams.com"));
    about.addCredit(i18n("Fela Winkelmolen"), i18n("Many patches and bugfixes"));
    about.addCredit(i18n("Tom Vincent Peters"), i18n("Hard AI"));
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
    parser.addOption(QCommandLineOption(QStringList() <<  i18n("iteration-size"), i18n("Number of games to play per iteration"), i18n("iteration-size")));
    parser.addOption(QCommandLineOption(QStringList() <<  i18n("initial-model"), i18n("Model to start self-play."), i18n("initial-model")));
    parser.addOption(QCommandLineOption(QStringList() <<  i18n("target-model"), i18n("Model generated by self-play"), i18n("target-model")));

    about.setupCommandLine(&parser);
    parser.process(app);
    about.processCommandLine(&parser);
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
        } else {
            QMessageBox::critical(nullptr, i18n("Error"), i18n("ERROR: unknown dataset-generator"));
            return 1;
        }
    }

    // get dataset destination
    QString datasetDest = i18n("./");
    if (parser.isSet(i18n("dataset-dest"))) {
        datasetDest = parser.value(i18n("dataset-dest"));
    }

    // allow gpu acceleration?
    if (parser.isSet(i18n("gpu"))) {
        AlphaDots::ModelManager::getInstance().allowGPU(true);
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
        AlphaDots::ModelEvaluation *modelEvaluation = new AlphaDots::ModelEvaluation(parser.value(i18n("models")), false, threads, gamesPerAi_slow);
        modelEvaluation->show();
    } else if (parser.isSet(i18n("fast-model-evaluation"))) {
        AlphaDots::ModelEvaluation *modelEvaluation = new AlphaDots::ModelEvaluation(parser.value(i18n("models")), true, threads, gamesPerAi_fast);
        modelEvaluation->show();
    } else if (parser.isSet(i18n("model-list"))) {
        AlphaDots::ModelEvaluation::printModelList();
        return 0;
    } else if (parser.isSet(i18n("self-play"))) {
        AlphaDots::SelfPlay *selfPlay = new AlphaDots::SelfPlay(datasetDest, threads, initialModelName, targetModelName, iterationSize);
        selfPlay->show();
    } else {
        KSquaresWindow *mainWindow = new KSquaresWindow;
        mainWindow->show();
    }

    return app.exec();
}

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

#include "ksquareswindow.h"
#include "ksquaresdemowindow.h"
#include "ksquarestestwindow.h"
#include "settings.h"
#include "alphaDots/MLDataGenerator.h"

static const char description[] =
    I18N_NOOP("Take it in turns to draw lines.\nIf you complete a squares, you get another go.");

static const char version[] = "0.6";

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    qRegisterMetaType< aiBoard::Ptr >("aiBoard::Ptr");

    Kdelibs4ConfigMigrator migrate(QStringLiteral("ksquares"));
    migrate.setConfigFiles(QStringList() << QStringLiteral("ksquaresrc"));
    migrate.setUiFiles(QStringList() << QStringLiteral("ksquaresui.rc"));
    migrate.migrate();
    KLocalizedString::setApplicationDomain("ksquares");
    KAboutData about(QStringLiteral("ksquares"), i18n("KSquares"), QLatin1Literal(version), i18n(description),
                     KAboutLicense::GPL, i18n("(C) 2006-2007 Matt Williams"));
    about.addAuthor(i18n("Matt Williams"), i18n("Original creator and maintainer"), QStringLiteral("matt@milliams.com"), QStringLiteral("http://milliams.com"));
    about.addCredit(i18n("Fela Winkelmolen"), i18n("Many patches and bugfixes"));
    about.addCredit(i18n("Tom Vincent Peters"), i18n("Hard AI"));
    about.setHomepage(QStringLiteral("http://games.kde.org/ksquares"));

    QCommandLineParser parser;
    KAboutData::setApplicationData(about);
    KCrash::initialize();
    parser.addVersionOption();
    parser.addHelpOption();
    parser.addOption(QCommandLineOption(QStringList() <<  QStringLiteral("demo"), i18n("Run game in demo (autoplay) mode")));
    parser.addOption(QCommandLineOption(QStringList() <<  QStringLiteral("test"), i18n("Run AI tests")));
    parser.addOption(QCommandLineOption(QStringList() <<  QStringLiteral("full-test"), i18n("Start over all AI tests")));
    parser.addOption(QCommandLineOption(QStringList() <<  QStringLiteral("generate"), i18n("Generate training data"), QStringLiteral("generate")));
    parser.addOption(QCommandLineOption(QStringList() <<  QStringLiteral("show-generate"), i18n("Generate training data")));
    parser.addOption(QCommandLineOption(QStringList() <<  QStringLiteral("dataset-generator"),
        i18n("Select dataset type to generate. valid values: firstTry, stageOne, basicStrategy, LSTM (only works for numpy dataset generation, not gui)"), QStringLiteral("dataset-generator")));
    parser.addOption(QCommandLineOption(QStringList() <<  QStringLiteral("dataset-width"), i18n("Dataset width in boxes"), QStringLiteral("dataset-width")));
    parser.addOption(QCommandLineOption(QStringList() <<  QStringLiteral("dataset-hight"), i18n("Dataset height in boxes"), QStringLiteral("dataset-height")));
    parser.addOption(QCommandLineOption(QStringList() <<  QStringLiteral("dataset-dest"), i18n("Dataset destination directory"), QStringLiteral("dataset-dest")));

    about.setupCommandLine(&parser);
    parser.process(app);
    about.processCommandLine(&parser);
    KDBusService service;

    app.setWindowIcon(QIcon::fromTheme(QStringLiteral("ksquares")));

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

    if (parser.isSet(QStringLiteral("demo"))) {
        KSquaresDemoWindow *demoWindow = new KSquaresDemoWindow;
        demoWindow->show();
        demoWindow->gameNew();
    } else if (parser.isSet(QStringLiteral("test"))) {
        KSquaresTestWindow *testWindow = new KSquaresTestWindow;
        testWindow->show();
        testWindow->gameNew();
    } else if (parser.isSet(QStringLiteral("full-test"))) {
        KSquaresTestWindow *testWindow = new KSquaresTestWindow(true);
        testWindow->show();
        testWindow->gameNew();
    }  else if (parser.isSet(QStringLiteral("generate"))) {
        bool ok = false;
        long exampleCnt = parser.value(QStringLiteral("generate")).toLong(&ok);
        qDebug() << parser.value(QStringLiteral("generate"));

        // specific dataset generator requested?
        AlphaDots::DatasetType datasetType = AlphaDots::FirstTry;
        if (parser.isSet(QStringLiteral("dataset-generator"))) {
            QString datasetGeneratorParam = parser.value(QStringLiteral("dataset-generator")).toLower();
            if (datasetGeneratorParam == QStringLiteral("firsttry")) {
                datasetType = AlphaDots::FirstTry;
            } else if (datasetGeneratorParam == QStringLiteral("stageone")) {
                datasetType = AlphaDots::StageOne;
            } else if (datasetGeneratorParam == QStringLiteral("basicstrategy")) {
                datasetType = AlphaDots::BasicStrategy;
            } else if (datasetGeneratorParam == QStringLiteral("lstm")) {
                datasetType = AlphaDots::LSTM;
            } else {
                ok = false;
            }
        }

        int boardWidth = 5;
        int boardHeight = 4;
        if (
                parser.isSet(QStringLiteral("dataset-width")) &&
                parser.isSet(QStringLiteral("dataset-height"))
        ) {
            bool ok2 = false;
            int tmp = parser.value(QStringLiteral("dataset-width")).toInt(&ok2);
            if (ok2) {
                boardWidth = tmp;
                tmp = parser.value(QStringLiteral("dataset-height")).toInt(&ok2);
                if (ok2) {
                    boardHeight = tmp;
                } else {
                    qDebug() << "invalid dataset-height value";
                }
            } else {
                qDebug() << "invalid dataset-width value";
            }
        }

        QString datasetDest = QStringLiteral("/run/media/ofenrohr/Data/AlphaDots/data");
        if (parser.isSet(QStringLiteral("dataset-dest"))) {
            datasetDest = parser.value(QStringLiteral("dataset-dest"));
        }

        AlphaDots::MLDataGenerator *dataGenerator=nullptr;
        if (ok) {
            dataGenerator = new AlphaDots::MLDataGenerator(exampleCnt, datasetType, boardWidth, boardHeight, datasetDest);
        } else {
            dataGenerator = new AlphaDots::MLDataGenerator();
        }
        dataGenerator->show();
    } else if (parser.isSet(QStringLiteral("show-generate"))) {
        AlphaDots::MLDataGenerator *dataGenerator = new AlphaDots::MLDataGenerator();
        dataGenerator->show();
    } else {
        KSquaresWindow *mainWindow = new KSquaresWindow;
        mainWindow->show();
    }

    return app.exec();
}

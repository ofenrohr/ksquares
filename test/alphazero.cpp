//
// Created by ofenrohr on 17.05.18.
//

#include <ksquaresio.h>
#include <QtTest/QtTest>
#include <aifunctions.h>
#include <alphaDots/ModelManager.h>
#include <alphaDots/MLImageGenerator.h>
#include <aiAlphaZeroMCTS.h>
#include <aiEasyMediumHard.h>
#include "alphazero.h"
#include "testutils.h"

// generated
#include "testboardpath.h"

void alphazero::testAlphaZero001() {
    //AlphaDots::ModelManager::getInstance().allowGPU(true);

    QList<int> lines;
    QScopedPointer<KSquaresGame> sGame(new KSquaresGame());
    QVERIFY(KSquaresIO::loadGame(QStringLiteral(TESTBOARDPATH) + QObject::tr("/berlekamp-3.3.dbl"), sGame.data(), &lines));
    for (int i = 0; i < lines.size(); i++) {
        bool nextPlayer, boardFilled;
        QList<int> completedSquares;
        sGame->board()->addLine(lines[i], &nextPlayer, &boardFilled, &completedSquares);
    }

    AlphaDots::aiAlphaZeroMCTS::setDebug(true);
    QList<int> testAIs;
    testAIs.append(KSquares::AI_MCTS_ALPHAZERO);
    QList<int> expectedLines;
    expectedLines.append(7);
    testutils::executeAi(testAIs, sGame->board(), lines.size() % 2, "alphazero-01", expectedLines);
}


void alphazero::testAlphaZero002() {
    //AlphaDots::ModelManager::getInstance().allowGPU(true);

    QList<int> lines;
    QScopedPointer<KSquaresGame> sGame(new KSquaresGame());
    QVERIFY(KSquaresIO::loadGame(QStringLiteral(TESTBOARDPATH) + QObject::tr("/6x5-nan.dbl"), sGame.data(), &lines));
    for (int i = 0; i < lines.size(); i++) {
        bool nextPlayer, boardFilled;
        QList<int> completedSquares;
        sGame->board()->addLine(lines[i], &nextPlayer, &boardFilled, &completedSquares);
    }

    AlphaDots::aiAlphaZeroMCTS::setDebug(true);
    QList<KSquaresAi::Ptr> testAIs;
    aiController aic(1, 1, 6, 5, 0, 5000, tr("AlphaZeroV11"));
    testAIs.append(aic.getAi());
    QList<int> expectedLines;
    expectedLines << 56 << 55 << 48 << 43;
    testutils::executeAi(testAIs, sGame->board(), lines.size() % 2, "alphazero-02", expectedLines);
}

void alphazero::testAlphaZero003() {
    double a = 10.0;
    double b = 0.0;

    double c = a/b;

    qDebug() << "c: " << c;

    QVERIFY(c == NAN);
}

void alphazero::runTest(KSquares::AILevel ai, QString testName, QString modelName, QList<QString> allNames, QList<QString> boardPaths,
                        QList<QList<int>> allExpectedLines, bool gpu) {

    qDebug() << allNames.size() << allNames;
    qDebug() << boardPaths.size() << boardPaths;
    qDebug() << allExpectedLines.size() << allExpectedLines;
    QVERIFY(boardPaths.size() == allExpectedLines.size());
    QVERIFY(allExpectedLines.size() == allNames.size());

    AlphaDots::aiAlphaZeroMCTS::setDebug(true);
    AlphaDots::ModelManager::getInstance().allowGPU(gpu);

    // prepare result table
    QString id = QString::number(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count());
    QString C = QString::number(AlphaDots::aiAlphaZeroMCTS::C_puct, 'g', 1);
    QString dir = QString::number(AlphaDots::aiAlphaZeroMCTS::dirichlet_alpha, 'g', 2);
    QString path = testName + "-" + modelName + "-C_" + C + "-dir_" + dir + "-" + testName + "-" + id + ".csv";
    QFile resultFile(path);
    QVERIFY(resultFile.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text));
    QTextStream stream(&resultFile);

    bool first = true;
    for (const QString &name : allNames) {
        if (first) {
            first = false;
        } else {
            stream << tr(",");
        }
        stream << name;
    }
    stream << "\n";

    // run berlekamp tests
    int prevWidth = 0;
    int prevHeight = 0;
    first = true;
    for (int testIdx = 0; testIdx < allNames.size(); testIdx++) {
        // prepare test run
        QString boardPath = boardPaths[testIdx];
        QList<int> expectedLines = allExpectedLines[testIdx];
        QString name = allNames[testIdx];

        // load board
        QList<int> lines;
        QScopedPointer<KSquaresGame> sGame(new KSquaresGame());
        QVERIFY(KSquaresIO::loadGame(boardPath, sGame.data(), &lines));
        for (int line : lines) {
            bool nextPlayer, boardFilled;
            QList<int> completedSquares;
            sGame->board()->addLine(line, &nextPlayer, &boardFilled, &completedSquares);
        }

        // stop process
        /*
        if (prevWidth != sGame->board()->width() || prevHeight != sGame->board()->height()) {
            prevWidth = sGame->board()->width();
            prevHeight = sGame->board()->height();
            AlphaDots::ModelManager::getInstance().stopAll();
        }
         */

        // execute ai
        aiController aic(sGame->board()->currentPlayer(), 1, sGame->board()->width(), sGame->board()->height(),
                         ai, 5000, modelName);
        int aiLine = aic.chooseLine(sGame->board()->lines(), sGame->board()->squares(), sGame->board()->getLineHistory());

        // log result
        if (first) {
            first = false;
        } else {
            stream << tr(",");
        }
        stream << (expectedLines.contains(aiLine) ? "PASS" : "FAIL");
        stream.flush();
        resultFile.flush();

        qDebug() << name << " -> " << (expectedLines.contains(aiLine) ? "PASS" : "FAIL");
        qDebug() << "expected lines: " << expectedLines;
        qDebug() << "chosen line: " << aiLine;

    }

    resultFile.close();
}

// execute all berlekamp tests
void alphazero::testAlphaZero004() {
    QList<QString> modelNames = QList<QString>() << "AlphaZeroV7" << "AlphaZero-Competition-2018-10-28-23-44-19-SP_MCTS";
    QList<QString> allNames;
    QList<QString> boardPaths;
    QList<QList<int>> allExpectedLines;

    for (int i = 1; i <= 13; i++) {
        boardPaths << tr(TESTBOARDPATH) + tr("/berlekamp-3.") + QString::number(i) + tr(".dbl");
        allNames << tr("berlekamp-") + QString::number(i);
    }

    allExpectedLines << (QList<int>() << 16);
    allExpectedLines << (QList<int>() << 17 << 21);
    allExpectedLines << (QList<int>() << 7);
    allExpectedLines << (QList<int>() << 12 << 22 << 23);
    allExpectedLines << (QList<int>() << 0);
    allExpectedLines << (QList<int>() << 7 << 10 << 19 << 22);
    allExpectedLines << (QList<int>() << 9);
    allExpectedLines << (QList<int>() << 0);
    allExpectedLines << (QList<int>() << 0);
    allExpectedLines << (QList<int>() << 11);
    allExpectedLines << (QList<int>() << 2 << 20 << 21);
    allExpectedLines << (QList<int>() << 15);
    allExpectedLines << (QList<int>() << 17);

    AlphaDots::ModelManager::getInstance().setDebug(false);
    //AlphaDots::aiAlphaZeroMCTS::C_puct = 400;
    for (QString &modelName : modelNames) {
        runTest(KSquares::AI_MCTS_ALPHAZERO, "berlekamp.mcts", modelName, allNames, boardPaths, allExpectedLines, true);
    }
}


// execute custom mcts debug test
void alphazero::testAlphaZero005() {
    QList<QString> allNames;
    QList<QString> boardPaths;
    QList<QList<int>> allExpectedLines;

    boardPaths << tr(TESTBOARDPATH) + tr("/3x3-mcts-debug-1.dbl");
    allNames << tr("mcts-debug-1");
    allExpectedLines << (QList<int>() << 2 << 6);

    AlphaDots::ModelManager::getInstance().setDebug(false);
    runTest(KSquares::AI_MCTS_ALPHAZERO, tr("mcts-debug"), tr("AlphaZeroV13"), allNames, boardPaths, allExpectedLines, true);
    runTest(KSquares::AI_MCTS_ALPHAZERO, tr("mcts-debug"), tr("AlphaZeroV13_SP"), allNames, boardPaths, allExpectedLines, true);
    runTest(KSquares::AI_MCTS_ALPHAZERO, tr("mcts-debug"), tr("AlphaZeroV13_SP2"), allNames, boardPaths, allExpectedLines, true);
}

// execute custom mcts debug test
void alphazero::testAlphaZero006() {
    QList<QString> allNames;
    QList<QString> boardPaths;
    QList<QList<int>> allExpectedLines;

    boardPaths << tr(TESTBOARDPATH) + tr("/3x3-mcts-debug-1.dbl");
    allNames << tr("mcts-debug-2");
    allExpectedLines << (QList<int>() << 2 << 6);

    // extra verbose debugging
    AlphaDots::ModelManager::getInstance().setDebug(true);

    runTest(KSquares::AI_CONVNET, tr("mcts-debug"), tr("AlphaZeroV13"), allNames, boardPaths, allExpectedLines, false);
    runTest(KSquares::AI_CONVNET, tr("mcts-debug"), tr("AlphaZeroV13_SP"), allNames, boardPaths, allExpectedLines, false);
    runTest(KSquares::AI_CONVNET, tr("mcts-debug"), tr("AlphaZeroV13_SP2"), allNames, boardPaths, allExpectedLines, false);
}

// execute custom mcts debug test
void alphazero::testAlphaZero007() {
    QList<QString> allNames;
    QList<QString> boardPaths;
    QList<QList<int>> allExpectedLines;

    boardPaths << tr(TESTBOARDPATH) + tr("/3x3-mcts-debug-2.dbl");
    allNames << tr("mcts-debug-1");
    allExpectedLines << (QList<int>() << 22);

    AlphaDots::ModelManager::getInstance().setDebug(false);
    runTest(KSquares::AI_MCTS_ALPHAZERO, tr("mcts-debug"), tr("AlphaZeroV13"), allNames, boardPaths, allExpectedLines, true);
    runTest(KSquares::AI_MCTS_ALPHAZERO, tr("mcts-debug"), tr("AlphaZeroV13_SP"), allNames, boardPaths, allExpectedLines, true);

    // extra verbose debugging
    AlphaDots::ModelManager::getInstance().setDebug(true);
    runTest(KSquares::AI_CONVNET, tr("mcts-debug-conv"), tr("AlphaZeroV13"), allNames, boardPaths, allExpectedLines, true);
    runTest(KSquares::AI_CONVNET, tr("mcts-debug-conv"), tr("AlphaZeroV13_SP"), allNames, boardPaths, allExpectedLines, true);

}

// execute all berlekamp tests
void alphazero::testAlphaZero008() {
    QString modelName = tr("AlphaZeroV13_SP2");
    QList<QString> allNames;
    QList<QString> boardPaths;
    QList<QList<int>> allExpectedLines;

    for (int i = 1; i <= 13; i++) {
        boardPaths << tr(TESTBOARDPATH) + tr("/berlekamp-3.") + QString::number(i) + tr(".dbl");
        allNames << tr("berlekamp-") + QString::number(i);
    }

    allExpectedLines << (QList<int>() << 16);
    allExpectedLines << (QList<int>() << 17 << 21);
    allExpectedLines << (QList<int>() << 7);
    allExpectedLines << (QList<int>() << 12 << 22 << 23);
    allExpectedLines << (QList<int>() << 0);
    allExpectedLines << (QList<int>() << 7 << 10 << 19 << 22);
    allExpectedLines << (QList<int>() << 9);
    allExpectedLines << (QList<int>() << 0);
    allExpectedLines << (QList<int>() << 0);
    allExpectedLines << (QList<int>() << 11);
    allExpectedLines << (QList<int>() << 2 << 20 << 21);
    allExpectedLines << (QList<int>() << 15);
    allExpectedLines << (QList<int>() << 17);

    AlphaDots::ModelManager::getInstance().setDebug(false);
    AlphaDots::ModelManager::getInstance().allowGPU(true);
    runTest(KSquares::AI_MCTS_ALPHAZERO, tr("berlekamp"), modelName, allNames, boardPaths, allExpectedLines, true);
}

void alphazero::testAlphaZero009() {
    QList<QString> allNames;
    QList<QString> boardPaths;
    QList<QList<int>> allExpectedLines;

    boardPaths << tr(TESTBOARDPATH) + tr("/2x2-test.dbl");
    allNames << tr("2x2-test");
    allExpectedLines << (QList<int>() << 7 << 8 << 10 << 11);

    AlphaDots::ModelManager::getInstance().setDebug(false);
    runTest(KSquares::AI_MCTS_ALPHAZERO, tr("mcts-debug"), tr("AlphaZeroV13"), allNames, boardPaths, allExpectedLines, true);
    runTest(KSquares::AI_MCTS_ALPHAZERO, tr("mcts-debug"), tr("AlphaZeroV13_SP"), allNames, boardPaths, allExpectedLines, true);
    runTest(KSquares::AI_MCTS_ALPHAZERO, tr("mcts-debug"), tr("AlphaZeroV13_SP2"), allNames, boardPaths, allExpectedLines, true);
}

// execute custom mcts debug test on V14
void alphazero::testAlphaZero010() {
    QList<QString> allNames;
    QList<QString> boardPaths;
    QList<QList<int>> allExpectedLines;

    boardPaths << tr(TESTBOARDPATH) + tr("/3x3-mcts-debug-1.dbl");
    allNames << tr("mcts-debug-1");
    allExpectedLines << (QList<int>() << 2 << 6);

    // extra verbose debugging
    AlphaDots::ModelManager::getInstance().setDebug(true);

    //runTest(KSquares::AI_CONVNET, tr("mcts-debug"), tr("AlphaZeroV14"), allNames, boardPaths, allExpectedLines, false);
    //runTest(KSquares::AI_CONVNET, tr("mcts-debug"), tr("AlphaZeroV14_SP"), allNames, boardPaths, allExpectedLines, false);

    // disable extra verbose debugging
    AlphaDots::ModelManager::getInstance().setDebug(false);

    runTest(KSquares::AI_MCTS_ALPHAZERO, tr("mcts-debug"), tr("AlphaZeroV14"), allNames, boardPaths, allExpectedLines, false);
    runTest(KSquares::AI_MCTS_ALPHAZERO, tr("mcts-debug"), tr("AlphaZeroV14_SP"), allNames, boardPaths, allExpectedLines, false);
}

// execute custom mcts debug test on V14
void alphazero::testAlphaZero011() {
    QList<QString> allNames;
    QList<QString> boardPaths;
    QList<QList<int>> allExpectedLines;

    boardPaths << tr(TESTBOARDPATH) + tr("/3x3-mcts-debug-1-4-0.dbl");
    allNames << tr("mcts-debug-1");
    allExpectedLines << (QList<int>() << 8);

    // extra verbose debugging
    AlphaDots::ModelManager::getInstance().setDebug(true);

    runTest(KSquares::AI_CONVNET, tr("mcts-debug"), tr("AlphaZeroV14"), allNames, boardPaths, allExpectedLines, false);
}


// execute all berlekamp tests
void alphazero::testAlphaZero012() {
    QString modelName = tr("AlphaZeroV14");
    QList<QString> allNames;
    QList<QString> boardPaths;
    QList<QList<int>> allExpectedLines;

    for (int i = 1; i <= 13; i++) {
        boardPaths << tr(TESTBOARDPATH) + tr("/berlekamp-3.") + QString::number(i) + tr(".dbl");
        allNames << tr("berlekamp-") + QString::number(i);
    }

    allExpectedLines << (QList<int>() << 16);
    allExpectedLines << (QList<int>() << 17 << 21);
    allExpectedLines << (QList<int>() << 7);
    allExpectedLines << (QList<int>() << 12 << 22 << 23);
    allExpectedLines << (QList<int>() << 0);
    allExpectedLines << (QList<int>() << 7 << 10 << 19 << 22);
    allExpectedLines << (QList<int>() << 9);
    allExpectedLines << (QList<int>() << 0);
    allExpectedLines << (QList<int>() << 0);
    allExpectedLines << (QList<int>() << 11);
    allExpectedLines << (QList<int>() << 2 << 20 << 21);
    allExpectedLines << (QList<int>() << 15);
    allExpectedLines << (QList<int>() << 17);

    AlphaDots::ModelManager::getInstance().setDebug(false);
    AlphaDots::ModelManager::getInstance().allowGPU(true);
    //AlphaDots::aiAlphaZeroMCTS::tau = 0.1;
    runTest(KSquares::AI_MCTS_ALPHAZERO, tr("berlekamp"), modelName, allNames, boardPaths, allExpectedLines, true);
}


// execute all berlekamp tests
void alphazero::testAlphaZero013() {
    QString modelName = tr("AlphaZeroV14");
    QList<QString> allNames;
    QList<QString> boardPaths;
    QList<QList<int>> allExpectedLines;

    boardPaths << tr(TESTBOARDPATH) + tr("/berlekamp-3.3.dbl");
    allNames << tr("berlekamp-3");

    allExpectedLines << (QList<int>() << 7);

    //AlphaDots::aiAlphaZeroMCTS::use_move_sequences = false;

    AlphaDots::ModelManager::getInstance().setDebug(false);
    AlphaDots::ModelManager::getInstance().allowGPU(true);
    runTest(KSquares::AI_MCTS_ALPHAZERO, tr("berlekamp"), modelName, allNames, boardPaths, allExpectedLines, true);
}

void alphazero::testAlphaZero014() {
    QString modelName = tr("AlphaZeroV14");
    QList<QString> allNames;
    QList<QString> boardPaths;
    QList<QList<int>> allExpectedLines;

    boardPaths << tr(TESTBOARDPATH) + tr("/5x5-early-preemtive-sacrifice.dbl");
    allNames << tr("early-preemtive-sacrifice");

    allExpectedLines << (QList<int>() << 43);

    //AlphaDots::aiAlphaZeroMCTS::use_move_sequences = false;

    AlphaDots::ModelManager::getInstance().setDebug(false);
    AlphaDots::ModelManager::getInstance().allowGPU(true);
    runTest(KSquares::AI_MCTS_ALPHAZERO, tr("early-preemtive-sacrifice"), modelName, allNames, boardPaths, allExpectedLines, true);
}

void alphazero::testAlphaZero015() {
    QString modelName = tr("AlphaZeroV14");
    QString boardPath;

    boardPath = tr(TESTBOARDPATH) + "/3x3-easy-fail-theory.dbl";

    // load board
    QList<int> lines;
    QScopedPointer<KSquaresGame> sGame(new KSquaresGame());
    QVERIFY(KSquaresIO::loadGame(boardPath, sGame.data(), &lines));
    bool nextPlayer, boardFilled;
    QList<int> completedSquares;
    for (int line : lines) {
        sGame->board()->addLine(line, &nextPlayer, &boardFilled, &completedSquares);
    }
    // execute ai
    aiController aic(sGame->board()->currentPlayer(), 1, sGame->board()->width(), sGame->board()->height(),
                     KSquares::AI_CONVNET, 5000, modelName);
    int aiLine = aic.chooseLine(sGame->board()->lines(), sGame->board()->squares(),
                                sGame->board()->getLineHistory());
    sGame->board()->addLine(aiLine, &nextPlayer, &boardFilled, &completedSquares);
    qDebug() << aiLine;
    qDebug().noquote() << sGame->board()->toString();
    aiLine = aic.chooseLine(sGame->board()->lines(), sGame->board()->squares(),
                                sGame->board()->getLineHistory());
    sGame->board()->addLine(aiLine, &nextPlayer, &boardFilled, &completedSquares);
    qDebug() << aiLine;
    qDebug().noquote() << sGame->board()->toString();
    aiLine = aic.chooseLine(sGame->board()->lines(), sGame->board()->squares(),
                                sGame->board()->getLineHistory());
    sGame->board()->addLine(aiLine, &nextPlayer, &boardFilled, &completedSquares);
    qDebug() << aiLine;
    qDebug().noquote() << sGame->board()->toString();
    aiLine = aic.chooseLine(sGame->board()->lines(), sGame->board()->squares(),
                            sGame->board()->getLineHistory());
    sGame->board()->addLine(aiLine, &nextPlayer, &boardFilled, &completedSquares);
    qDebug() << aiLine;
    qDebug().noquote() << sGame->board()->toString();
}


void alphazero::testAlphaZero016() {
    QString modelName = tr("AlphaZero-Competition-2018-10-28-23-44-19-SP");
    QString boardPath;

    boardPath = tr(TESTBOARDPATH) + "/3x3-easy-fail-theory.dbl";

    // load board
    QList<int> lines;
    QScopedPointer<KSquaresGame> sGame(new KSquaresGame());
    QVERIFY(KSquaresIO::loadGame(boardPath, sGame.data(), &lines));
    bool nextPlayer, boardFilled;
    QList<int> completedSquares;
    for (int line : lines) {
        sGame->board()->addLine(line, &nextPlayer, &boardFilled, &completedSquares);
    }
    // execute ai
    aiController aic(sGame->board()->currentPlayer(), 1, sGame->board()->width(), sGame->board()->height(),
                     KSquares::AI_CONVNET, 5000, modelName);
    int aiLine = aic.chooseLine(sGame->board()->lines(), sGame->board()->squares(),
                                sGame->board()->getLineHistory());
    sGame->board()->addLine(aiLine, &nextPlayer, &boardFilled, &completedSquares);
    qDebug() << aiLine;
    qDebug().noquote() << sGame->board()->toString();
    aiLine = aic.chooseLine(sGame->board()->lines(), sGame->board()->squares(),
                            sGame->board()->getLineHistory());
    sGame->board()->addLine(aiLine, &nextPlayer, &boardFilled, &completedSquares);
    qDebug() << aiLine;
    qDebug().noquote() << sGame->board()->toString();
    aiLine = aic.chooseLine(sGame->board()->lines(), sGame->board()->squares(),
                            sGame->board()->getLineHistory());
    sGame->board()->addLine(aiLine, &nextPlayer, &boardFilled, &completedSquares);
    qDebug() << aiLine;
    qDebug().noquote() << sGame->board()->toString();
    aiLine = aic.chooseLine(sGame->board()->lines(), sGame->board()->squares(),
                            sGame->board()->getLineHistory());
    sGame->board()->addLine(aiLine, &nextPlayer, &boardFilled, &completedSquares);
    qDebug() << aiLine;
    qDebug().noquote() << sGame->board()->toString();
}


// execute all berlekamp tests
void alphazero::testAlphaZero017() {
    QList<QString> modelNames = QList<QString>() << "AlphaZeroV7" << "AlphaZero-Competition-2018-10-28-23-44-19-SP_MCTS";
    QList<QString> allNames;
    QList<QString> boardPaths;
    QList<QList<int>> allExpectedLines;

    for (int i = 1; i <= 13; i++) {
        boardPaths << tr(TESTBOARDPATH) + tr("/berlekamp-3.") + QString::number(i) + tr(".dbl");
        allNames << tr("berlekamp-") + QString::number(i);
    }

    allExpectedLines << (QList<int>() << 16);
    allExpectedLines << (QList<int>() << 17 << 21);
    allExpectedLines << (QList<int>() << 7);
    allExpectedLines << (QList<int>() << 12 << 22 << 23);
    allExpectedLines << (QList<int>() << 0);
    allExpectedLines << (QList<int>() << 7 << 10 << 19 << 22);
    allExpectedLines << (QList<int>() << 9);
    allExpectedLines << (QList<int>() << 0);
    allExpectedLines << (QList<int>() << 0);
    allExpectedLines << (QList<int>() << 11);
    allExpectedLines << (QList<int>() << 2 << 20 << 21);
    allExpectedLines << (QList<int>() << 15);
    allExpectedLines << (QList<int>() << 17);

    AlphaDots::ModelManager::getInstance().setDebug(false);
    //AlphaDots::aiAlphaZeroMCTS::C_puct = 400;
    for (QString &modelName : modelNames) {
        runTest(KSquares::AI_CONVNET, "berlekamp.conv", modelName, allNames, boardPaths, allExpectedLines, true);
    }
}


// execute all berlekamp tests
void alphazero::testAlphaZero018() {
    QList<QString> allNames;
    QList<QString> boardPaths;
    QList<QList<int>> allExpectedLines;

    for (int i = 1; i <= 13; i++) {
        boardPaths << tr(TESTBOARDPATH) + tr("/berlekamp-3.") + QString::number(i) + tr(".dbl");
        allNames << tr("berlekamp-") + QString::number(i);
    }

    allExpectedLines << (QList<int>() << 16);
    allExpectedLines << (QList<int>() << 17 << 21);
    allExpectedLines << (QList<int>() << 7);
    allExpectedLines << (QList<int>() << 12 << 22 << 23);
    allExpectedLines << (QList<int>() << 0);
    allExpectedLines << (QList<int>() << 7 << 10 << 19 << 22);
    allExpectedLines << (QList<int>() << 9);
    allExpectedLines << (QList<int>() << 0);
    allExpectedLines << (QList<int>() << 0);
    allExpectedLines << (QList<int>() << 11);
    allExpectedLines << (QList<int>() << 2 << 20 << 21);
    allExpectedLines << (QList<int>() << 15);
    allExpectedLines << (QList<int>() << 17);

    AlphaDots::ModelManager::getInstance().setDebug(false);
    //AlphaDots::aiAlphaZeroMCTS::C_puct = 400;
    for (int i = 0; i < boardPaths.size(); i++) {
        QString boardPath = boardPaths[i];
        QString name = allNames[i];
        QList<int> expectedLines = allExpectedLines[i];
        //runTest(KSquares::AI_CONVNET, "berlekamp.conv", modelName, allNames, boardPaths, allExpectedLines, true);
        // load board
        QList<int> lines;
        QScopedPointer<KSquaresGame> sGame(new KSquaresGame());
        QVERIFY(KSquaresIO::loadGame(boardPath, sGame.data(), &lines));
        for (int line : lines) {
            bool nextPlayer, boardFilled;
            QList<int> completedSquares;
            sGame->board()->addLine(line, &nextPlayer, &boardFilled, &completedSquares);
        }

        KSquaresIO::saveGame(boardPath + ".tex", sGame.data());
        for (int line : expectedLines) {
            bool nextPlayer, boardFilled;
            QList<int> completedSquares;
            sGame->board()->addLine(line, &nextPlayer, &boardFilled, &completedSquares);
        }
        KSquaresIO::saveGame(boardPath + ".solution.tex", sGame.data());
    }
}

// generate some images for the thesis
void alphazero::testAlphaZero019() {
    QList<int> lines;
    QScopedPointer<KSquaresGame> sGame(new KSquaresGame());
    QVERIFY(KSquaresIO::loadGame(tr(TESTBOARDPATH) + tr("/5x4-first-try-example.dbl") , sGame.data(), &lines));
    aiBoard::Ptr board = aiBoard::Ptr(new aiBoard(sGame->board()));
    for (int line : lines) {
        board->doMove(line);
    }
    QImage input = AlphaDots::MLImageGenerator::generateInputImage(board);
    input.save("firstTryInput.png");
    KSquaresAi::Ptr fastAi = KSquaresAi::Ptr(
            new aiEasyMediumHard(board->playerId, board->width, board->height, KSquares::AI_HARD));
    QImage output = AlphaDots::MLImageGenerator::generateOutputImage(board, fastAi);
    output.save("firstTryTarget.png");
}

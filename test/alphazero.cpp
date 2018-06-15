//
// Created by ofenrohr on 17.05.18.
//

#include <ksquaresio.h>
#include <QtTest/QtTest>
#include <aifunctions.h>
#include <alphaDots/ModelManager.h>
#include <aiAlphaZeroMCTS.h>
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

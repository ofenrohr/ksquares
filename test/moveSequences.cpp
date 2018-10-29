//
// Created by ofenrohr on 29.10.18.
//

#include "moveSequences.h"

void TestMoveSequences::testMoveSeqs001() {
    QList<int> lines;
    QScopedPointer<KSquaresGame> sGame(new KSquaresGame());
    QVERIFY(KSquaresIO::loadGame(QStringLiteral(TESTBOARDPATH) + QObject::tr("/berlekamp-3.3.dbl"), sGame.data(), &lines));
    for (int i = 0; i < lines.size(); i++) {
        bool nextPlayer, boardFilled;
        QList<int> completedSquares;
        sGame->board()->addLine(lines[i], &nextPlayer, &boardFilled, &completedSquares);
    }

    aiBoard::Ptr board(new aiBoard(sGame->board()));
    KSquares::BoardAnalysis analysis = BoardAnalysisFunctions::analyseBoard(board);
    QVERIFY(analysis.capturableShortChains.size() == 0);
    QVERIFY(analysis.capturableLoopChains.size() == 0);
    QVERIFY(analysis.capturableLongChains.size() == 0);
    QVERIFY(analysis.openLongChains.size() == 1);
    QVERIFY(analysis.openShortChains.size() == 1);
    QVERIFY(analysis.openLoopChains.size() == 0);
    qDebug() << analysis.chainsAfterCapture[analysis.openShortChains[0]].lines;
    qDebug() << analysis.chainsAfterCapture[analysis.openLongChains[0]].lines;
}

// 3x3-moveseqs.dbl
void TestMoveSequences::testMoveSeqs002() {
    QList<int> lines;
    QScopedPointer<KSquaresGame> sGame(new KSquaresGame());
    QVERIFY(KSquaresIO::loadGame(QStringLiteral(TESTBOARDPATH) + QObject::tr("/3x3-moveseqs.dbl"), sGame.data(), &lines));
    for (int i = 0; i < lines.size(); i++) {
        bool nextPlayer, boardFilled;
        QList<int> completedSquares;
        sGame->board()->addLine(lines[i], &nextPlayer, &boardFilled, &completedSquares);
    }

    aiBoard::Ptr board(new aiBoard(sGame->board()));
    qDebug().noquote() << aiFunctions::boardToString(board);
    KSquares::BoardAnalysis analysis = BoardAnalysisFunctions::analyseBoard(board);
    qDebug().noquote() << aiFunctions::boardToString(board);
    qDebug() << "chains: " << analysis.chains;
    qDebug() << "chainsAfterCapture: " << analysis.chainsAfterCapture;
    QVERIFY(analysis.capturableShortChains.size() == 0);
    QVERIFY(analysis.capturableLoopChains.size() == 0);
    QVERIFY(analysis.capturableLongChains.size() == 0);
    QVERIFY(analysis.openLongChains.size() == 1);
    QVERIFY(analysis.openShortChains.size() == 0);
    QVERIFY(analysis.openLoopChains.size() == 0);
    qDebug() << analysis.chains[analysis.openLongChains[0]].lines;
}
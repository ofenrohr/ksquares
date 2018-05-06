#include "berlekamp.h"

void berlekamp::executeAi(Board *board, int player, QString name, QList<int> expectedLines) {
    // open the file
    QString filename = tr("test-") + name + tr(".log");
    QFile file(filename);
    if (!file.open(QIODevice::ReadWrite | QIODevice::Truncate)) {
        qDebug() << "error: Can't open file";
        return;
    }

    QString summaryStr;
    QTextStream summary(&summaryStr);

    summary << "Summary for " << name << ": \n";
    for (int i = 0; i <= 2; i++) {
        aiController aic(player, 1, board->width(), board->height(), i);
        KSquaresAi::Ptr ai = aic.getAi();
        if (!ai->enabled())
            continue;
        else
            qDebug() << "AI " << ai->getName() << " is enabled";
        int aiLine = ai->chooseLine(board->lines(), board->squares(), board->getLineHistory());
        if (expectedLines.contains(aiLine)) {
            summary << "PASS " << name << ": " << ai->getName() << "\n";
        } else {
            summary << "FAIL " << name << ": " << ai->getName() << ", returned: " << aiLine << ", expected: ";
            for (int j = 0; j < expectedLines.size(); j++)
                summary << QString::number(expectedLines[j]) << (j != expectedLines.size() - 1 ? "," : "");
            summary << "\n";
        }
    }
    qDebug().noquote().nospace() << summaryStr;

    QTextStream summaryFile(&file);
    summaryFile << summaryStr;
    file.close();
    qDebug() << "summary written to: " << filename;
}

/**
 * test based on berlekamps book "sophisticated child's play"
 * page 13 & 14
 */
void berlekamp::testBerlekamp001() {
    QList<int> lines;
    QScopedPointer<KSquaresGame> sGame(new KSquaresGame());
    QVERIFY(KSquaresIO::loadGame(QStringLiteral(TESTBOARDPATH) + QObject::tr("/berlekamp-3.1.dbl"), sGame.data(), &lines));
    for (int i = 0; i < lines.size(); i++) {
        bool nextPlayer, boardFilled;
        QList<int> completedSquares;
        sGame->board()->addLine(lines[i], &nextPlayer, &boardFilled, &completedSquares);
    }

    QList<int> expectedLines;
    expectedLines.append(16);
    executeAi(sGame->board(), lines.size() % 2, QObject::tr("berlekamp-01"), expectedLines);
}

/**
 * test based on berlekamps book "sophisticated child's play"
 * page 13 & 14
 */
void berlekamp::testBerlekamp002() {
    QList<int> lines;
    QScopedPointer<KSquaresGame> sGame(new KSquaresGame());
    QVERIFY(KSquaresIO::loadGame(QStringLiteral(TESTBOARDPATH) + QObject::tr("/berlekamp-3.2.dbl"), sGame.data(), &lines));
    for (int i = 0; i < lines.size(); i++) {
        bool nextPlayer, boardFilled;
        QList<int> completedSquares;
        sGame->board()->addLine(lines[i], &nextPlayer, &boardFilled, &completedSquares);
    }

    QList<int> expectedLines;
    expectedLines.append(17);
    expectedLines.append(21);
    executeAi(sGame->board(), lines.size() % 2, QObject::tr("berlekamp-02"), expectedLines);
}

/**
 * test based on berlekamps book "sophisticated child's play"
 * page 13 & 14
 */
void berlekamp::testBerlekamp003() {
    QList<int> lines;
    QScopedPointer<KSquaresGame> sGame(new KSquaresGame());
    QVERIFY(KSquaresIO::loadGame(QStringLiteral(TESTBOARDPATH) + QObject::tr("/berlekamp-3.3.dbl"), sGame.data(), &lines));
    for (int i = 0; i < lines.size(); i++) {
        bool nextPlayer, boardFilled;
        QList<int> completedSquares;
        sGame->board()->addLine(lines[i], &nextPlayer, &boardFilled, &completedSquares);
    }

    QList<int> expectedLines;
    expectedLines.append(7);
    executeAi(sGame->board(), lines.size() % 2, QObject::tr("berlekamp-03"), expectedLines);
}

/**
 * test based on berlekamps book "sophisticated child's play"
 * page 15 & 16
 */
void berlekamp::testBerlekamp004() {
    QList<int> lines;
    QScopedPointer<KSquaresGame> sGame(new KSquaresGame());
    QVERIFY(KSquaresIO::loadGame(QStringLiteral(TESTBOARDPATH) + QObject::tr("/berlekamp-3.4.dbl"), sGame.data(), &lines));
    for (int i = 0; i < lines.size(); i++) {
        bool nextPlayer, boardFilled;
        QList<int> completedSquares;
        sGame->board()->addLine(lines[i], &nextPlayer, &boardFilled, &completedSquares);
    }

    QList<int> expectedLines;
    expectedLines.append(12);
    expectedLines.append(22); // this result is not listed in berlekamp's book!
    expectedLines.append(23); // this result is not listed in berlekamp's book!
    executeAi(sGame->board(), lines.size() % 2, QObject::tr("berlekamp-04"), expectedLines);
}

/**
 * test based on berlekamps book "sophisticated child's play"
 * page 15 & 16
 */
void berlekamp::testBerlekamp005() {
    QList<int> lines;
    QScopedPointer<KSquaresGame> sGame(new KSquaresGame());
    QVERIFY(KSquaresIO::loadGame(QStringLiteral(TESTBOARDPATH) + QObject::tr("/berlekamp-3.5.dbl"), sGame.data(), &lines));
    for (int i = 0; i < lines.size(); i++) {
        bool nextPlayer, boardFilled;
        QList<int> completedSquares;
        sGame->board()->addLine(lines[i], &nextPlayer, &boardFilled, &completedSquares);
    }

    QList<int> expectedLines;
    expectedLines.append(0);
    executeAi(sGame->board(), lines.size() % 2, QObject::tr("berlekamp-05"), expectedLines);
}

/**
 * test based on berlekamps book "sophisticated child's play"
 * page 15 & 16
 */
void berlekamp::testBerlekamp006() {
    QList<int> lines;
    QScopedPointer<KSquaresGame> sGame(new KSquaresGame());
    QVERIFY(KSquaresIO::loadGame(QStringLiteral(TESTBOARDPATH) + QObject::tr("/berlekamp-3.6.dbl"), sGame.data(), &lines));
    for (int i = 0; i < lines.size(); i++) {
        bool nextPlayer, boardFilled;
        QList<int> completedSquares;
        sGame->board()->addLine(lines[i], &nextPlayer, &boardFilled, &completedSquares);
    }

    QList<int> expectedLines;
    expectedLines.append(7);
    expectedLines.append(10);
    expectedLines.append(19);
    expectedLines.append(22);
    executeAi(sGame->board(), lines.size() % 2, QObject::tr("berlekamp-06"), expectedLines);
}

/**
 * test based on berlekamps book "sophisticated child's play"
 * page 15 & 16
 */
void berlekamp::testBerlekamp007() {
    QList<int> lines;
    QScopedPointer<KSquaresGame> sGame(new KSquaresGame());
    QVERIFY(KSquaresIO::loadGame(QStringLiteral(TESTBOARDPATH) + QObject::tr("/berlekamp-3.7.dbl"), sGame.data(), &lines));
    for (int i = 0; i < lines.size(); i++) {
        bool nextPlayer, boardFilled;
        QList<int> completedSquares;
        sGame->board()->addLine(lines[i], &nextPlayer, &boardFilled, &completedSquares);
    }

    QList<int> expectedLines;
    expectedLines.append(9);
    executeAi(sGame->board(), lines.size() % 2, QObject::tr("berlekamp-07"), expectedLines);
}

/**
 * test based on berlekamps book "sophisticated child's play"
 * page 17 & 18
 */
void berlekamp::testBerlekamp008() {
    QList<int> lines;
    QScopedPointer<KSquaresGame> sGame(new KSquaresGame());
    QVERIFY(KSquaresIO::loadGame(QStringLiteral(TESTBOARDPATH) + QObject::tr("/berlekamp-3.8.dbl"), sGame.data(), &lines));
    for (int i = 0; i < lines.size(); i++) {
        bool nextPlayer, boardFilled;
        QList<int> completedSquares;
        sGame->board()->addLine(lines[i], &nextPlayer, &boardFilled, &completedSquares);
    }

    QList<int> expectedLines;
    expectedLines.append(0);
    executeAi(sGame->board(), sGame->board()->currentPlayer(), QObject::tr("berlekamp-08"), expectedLines);
}

/**
 * test based on berlekamps book "sophisticated child's play"
 * page 17 & 18
 */
void berlekamp::testBerlekamp009() {
    QList<int> lines;
    QScopedPointer<KSquaresGame> sGame(new KSquaresGame());
    QVERIFY(KSquaresIO::loadGame(QStringLiteral(TESTBOARDPATH) + QObject::tr("/berlekamp-3.9.dbl"), sGame.data(), &lines));
    for (int i = 0; i < lines.size(); i++) {
        bool nextPlayer, boardFilled;
        QList<int> completedSquares;
        sGame->board()->addLine(lines[i], &nextPlayer, &boardFilled, &completedSquares);
    }

    QList<int> expectedLines;
    expectedLines.append(0);
    executeAi(sGame->board(), sGame->board()->currentPlayer(), QObject::tr("berlekamp-09"), expectedLines);
}

/**
 * test based on berlekamps book "sophisticated child's play"
 * page 17 & 18
 */
void berlekamp::testBerlekamp010() {
    QList<int> lines;
    QScopedPointer<KSquaresGame> sGame(new KSquaresGame());
    QVERIFY(KSquaresIO::loadGame(QStringLiteral(TESTBOARDPATH) + QObject::tr("/berlekamp-3.10.dbl"), sGame.data(), &lines));
    for (int i = 0; i < lines.size(); i++) {
        bool nextPlayer, boardFilled;
        QList<int> completedSquares;
        sGame->board()->addLine(lines[i], &nextPlayer, &boardFilled, &completedSquares);
    }

    QList<int> expectedLines;
    expectedLines.append(11);
    executeAi(sGame->board(), sGame->board()->currentPlayer(), QObject::tr("berlekamp-10"), expectedLines);
}

/**
 * test based on berlekamps book "sophisticated child's play"
 * page 17 & 18
 */
void berlekamp::testBerlekamp011() {
    QList<int> lines;
    QScopedPointer<KSquaresGame> sGame(new KSquaresGame());
    QVERIFY(KSquaresIO::loadGame(QStringLiteral(TESTBOARDPATH) + QObject::tr("/berlekamp-3.11.dbl"), sGame.data(), &lines));
    for (int i = 0; i < lines.size(); i++) {
        bool nextPlayer, boardFilled;
        QList<int> completedSquares;
        sGame->board()->addLine(lines[i], &nextPlayer, &boardFilled, &completedSquares);
    }

    QList<int> expectedLines;
    expectedLines.append(2);
    expectedLines.append(20);
    expectedLines.append(21);
    executeAi(sGame->board(), sGame->board()->currentPlayer(), QObject::tr("berlekamp-11"), expectedLines);
}

/**
 * test based on berlekamps book "sophisticated child's play"
 * page 19 & 20
 */
void berlekamp::testBerlekamp012() {
    QList<int> lines;
    QScopedPointer<KSquaresGame> sGame(new KSquaresGame());
    QVERIFY(KSquaresIO::loadGame(QStringLiteral(TESTBOARDPATH) + QObject::tr("/berlekamp-3.12.dbl"), sGame.data(), &lines));
    for (int i = 0; i < lines.size(); i++) {
        bool nextPlayer, boardFilled;
        QList<int> completedSquares;
        sGame->board()->addLine(lines[i], &nextPlayer, &boardFilled, &completedSquares);
    }

    QList<int> expectedLines;
    expectedLines.append(15);
    executeAi(sGame->board(), sGame->board()->currentPlayer(), QObject::tr("berlekamp-12"), expectedLines);
}

/**
 * test based on berlekamps book "sophisticated child's play"
 * page 19 & 20
 */
void berlekamp::testBerlekamp013() {
    QList<int> lines;
    QScopedPointer<KSquaresGame> sGame(new KSquaresGame());
    QVERIFY(KSquaresIO::loadGame(QStringLiteral(TESTBOARDPATH) + QObject::tr("/berlekamp-3.13.dbl"), sGame.data(), &lines));
    for (int i = 0; i < lines.size(); i++) {
        bool nextPlayer, boardFilled;
        QList<int> completedSquares;
        sGame->board()->addLine(lines[i], &nextPlayer, &boardFilled, &completedSquares);
    }

    QList<int> expectedLines;
    expectedLines.append(17);
    executeAi(sGame->board(), sGame->board()->currentPlayer(), QObject::tr("berlekamp-13"), expectedLines);
}

//QTEST_MAIN(berlekamp)


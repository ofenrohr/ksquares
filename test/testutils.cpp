//
// Created by ofenrohr on 20.05.18.
//

#include <QtCore/QFile>
#include <aifunctions.h>
#include <aicontroller.h>
#include "testutils.h"

void testutils::executeAi(QList<int> testAIs, Board *board, int player, std::string name, QList<int> expectedLines) {
    // open the file
    QString qname = QString::fromStdString(name);
    std::string filename = "test-" + name + ".log";
    QString qfilename = QString::fromStdString(filename);
    QFile file(qfilename);
    if (!file.open(QIODevice::ReadWrite | QIODevice::Truncate)) {
        qDebug() << "error: Can't open file";
        return;
    }

    QString summaryStr;
    QTextStream summary(&summaryStr);

    summary << "Summary for " << qname << ": \n";
    summary << "input board:" << board->toString() << "\n\n";

    /*
    Board tmp(board->getNumOfPlayers(), board->width(), board->height());
    for (const auto &line : expectedLines) {
        bool next, filled;
        QList<int> completed;
        tmp.addLine(line, &next, &filled, &completed);
    }
     */
    summary << "expected output:" << aiFunctions::linelistToString(expectedLines, aiFunctions::toLinesSize(board->width(), board->height()), board->width(), board->height()) << "\n\n";

    //for (int i = 0; i <= 2; i++) {
    for (int i : testAIs) {
        aiController aic(player, 1, board->width(), board->height(), i);
        KSquaresAi::Ptr ai = aic.getAi();
        if (!ai->enabled())
            continue;
        else
            qDebug() << "AI " << ai->getName() << " is enabled";
        qDebug() << "current player: " << board->currentPlayer() << ", player: " << player;
        int aiLine = ai->chooseLine(board->lines(), board->squares(), board->getLineHistory());
        if (expectedLines.contains(aiLine)) {
            summary << "PASS " << qname << ": " << ai->getName() << "\n";
        } else {
            summary << "FAIL " << qname << ": " << ai->getName() << ", expected: ";
            for (int j = 0; j < expectedLines.size(); j++)
                summary << QString::number(expectedLines[j]) << (j != expectedLines.size() - 1 ? "," : "");
            summary << ", returned: " << aiLine << ":\n";
            summary  << aiFunctions::linelistToString(QList<int>() << aiLine, aiFunctions::toLinesSize(board->width(), board->height()), board->width(), board->height()) << "\n\n";
            summary << "\n";
        }
    }
    qDebug().noquote().nospace() << summaryStr;

    QTextStream summaryFile(&file);
    summaryFile << summaryStr;
    file.close();
    qDebug() << "summary written to: " << qfilename;
}
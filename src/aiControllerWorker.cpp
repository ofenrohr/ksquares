//
// Created by ofenrohr on 24.07.18.
//

#include "aiControllerWorker.h"

aiControllerWorker::aiControllerWorker(aiController::Ptr aic, const QList<bool> &newLines,
                                       const QList<int> &newSquareOwners,const QList<Board::Move> &newLineHistory) {
    aicontroller = aic;
    lines = newLines;
    squares = newSquareOwners;
    lineHistory = newLineHistory;
}

aiControllerWorker::~aiControllerWorker() {
}


void aiControllerWorker::process() {
    int line = aicontroller->chooseLine(lines, squares, lineHistory);
    emit lineChosen(line);
    emit finished();
}


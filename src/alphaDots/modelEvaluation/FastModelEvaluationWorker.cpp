//
// Created by ofenrohr on 30.03.18.
//

#include <aiBoard.h>
#include <aicontroller.h>
#include <QtCore/QCoreApplication>
#include "FastModelEvaluationWorker.h"

using namespace AlphaDots;

FastModelEvaluationWorker::FastModelEvaluationWorker(AITestSetupManager *testSetupManager,
                                                     TestResultModel *testResultModel, int thread) :
    setupManager(testSetupManager),
    resultModel(testResultModel),
    threadID(thread)
{
    qDebug() << "[FastModelEvaluationWorker] init";
}

FastModelEvaluationWorker::~FastModelEvaluationWorker() {

}

void FastModelEvaluationWorker::process() {
    //qDebug() << "[FastModelEvaluationWorker] starting actual model evaluation";
    do {
        QCoreApplication::processEvents();
        // get setup
        bool ok = false;
        AITestSetup setup = setupManager->popSetup(&ok);
        if (!ok) {
            emit(finished(threadID));
            return;
        }

        qDebug() << "[FastModelEvaluationWorker] starting match: " << setup.aiLevelP1 << "vs." << setup.aiLevelP2;

        // execute setup
        int width = setup.boardSize.x();
        int height = setup.boardSize.y();
        Board board(2, width, height);
        aiController::Ptr aic0(new aiController(0, 1, width, height, setup.modelAiP1, setup.timeout, setup.modelNameP1));
        aiController::Ptr aic1(new aiController(1, 1, width, height, setup.modelAiP2, setup.timeout, setup.modelNameP2));

        //qDebug() << "board info: " << board.lines().size() << ":" << board.lines();

        bool nextPlayer = false;
        bool boardFilled = false;
        QList<int> completedSquares;
        QList<int> moveTimesP1;
        QList<int> moveTimesP2;
        while (!boardFilled) {
            int line = -1;
            if (board.currentPlayer() == 0) {
                line = aic0->chooseLine(board.lines(), board.squares(), board.getLineHistory());
                moveTimesP1.append(aic0->lastMoveTime());
            } else {
                line = aic1->chooseLine(board.lines(), board.squares(), board.getLineHistory());
                moveTimesP2.append(aic1->lastMoveTime());
            }
            completedSquares.clear();
            if (!board.addLine(line, &nextPlayer, &boardFilled, &completedSquares)) {
                qDebug() << "ERROR: failed to add line" << line;
                qDebug().nospace().noquote() << board.toString();
            }
        }
        //qDebug() << "finished game";

        // save result
        AITestResult result;
        result.setup = setup;
        result.scoreP1 = board.squares().count(0);
        result.scoreP2 = board.squares().count(1);
        result.taintedP1 = aic0->getAi()->tainted();
        result.taintedP2 = aic1->getAi()->tainted();
        result.crashesP1 = aic0->getAi()->crashCount();
        result.crashesP2 = aic1->getAi()->crashCount();
        QList<int> moves;
        foreach (Board::Move move, board.getLineHistory()) {
            moves.append(move.line);
        }
        result.moves = moves;
        result.timeP1 = moveTimesP1;
        result.timeP2 = moveTimesP2;

        resultModel->addResult(result);
        //qDebug() << "saved result";

    } while (true);
}

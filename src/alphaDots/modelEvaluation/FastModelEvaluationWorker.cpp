//
// Created by ofenrohr on 30.03.18.
//

#include <aiBoard.h>
#include <aicontroller.h>
#include <QtCore/QCoreApplication>
#include <alphaDots/AlphaDotsExceptions.h>
#include "FastModelEvaluationWorker.h"

using namespace AlphaDots;

FastModelEvaluationWorker::FastModelEvaluationWorker(AITestSetupManager *testSetupManager,
                                                     TestResultModel *testResultModel, int thread,
                                                     QList<ModelInfo> *modelList, QList<ModelInfo> *opponentModelList,
                                                     bool doQuickStart) :
    setupManager(testSetupManager),
    resultModel(testResultModel),
    threadID(thread),
    models(modelList),
    opponentModels(opponentModelList),
    quickStart(doQuickStart)
{
    qDebug() << "[FastModelEvaluationWorker] init";
}

FastModelEvaluationWorker::~FastModelEvaluationWorker() = default;

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

        QString p1l = setup.aiLevelP1 < 0 ? opponentModels->at(-setup.aiLevelP1 -1).ai() : models->at(setup.aiLevelP1 -1).ai();
        QString p2l = setup.aiLevelP2 < 0 ? opponentModels->at(-setup.aiLevelP2 -1).ai() : models->at(setup.aiLevelP2 -1).ai();
        ok = false;
        int p1 = aiFunctions::parseAiLevel(p1l, &ok);
        int p2 = aiFunctions::parseAiLevel(p2l, &ok);

        // execute setup
        int width = setup.boardSize.x();
        int height = setup.boardSize.y();
        Board board(2, width, height);
        aiController::Ptr aic0(new aiController(0, 1, width, height, p1, setup.timeout, setup.modelNameP1));
        aiController::Ptr aic1(new aiController(1, 1, width, height, p2, setup.timeout, setup.modelNameP2));

        //qDebug() << "board info: " << board.lines().size() << ":" << board.lines();
        QList<int> lines = aiController::autoFill(12, width, height);


        bool error = false;
        bool nextPlayer = false;
        bool boardFilled = false;
        QList<int> completedSquares;
        QList<int> moveTimesP1;
        QList<int> moveTimesP2;

        if (quickStart) {
            for (const auto line : lines) {
                completedSquares.clear();
                board.addLine(line, &nextPlayer, &boardFilled, &completedSquares);
                assert(nextPlayer);
                assert(!boardFilled);
                assert(completedSquares.empty());
            }
        }
        while (!boardFilled) {
            int line;
            try {
                if (board.currentPlayer() == 0) {
                    line = aic0->chooseLine(board.lines(), board.squares(), board.getLineHistory());
                    moveTimesP1.append(aic0->lastMoveTime());
                } else {
                    line = aic1->chooseLine(board.lines(), board.squares(), board.getLineHistory());
                    moveTimesP2.append(aic1->lastMoveTime());
                }
            } catch (InternalAiException &ex) {
                qDebug() << "ERROR: InternalAiException" << ex.what();
                qDebug().nospace().noquote() << board.toString();
                error = true;
                break;
            }
            completedSquares.clear();
            if (!board.addLine(line, &nextPlayer, &boardFilled, &completedSquares)) {
                qDebug() << "ERROR: failed to add line" << line;
                qDebug().nospace().noquote() << board.toString();
                error = true;
                break;
            }
        }
        //qDebug() << "finished game";

        // save result
        AITestResult result;
        result.setup = setup;
        result.nameP1 = aic0->getAi()->getName();
        result.nameP2 = aic1->getAi()->getName();
        result.scoreP1 = board.squares().count(0);
        result.scoreP2 = board.squares().count(1);
        result.taintedP1 = aic0->getAi()->tainted() || error;
        result.taintedP2 = aic1->getAi()->tainted() || error;
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

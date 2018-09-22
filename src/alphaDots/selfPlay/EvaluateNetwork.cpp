//
// Created by ofenrohr on 16.09.18.
//

#include <alphaDots/modelEvaluation/ModelEvaluation.h>
#include "EvaluateNetwork.h"


using namespace AlphaDots;

EvaluateNetwork::EvaluateNetwork(const AlphaDots::ModelInfo &initialModel, const int games, const int threads) :
    contendingModel(),
    gamesPerAi(games),
    threadCnt(threads)
{
    bestModel = initialModel;
    resultModel = new TestResultModel(this, &modelList, &opponentModelList, gamesPerAi);
    fastModelEvaluation = new FastModelEvaluation(threadCnt);
    connect(fastModelEvaluation, SIGNAL(evaluationFinished()), this, SLOT(fastModelEvaluationFinished()));
}

EvaluateNetwork::~EvaluateNetwork() {
    resultModel->deleteLater();
    fastModelEvaluation->deleteLater();
}

void EvaluateNetwork::startEvaluation(const AlphaDots::ModelInfo &newModel) {
    qDebug() << "[EvaluateNetwork] starting fast model evaluation";

    contendingModel = newModel;

    modelList.clear();
    modelList.append(contendingModel);
    opponentModelList.clear();
    opponentModelList.append(bestModel);

    ModelEvaluation::createTestSetups(testSetups, QPoint(5,5), 5000, modelList, opponentModelList, gamesPerAi);

    resultModel->reset(&modelList, &opponentModelList, gamesPerAi);

    fastModelEvaluation->startEvaluation(&testSetups, resultModel, &modelList, &opponentModelList);
    emit(infoChanged());
}

void EvaluateNetwork::fastModelEvaluationFinished() {
    int games = resultModel->rawData(0,0);
    int winsByContender = resultModel->rawData(0,1);

    qDebug() << "[EvaluateNetwork] fast model evaluation finished, games: " << games << ", wins by contending model: "
             << winsByContender;

    if (winsByContender > games / 2) {
        qDebug() << "[EvaluateNetwork] contending network won at least 50% of games -> it becomes the new best network";
        bestModel = contendingModel;
    } else {
        qDebug() << "[EvaluateNetwork] contending network won less than 50% of games -> best network stays the same";
    }
    emit infoChanged();
    emit evaluationFinished();
}

const ModelInfo &AlphaDots::EvaluateNetwork::getBestModel() const {
    return bestModel;
}

const ModelInfo &AlphaDots::EvaluateNetwork::getContendingModel() const {
    return contendingModel;
}

TestResultModel *AlphaDots::EvaluateNetwork::getResultModel() const {
    return resultModel;
}


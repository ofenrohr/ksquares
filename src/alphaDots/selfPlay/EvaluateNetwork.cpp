//
// Created by ofenrohr on 16.09.18.
//

#include "EvaluateNetwork.h"

using namespace AlphaDots;

EvaluateNetwork::EvaluateNetwork(const AlphaDots::ModelInfo &initialModel) :
    contendingModel()
{
    resultModel = nullptr;
    bestModel = initialModel;
}

void EvaluateNetwork::startEvaluation(int games, const AlphaDots::ModelInfo &newModel) {
    contendingModel = newModel;
    bestModel = newModel;
    emit evaluationFinished();
}

const ModelInfo &AlphaDots::EvaluateNetwork::getBestModel() const {
    return bestModel;
}

const ModelInfo &AlphaDots::EvaluateNetwork::getContendingModel() const {
    return contendingModel;
}

const TestResultModel *AlphaDots::EvaluateNetwork::getResultModel() const {
    return resultModel;
}


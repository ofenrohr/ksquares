//
// Created by ofenrohr on 16.09.18.
//

#include "EvaluateNetwork.h"

AlphaDots::EvaluateNetwork::EvaluateNetwork(const AlphaDots::ModelInfo &initialModel) {
    resultModel = nullptr;
    bestModel = initialModel;
}

void AlphaDots::EvaluateNetwork::startEvaluation(int games, const AlphaDots::ModelInfo &newModel) {

}

const AlphaDots::ModelInfo &AlphaDots::EvaluateNetwork::getBestModel() const {
    return bestModel;
}

const AlphaDots::TestResultModel *AlphaDots::EvaluateNetwork::getResultModel() const {
    return resultModel;
}


//
// Created by ofenrohr on 16.09.18.
//

#include <alphaDots/modelEvaluation/ModelEvaluation.h>
#include "EvaluateNetwork.h"

#include <QFile>

using namespace AlphaDots;

EvaluateNetwork::EvaluateNetwork(const AlphaDots::ModelInfo &initialModel, const int games, const int threads,
        const bool doQuickStart) :
    contendingModel(),
    gamesPerAi(games),
    threadCnt(threads),
    quickStart(doQuickStart)
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

    fastModelEvaluation->startEvaluation(&testSetups, resultModel, &modelList, &opponentModelList, quickStart);
    startTime = QDateTime::currentDateTime();
    emit(infoChanged());
}

void EvaluateNetwork::fastModelEvaluationFinished() {
    endTime = QDateTime::currentDateTime();
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

QString &EvaluateNetwork::saveResults() {
    QString datetime = QDateTime::currentDateTime().toString(QObject::tr("yyyy-MM-dd_hh-mm-ss"));
    resultPath = "ModelEvaluationReport-" + datetime + ".md";

    QFile outputFile(resultPath);
    if (!outputFile.open(QIODevice::ReadWrite)) {
        qDebug() << "failed to open output file!";
        return resultPath;
    }

    QTextStream outputStream(&outputFile);
    ModelEvaluation::writeResultsToStream(outputStream, startTime, endTime, resultModel, threadCnt, false, false, true);

    return resultPath;
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


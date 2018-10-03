//
// Created by ofenrohr on 16.09.18.
//

#include <alphaDots/modelEvaluation/ModelEvaluation.h>
#include "EvaluateNetwork.h"

#include <QFile>

using namespace AlphaDots;

EvaluateNetwork::EvaluateNetwork(const AlphaDots::ModelInfo &initialModel, const int games, const int threads,
        const bool doQuickStart, QPoint &evalBoardSize) :
    contendingModel(),
    gamesPerAi(games),
    threadCnt(threads),
    quickStart(doQuickStart)
{
    bestModel = initialModel;
    boardSize = evalBoardSize;
    resultModel = new TestResultModel(this, &modelList, &opponentModelList, gamesPerAi);
    fastModelEvaluation = new FastModelEvaluation(threadCnt);
    connect(fastModelEvaluation, SIGNAL(evaluationFinished()), this, SLOT(fastModelEvaluationFinished()));
}

EvaluateNetwork::~EvaluateNetwork() {
    resultModel->deleteLater();
    fastModelEvaluation->deleteLater();
}

void EvaluateNetwork::startEvaluation(const AlphaDots::ModelInfo &newModel, const QPoint &evalBoardSize) {
    qDebug() << "[EvaluateNetwork] starting fast model evaluation";

    contendingModel = newModel;
    boardSize = evalBoardSize;

    modelList.clear();
    modelList.append(contendingModel);
    opponentModelList.clear();
    opponentModelList.append(bestModel);

    ModelEvaluation::createTestSetups(testSetups, boardSize, 5000, modelList, opponentModelList, gamesPerAi);

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

    if (winsByContender >= games / 2) {
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

    ReportLogger::Ptr report = ReportLogger::Ptr(new ReportLogger(resultPath));
    ModelEvaluation::writeResultsToReport(report, startTime, endTime, resultModel, threadCnt, false, modelList,
            opponentModelList, false, true);

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


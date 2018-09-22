//
// Created by ofenrohr on 16.09.18.
//

#ifndef KSQUARES_EVALUATENETWORK_H
#define KSQUARES_EVALUATENETWORK_H

#include <QObject>
#include <alphaDots/ModelInfo.h>
#include <alphaDots/modelEvaluation/TestResultModel.h>
#include <alphaDots/modelEvaluation/FastModelEvaluation.h>

namespace AlphaDots {
    class EvaluateNetwork : public QObject {
    Q_OBJECT
    public:
        EvaluateNetwork(const ModelInfo &initialModel, int games, int threads);
        ~EvaluateNetwork();

        const ModelInfo &getBestModel() const;
        const ModelInfo &getContendingModel() const;
        TestResultModel *getResultModel() const;

    public slots:
        void startEvaluation(const ModelInfo &newModel);
        void fastModelEvaluationFinished();

    signals:
        void infoChanged();
        void evaluationFinished();

    private:
        ModelInfo bestModel;
        ModelInfo contendingModel;
        TestResultModel *resultModel;
        QList<ModelInfo> modelList;
        QList<ModelInfo> opponentModelList;
        int gamesPerAi;
        QList<AITestSetup> testSetups;
        FastModelEvaluation *fastModelEvaluation;
        int threadCnt;
    };
}

#endif //KSQUARES_EVALUATENETWORK_H

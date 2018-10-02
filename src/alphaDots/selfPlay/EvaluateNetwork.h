//
// Created by ofenrohr on 16.09.18.
//

#ifndef KSQUARES_EVALUATENETWORK_H
#define KSQUARES_EVALUATENETWORK_H

#include <QObject>
#include <QDateTime>
#include <alphaDots/ModelInfo.h>
#include <alphaDots/modelEvaluation/TestResultModel.h>
#include <alphaDots/modelEvaluation/FastModelEvaluation.h>

namespace AlphaDots {
    class EvaluateNetwork : public QObject {
    Q_OBJECT
    public:
        EvaluateNetwork(const ModelInfo &initialModel, int games, int threads, bool doQuickStart, QPoint &boardSize);
        ~EvaluateNetwork();

        const ModelInfo &getBestModel() const;
        const ModelInfo &getContendingModel() const;
        TestResultModel *getResultModel() const;
        QString &saveResults();
        int getGames() const {return gamesPerAi;}

    public slots:
        void startEvaluation(const ModelInfo &newModel, const QPoint &boardSize);
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
        bool quickStart;
        QDateTime startTime;
        QDateTime endTime;
        QString resultPath;
        QPoint boardSize;
    };
}

#endif //KSQUARES_EVALUATENETWORK_H

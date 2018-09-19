//
// Created by ofenrohr on 16.09.18.
//

#ifndef KSQUARES_EVALUATENETWORK_H
#define KSQUARES_EVALUATENETWORK_H

#include <QObject>
#include <alphaDots/ModelInfo.h>
#include <alphaDots/modelEvaluation/TestResultModel.h>

namespace AlphaDots {
    class EvaluateNetwork : public QObject {
    Q_OBJECT
    public:
        EvaluateNetwork(const ModelInfo &initialModel);

        const ModelInfo &getBestModel() const;
        const ModelInfo &getContendingModel() const;
        const TestResultModel *getResultModel() const;

    public slots:
        void startEvaluation(int games, const ModelInfo &newModel);

    signals:
        void infoChanged();
        void evaluationFinished();

    private:
        ModelInfo bestModel;
        ModelInfo contendingModel;
        TestResultModel *resultModel;
    };
}

#endif //KSQUARES_EVALUATENETWORK_H

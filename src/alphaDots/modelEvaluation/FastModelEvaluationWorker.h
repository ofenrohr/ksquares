//
// Created by ofenrohr on 30.03.18.
//

#ifndef KSQUARES_FASTMODELEVALUATIONWORKER_H
#define KSQUARES_FASTMODELEVALUATIONWORKER_H


#include <QtCore/QObject>
#include "AITestSetup.h"
#include "AITestSetupManager.h"
#include "TestResultModel.h"

namespace AlphaDots {
    class FastModelEvaluationWorker : public QObject {
    Q_OBJECT
    public:
        FastModelEvaluationWorker(AITestSetupManager *testSetupManager, TestResultModel *testResultModel, int thread);
        ~FastModelEvaluationWorker();

    public slots:
        void process();

    signals:
        void finished(int threadID);

    private:
        AITestSetupManager *setupManager;
        TestResultModel *resultModel;
        int threadID;
    };
}

#endif //KSQUARES_FASTMODELEVALUATIONWORKER_H

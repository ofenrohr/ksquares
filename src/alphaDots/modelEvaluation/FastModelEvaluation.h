//
// Created by ofenrohr on 30.03.18.
//

#ifndef KSQUARES_FASTMODELEVALUATION_H
#define KSQUARES_FASTMODELEVALUATION_H


#include <QObject>
#include "AITestSetup.h"
#include "TestResultModel.h"
#include "AITestSetupManager.h"

namespace AlphaDots {
    class FastModelEvaluation : public QObject {
    Q_OBJECT
    public:
        FastModelEvaluation(int threads);

        ~FastModelEvaluation();

        void startEvaluation(QList<AITestSetup> *testSetups, TestResultModel *resultModel);
    private:
        AITestSetupManager *setupManager;
        int threadCnt;
    };
}

#endif //KSQUARES_FASTMODELEVALUATION_H

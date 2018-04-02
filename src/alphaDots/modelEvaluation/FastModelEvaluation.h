//
// Created by ofenrohr on 30.03.18.
//

#ifndef KSQUARES_FASTMODELEVALUATION_H
#define KSQUARES_FASTMODELEVALUATION_H


#include <QObject>
#include "AITestSetup.h"
#include "TestResultModel.h"

namespace AlphaDots {
    class FastModelEvaluation : public QObject {
    Q_OBJECT
    public:
        FastModelEvaluation();

        ~FastModelEvaluation() = default;

        void startEvaluation(QList<AITestSetup> testSetups, TestResultModel *resultModel);
    };
}

#endif //KSQUARES_FASTMODELEVALUATION_H

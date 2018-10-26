//
// Created by ofenrohr on 04.04.18.
//

#ifndef KSQUARES_GAMEPLAYANALYSIS_H
#define KSQUARES_GAMEPLAYANALYSIS_H


#include <QList>
#include <QVariant>
#include <alphaDots/modelEvaluation/AITestResult.h>

namespace AlphaDots {
    class GameplayAnalysis {
    public:
        ~GameplayAnalysis() = default;

        virtual QList<QVariant> analyseResult(AITestResult &result, bool *ok) = 0;

        virtual int headerCount() = 0;

        virtual QList<QString> headers() = 0;

        virtual QString moduleName() = 0;
    };
}


#endif //KSQUARES_GAMEPLAYANALYSIS_H

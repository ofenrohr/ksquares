//
// Created by ofenrohr on 22.10.18.
//

#ifndef KSQUARES_DOUBLEDEALINGANALYSIS_H
#define KSQUARES_DOUBLEDEALINGANALYSIS_H


#include "GameplayAnalysis.h"

class DoubleDealingAnalysis : public GameplayAnalysis {
public:
    DoubleDealingAnalysis();
    ~DoubleDealingAnalysis();

    QList<QVariant> analyseResult(AITestResult &result, bool *ok) override;

    int headerCount() override;

    QList<QString> headers() override;

    QString moduleName() override;
private:
    QList<QString> _headers;
};


#endif //KSQUARES_DOUBLEDEALINGANALYSIS_H

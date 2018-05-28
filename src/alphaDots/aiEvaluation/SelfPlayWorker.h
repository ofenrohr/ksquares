//
// Created by ofenrohr on 21.05.18.
//

#ifndef KSQUARES_SELFPLAYWORKER_H
#define KSQUARES_SELFPLAYWORKER_H


#include <QtCore/QObject>
#include <alphaDots/ModelInfo.h>
#include <alphaDots/datasets/DatasetGenerator.h>

namespace AlphaDots {
    class SelfPlayWorker : public QObject {
    Q_OBJECT
    public:
        SelfPlayWorker(DatasetGenerator *generator,
                       int threadIndex,
                       int samples,
                       ModelInfo &modelInfo,
                       std::vector<uint8_t> *inputData,
                       std::vector<uint8_t> *outputData,
                       std::vector<double> *valueData);

    public slots:
        void process();

    signals:
        void progress(int samplesGenerated, int threadID);
        void finished(int threadID);

    private:
        DatasetGenerator *datasetGen;
        int threadIdx;
        int sampleCnt;
        int samplesGenerated;
        ModelInfo model;
        std::vector<uint8_t> *input;
        std::vector<uint8_t> *output;
        std::vector<double> *value;
    };
}


#endif //KSQUARES_SELFPLAYWORKER_H

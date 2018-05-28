//
// Created by ofenrohr on 21.05.18.
//

#include <QtCore/QCoreApplication>
#include "SelfPlayWorker.h"

AlphaDots::SelfPlayWorker::SelfPlayWorker(DatasetGenerator *generator, int threadIndex, int samples, AlphaDots::ModelInfo &modelInfo,
                                          std::vector<uint8_t> *inputData, std::vector<uint8_t> *outputData,
                                          std::vector<double> *valueData) {
    datasetGen = generator;
    threadIdx = threadIndex;
    sampleCnt = samples;
    samplesGenerated = 0;
    model = modelInfo;
    input = inputData;
    output = outputData;
    value = valueData;
}

void AlphaDots::SelfPlayWorker::process() {
    for (int i = 0; i < sampleCnt; i++) {
        datasetGen->generateDataset();

        samplesGenerated++;
        emit progress(samplesGenerated, threadIdx);
        QCoreApplication::processEvents();
    }
    emit finished(threadIdx);
}

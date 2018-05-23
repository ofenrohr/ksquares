//
// Created by ofenrohr on 21.05.18.
//

#include "SelfPlayWorker.h"

AlphaDots::SelfPlayWorker::SelfPlayWorker(int threadIndex, int samples, AlphaDots::ModelInfo &modelInfo,
                                          std::vector<uint8_t> *inputData, std::vector<uint8_t> *outputData,
                                          std::vector<double> *valueData) {
    threadIdx = threadIndex;
    sampleCnt = samples;
    model = modelInfo;
    input = inputData;
    output = outputData;
    value = valueData;
}

void AlphaDots::SelfPlayWorker::process() {

}

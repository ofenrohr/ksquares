//
// Created by ofenrohr on 19.04.18.
//

#ifndef KSQUARES_STAGETHREE_H
#define KSQUARES_STAGETHREE_H

#include "DatasetGenerator.h"

namespace AlphaDots {
    class StageThreeDataset : public DatasetGenerator {
    public:
        typedef QSharedPointer<StageThreeDataset> Ptr;

        StageThreeDataset(bool gui, int w, int h, int thread=-1, int threads=-1);

        ~StageThreeDataset();

        Dataset generateDataset() override;

        void startConverter(int samples, QString destinationDirectory) override;
        void stopConverter() override;

        bool isLocalNPZ() override {return true;}
        std::vector<uint8_t>* getInputData() override { return input; }
        std::vector<uint8_t>* getOutputData() override { return output; }

        void setInputData(std::vector<uint8_t>* x) override {input = x;}
        void setOutputData(std::vector<uint8_t>* x) override {output = x;}

        void cleanup() override;

    protected:
        int width;
        int height;

        bool isGUI;
        int sampleIdx;

        int threadIdx;
        int threadCnt;
        int sampleCnt;
        QString destDir;
        std::vector<size_t> dataSize;
        std::vector<uint8_t> *input;
        std::vector<uint8_t> *output;
    };
}


#endif //KSQUARES_STAGETHREE_H

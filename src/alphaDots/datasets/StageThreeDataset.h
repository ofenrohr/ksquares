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
        bool stopConverter() override;

        bool isLocalNPZ() override {return true;}
        std::vector<uint8_t>* getInputData() override { return input; }
        std::vector<uint8_t>* getPolicyData() override { return policy; }
        std::vector<double>* getValueData() override { return value; }

        void setInputData(std::vector<uint8_t>* x) override {input = x;}
        void setPolicyData(std::vector<uint8_t> *x) override {policy = x;}
        void setValueData(std::vector<double> *x) override {value = x;}

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
        std::vector<size_t> valueDataSize;
        std::vector<uint8_t> *input;
        std::vector<uint8_t> *policy;
        std::vector<double> *value;
    };
}


#endif //KSQUARES_STAGETHREE_H

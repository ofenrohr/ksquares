//
// Created by ofenrohr on 22.05.18.
//

#ifndef KSQUARES_STAGEFOURDATASET_H
#define KSQUARES_STAGEFOURDATASET_H


#include <alphaDots/ModelInfo.h>
#include <gsl/gsl_randist.h>
#include "DatasetGenerator.h"

namespace AlphaDots {
    class StageFourDataset : public DatasetGenerator {
    public:
        typedef QSharedPointer<StageFourDataset> Ptr;

        StageFourDataset(bool gui, int w, int h, QString modelName, int thread=-1, int threads=-1, bool useMCTS=true,
                int fastAiLevel=2);

        ~StageFourDataset();

        Dataset generateDataset() override;

        void startConverter(int samples, QString destinationDirectory) override;
        void startConverter(int samples, QString destinationDirectory, bool createData);
        bool stopConverter() override;

        bool isLocalNPZ() override {return true;}
        std::vector<uint8_t>* getInputData() override { return input; }
        std::vector<uint8_t>* getPolicyData() override { return policy; }
        std::vector<double>* getValueData() override { return value; }

        void setInputData(std::vector<uint8_t>* x) override {input = x;}
        void setPolicyData(std::vector<uint8_t> *x) override {policy = x;}
        void setValueData(std::vector<double> *x) override {value = x;}

        void cleanup() override;


        // StageFourDataset specific stuff:
        QString getDatasetPath() {return datasetPath;}

        // hyperparameters
        static double sigmaScale;// = 0.2;
        static double meanScale;// = 0.6;

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
        ModelInfo model;
        gsl_rng *rng;
        QString datasetPath;
        bool createdData;
        bool useMCTSai;
        int fastAiLevel;
        int failRecursionLevel;
    };
}

#endif //KSQUARES_STAGEFOURDATASET_H

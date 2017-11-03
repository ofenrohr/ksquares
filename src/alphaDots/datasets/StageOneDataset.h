//
// Created by ofenrohr on 30.10.17.
//

#ifndef KSQUARES_STAGEONEDATASET_H
#define KSQUARES_STAGEONEDATASET_H


#include "DatasetGenerator.h"
#include <zmq.hpp>

namespace AlphaDots {
    class StageOneDataset : public DatasetGenerator {
    public:
        typedef QSharedPointer<StageOneDataset> Ptr;

        StageOneDataset(bool gui);

        ~StageOneDataset();

        Dataset generateDataset() override;

        void startConverter(int width, int height, int samples);

        void cleanup() override;

    protected:
        int width;
        int height;

        bool isGUI;
        int sampleIdx;

        ExternalProcess *converter;
    };
}


#endif //KSQUARES_STAGEONEDATASET_H

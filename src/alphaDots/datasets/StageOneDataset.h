//
// Created by ofenrohr on 30.10.17.
//

#ifndef KSQUARES_STAGEONEDATASET_H
#define KSQUARES_STAGEONEDATASET_H


#include "DatasetGenerator.h"
#include <zmq.hpp>

class StageOneDataset : public DatasetGenerator {
public:
    StageOneDataset(int w, int h);
    ~StageOneDataset();
    void generateDataset() override;
protected:
    int width;
    int height;
};


#endif //KSQUARES_STAGEONEDATASET_H

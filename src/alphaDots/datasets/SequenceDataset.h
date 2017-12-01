//
// Created by ofenrohr on 01.12.17.
//

#ifndef KSQUARES_SEQUENCEDATASET_H
#define KSQUARES_SEQUENCEDATASET_H


#include <alphaDots/ExternalProcess.h>
#include "DatasetGenerator.h"

using namespace AlphaDots;
class SequenceDataset : public DatasetGenerator {
public:
    typedef QSharedPointer<SequenceDataset> Ptr;

    SequenceDataset(bool gui, int width, int height);

    ~SequenceDataset();

    Dataset generateDataset() override;

    void startConverter(int samples);

    void cleanup() override;

protected:
    int width;
    int height;
    int linesSize;

    bool isGUI;
    int sampleIdx;

    ExternalProcess *converter;
};


#endif //KSQUARES_SEQUENCEDATASET_H

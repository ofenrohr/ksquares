//
// Created by ofenrohr on 01.12.17.
//

#ifndef KSQUARES_SEQUENCEDATASET_H
#define KSQUARES_SEQUENCEDATASET_H


#include <alphaDots/ExternalProcess.h>
#include <zmq.hpp>
#include "DatasetGenerator.h"

using namespace AlphaDots;
class SequenceDataset : public DatasetGenerator {
public:
    typedef QSharedPointer<SequenceDataset> Ptr;

    SequenceDataset(bool gui, int width, int height);

    ~SequenceDataset();

    Dataset generateDataset() override;

    void startConverter(int samples, QString destinationDirectory) override;

    void cleanup() override;

protected:
    int width;
    int height;
    int linesSize;

    bool isGUI;
    int sampleIdx;

    ExternalProcess *converter;

    bool connectionReady;
    zmq::context_t* context;
    zmq::socket_t* socket;
    zmq::socket_t* getSocket();
};


#endif //KSQUARES_SEQUENCEDATASET_H

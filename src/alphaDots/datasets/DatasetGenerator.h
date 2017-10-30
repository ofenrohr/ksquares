//
// Created by ofenrohr on 29.10.17.
//

#ifndef KSQUARES_DATASETGENERATOR_H
#define KSQUARES_DATASETGENERATOR_H

#include <QtCore/QSharedPointer>


class DatasetGenerator {
public:
    typedef QSharedPointer<DatasetGenerator> Ptr;
    virtual void generateDataset() {}
};


#endif //KSQUARES_DATASETGENERATOR_H

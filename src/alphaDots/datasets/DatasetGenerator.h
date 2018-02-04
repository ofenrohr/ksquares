//
// Created by ofenrohr on 29.10.17.
//

#ifndef KSQUARES_DATASETGENERATOR_H
#define KSQUARES_DATASETGENERATOR_H

#include <QtCore/QSharedPointer>
#include <aiBoard.h>
#include <QtGui/QImage>
#include "Dataset.h"


namespace AlphaDots {
    class DatasetGenerator : public QObject {
    Q_OBJECT
    public:
        typedef QSharedPointer<DatasetGenerator> Ptr;

        virtual Dataset generateDataset() = 0;

        virtual void startConverter(int examplesCnt, QString destinationDirectory) = 0;

        virtual void cleanup() {}

    signals:

        void sendGUIsample(aiBoard::Ptr board, QImage inp, QImage outp);
    };
}


#endif //KSQUARES_DATASETGENERATOR_H

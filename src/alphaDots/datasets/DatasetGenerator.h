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
        virtual void stopConverter() {};

        virtual void cleanup() {}

        /**
         * Returns true if it's a local dataset generator that doesn't use the python converter but generates
         * the .npz file directly. If it's a local dataset, the startConverter function will allocate the space
         * for the local npz generation.
         * @return true if it doesn't use the python converter
         */
        virtual bool isLocalNPZ() { return false; }

        /**
         * Returns the data that was allocated in startConverter if this is a local npz dataset generator.
         * @return allocated npz data
         */
        virtual std::vector<uint8_t>* getInputData() { return nullptr; }
        virtual std::vector<uint8_t>* getPolicyData() { return nullptr; }
        virtual std::vector<double>* getValueData() { return nullptr; }
        virtual void setInputData(std::vector<uint8_t>* ) {}
        virtual void setPolicyData(std::vector<uint8_t> *) {}
        virtual void setValueData(std::vector<double> *) {}

    signals:

        void sendGUIsample(aiBoard::Ptr board, QImage inp, QImage outp);
    };
}


#endif //KSQUARES_DATASETGENERATOR_H

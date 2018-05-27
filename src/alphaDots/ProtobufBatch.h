//
// Created by ofenrohr on 27.05.18.
//

#ifndef KSQUARES_PROTOBUFBATCH_H
#define KSQUARES_PROTOBUFBATCH_H


#include <QtCore/QObject>
#include <QtCore/QMutex>
#include <PolicyValueData.pb.h>
#include <DotsAndBoxesImage.pb.h>

namespace AlphaDots {
    class ProtobufBatch : public QObject {
    Q_OBJECT
    public:
        ProtobufBatch(int batchSize);

        PolicyValueData predict(DotsAndBoxesImage input);

    private:
        int batchSize;
        QMutex addBatchMutex;
    };
}


#endif //KSQUARES_PROTOBUFBATCH_H

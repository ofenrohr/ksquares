//
// Created by ofenrohr on 28.10.17.
//

#ifndef KSQUARES_PBCONNECTOR_H
#define KSQUARES_PBCONNECTOR_H

#include <QtGui/QImage>
#include <zmq.hpp>
#include <GameSequence.pb.h>
#include <TrainingExample.pb.h>
#include <DotsAndBoxesImage.pb.h>
#include <AlphaDotsModel.pb.h>
#include <ModelList.pb.h>
#include <AlphaZeroMCTSNode.h>
#include <aiBoard.h>
#include <QtCore/QMutex>
#include <PolicyValueData.pb.h>
#include "ModelInfo.h"

namespace AlphaDots {
    class ProtobufConnector {
    public:
        static ProtobufConnector& getInstance();
        ProtobufConnector(ProtobufConnector const &) = delete;
        void operator=(ProtobufConnector const &) = delete;

        /**
         * Put the image data in the protobuf data class
         * @param img input image
         * @return protobuf data
         */
        static DotsAndBoxesImage dotsAndBoxesImageToProtobuf(const QImage &img);
        static void copyDataToProtobuf(DotsAndBoxesImage *pb, const QImage &img);
        static TrainingExample trainingExampleToProtobuf(QImage inp, QImage outp);
        static GameSequence gameSequenceToProtobuf(QList<QImage> seq);
        static GameSequence gameSequenceToProtobuf(QList<QImage> inputSeq, QList<QImage> targetSeq);

        /**
         * Put the protobuf image data back in a QImage
         * @param msg protobuf data
         * @return image data
         */
        static QImage protobufDotsAndBoxesImageToQImage(const DotsAndBoxesImage &img);


        /**
         * Get model list.
         * @return
         */
        QList<ModelInfo> getModelList();
        ModelInfo getModelByName(QString name);

        /**
         * Converts a pixel line position to the line index format used by ksquares
         * @param linePoint the pixel position of the line in the image
         * @param width in boxes
         * @return line index for ksquares
         */
        static int pointToLineIndex(QPoint linePoint, int width);

        static bool sendString(zmq::socket_t &socket, std::string msg);

        static std::string recvString(zmq::socket_t &socket, bool *ok);

        PolicyValueData batchPredict(zmq::socket_t &socket, QImage &img);
        void releaseBatchSample();

        void setBatchSize(int size);

        void setBatchPredict(bool mode);
        bool getBatchPredict();

    private:
        ProtobufConnector();
        zmq::context_t context;
        QList<ModelInfo> cachedModelList;
        QMutex modelListMutex;
        QAtomicInt batchSize;
        QAtomicInt batchCnt;
        DotsAndBoxesImage *firstImgInBatch;
        QList<DotsAndBoxesImage *> requestBatch;
        PolicyValueData *firstPredictionInBatch;
        QList<PolicyValueData *> responseBatch;
        QMutex batchImgMutex;
        QAtomicInt batchPredictionState;
        bool batchPredictMode;
    };
}


#endif //KSQUARES_PBCONNECTOR_H

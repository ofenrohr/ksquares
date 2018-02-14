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
#include "ModelInfo.h"

namespace AlphaDots {
    class ProtobufConnector {
    public:
        /**
         * Put the image data in the protobuf data class
         * @param img input image
         * @return protobuf data
         */
        static DotsAndBoxesImage dotsAndBoxesImageToProtobuf(QImage img);
        static TrainingExample trainingExampleToProtobuf(QImage inp, QImage outp);
        static GameSequence gameSequenceToProtobuf(QList<QImage> seq);
        static GameSequence gameSequenceToProtobuf(QList<QImage> inputSeq, QList<QImage> targetSeq);

        /**
         * Put the protobuf image data back in a QImage
         * @param msg protobuf data
         * @return image data
         */
        static QImage fromProtobuf(std::string msg);

        /**
         * Get model list.
         * @return
         */
        static QList<ModelInfo> getModelList();
        static ModelInfo getModelByName(QString name);

        /**
         * Converts a pixel line position to the line index format used by ksquares
         * @param linePoint the pixel position of the line in the image
         * @param width in boxes
         * @return line index for ksquares
         */
        static int pointToLineIndex(QPoint linePoint, int width);

        static bool sendString(zmq::socket_t &socket, std::string msg);

        static std::string recvString(zmq::socket_t &socket);
    };
}


#endif //KSQUARES_PBCONNECTOR_H

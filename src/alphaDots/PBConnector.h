//
// Created by ofenrohr on 28.10.17.
//

#ifndef KSQUARES_PBCONNECTOR_H
#define KSQUARES_PBCONNECTOR_H

#include <QtGui/QImage>
#include <zmq.hpp>
#include "alphaDots/DotsAndBoxesImage.pb.h"

namespace AlphaDots {
    class PBConnector {
    public:
        /**
         * Put the image data in the protobuf data class
         * @param img input image
         * @return protobuf data
         */
        static DotsAndBoxesImage toProtobuf(QImage img);

        /**
         * Put the protobuf image data back in a QImage
         * @param msg protobuf data
         * @return image data
         */
        static QImage fromProtobuf(std::string msg);

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

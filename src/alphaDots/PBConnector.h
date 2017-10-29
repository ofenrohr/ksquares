//
// Created by ofenrohr on 28.10.17.
//

#ifndef KSQUARES_PBCONNECTOR_H
#define KSQUARES_PBCONNECTOR_H

#include <QtGui/QImage>
#include "alphaDots/DotsAndBoxesImage.pb.h"

class PBConnector {
public:
    static alphaDots::DotsAndBoxesImage toProtobuf(QImage img);
    static QImage fromProtobuf(std::string msg);
};


#endif //KSQUARES_PBCONNECTOR_H

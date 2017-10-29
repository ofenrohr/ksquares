//
// Created by ofenrohr on 28.10.17.
//

#include "PBConnector.h"

#include <QDebug>

alphaDots::DotsAndBoxesImage PBConnector::toProtobuf(QImage img) {
    alphaDots::DotsAndBoxesImage ret;

    ret.set_width(img.width());
    ret.set_height(img.height());
    for (int y = 0; y < img.height(); y++) {
        for (int x = 0; x < img.width(); x++) {
            ret.add_pixels(img.pixelColor(x,y).red());
        }
    }

    return ret;
}

QImage PBConnector::fromProtobuf(std::string msg) {
    alphaDots::DotsAndBoxesImage img;
    img.ParseFromString(msg);
    QImage ret(img.width(), img.height(), QImage::Format_ARGB32);
    for (int i = 0; i < img.width() * img.height(); i++) {
        int c = img.pixels().Get(i);
        int x = i % img.width();
        int y = i / img.width();
        ret.setPixel(x, y, qRgb(c,c,c));
    }
    return ret;
}

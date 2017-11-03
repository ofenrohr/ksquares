//
// Created by ofenrohr on 28.10.17.
//

#include "PBConnector.h"

#include <QDebug>

using namespace AlphaDots;

DotsAndBoxesImage PBConnector::toProtobuf(QImage img) {
    DotsAndBoxesImage ret;

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
    DotsAndBoxesImage img;
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

int PBConnector::pointToLineIndex(QPoint linePoint, int width) {
	int ret;
	if (linePoint.x() % 2 == 0) { // horizontal line
		ret = (linePoint.x() / 2 - 1) + (linePoint.y() / 2) * width + ((linePoint.y() - 1) / 2) * (width + 1);
	} else { // vertical line
		ret = (linePoint.x() / 2) + (linePoint.y() / 2) * width + (linePoint.y()/2 -1) * (width+1);
	}
    return ret;
}

bool PBConnector::sendString(zmq::socket_t &socket, std::string msg) {
	ulong message_size = msg.size();
	zmq::message_t request(message_size);
	memcpy(request.data(), msg.c_str(), message_size);
    try {
        socket.send(request);
    } catch (zmq::error_t &ex) {
        qDebug() << "zmq send error: " << ex.num();
        qDebug() << "msg: " << ex.what();
        return false;
    }
    return true;
}

std::string PBConnector::recvString(zmq::socket_t &socket) {
	zmq::message_t reply;
    bool done = false;
    int tries = 0;
    while (!done && tries < 3) {
        try {
            socket.recv(&reply);
            done = true;
        } catch (zmq::error_t &ex) {
            qDebug() << "zmq recv error: " << ex.num();
            qDebug() << "msg: " << ex.what();
        }
        tries++;
    }
    if (!done) {
        return "";
    }
    std::string rpl = std::string(static_cast<char*>(reply.data()), reply.size());
    return rpl;
}

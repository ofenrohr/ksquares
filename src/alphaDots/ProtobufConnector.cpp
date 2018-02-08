//
// Created by ofenrohr on 28.10.17.
//

#include "ProtobufConnector.h"

#include <QDebug>

using namespace AlphaDots;

DotsAndBoxesImage ProtobufConnector::dotsAndBoxesImageToProtobuf(QImage img) {
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

TrainingExample ProtobufConnector::trainingExampleToProtobuf(QImage inp, QImage outp) {
    TrainingExample ret;

    ret.set_width(inp.width());
    ret.set_height(inp.height());

    for (int y = 0; y < inp.height(); y++) {
        for (int x = 0; x < inp.width(); x++) {
            ret.add_input(inp.pixelColor(x,y).red());
            ret.add_output(outp.pixelColor(x,y).red());
        }
    }

    return ret;
}

GameSequence ProtobufConnector::gameSequenceToProtobuf(QList<QImage> seq) {
    GameSequence ret;

    if (seq.count() <= 0) {
        return ret;
    }

    int w = seq[0].width();
    int h = seq[0].height();

    ret.set_width(w);
    ret.set_height(h);

    for (int i = 0; i < seq.count(); i++) {
        TrainingExample *frame = ret.add_game();
        frame->set_width(w);
        frame->set_height(h);
        for (int y = 0; y < h; y++) {
            for (int x = 0; x < w; x++) {
                frame->add_input(seq[i].pixelColor(x, y).red());
            }
        }
    }
    return ret;
}

GameSequence ProtobufConnector::gameSequenceToProtobuf(QList<QImage> inputSeq, QList<QImage> targetSeq) {
    GameSequence ret;

    if (inputSeq.count() != targetSeq.count() ||
        inputSeq.count() <= 0 ||
        targetSeq.count() <= 0 ||
        inputSeq[0].width() != targetSeq[0].width() ||
        inputSeq[0].height() != targetSeq[0].height()
    ) {
        qDebug() << "WARNING: invalid data!";
        return ret;
    }

    int w = inputSeq[0].width();
    int h = inputSeq[0].height();

    ret.set_width(w);
    ret.set_height(h);

    for (int i = 0; i < inputSeq.count(); i++) {
        TrainingExample *frame = ret.add_game();
        frame->set_width(w);
        frame->set_height(h);
        for (int y = 0; y < h; y++) {
            for (int x = 0; x < w; x++) {
                frame->add_input(inputSeq[i].pixelColor(x, y).red());
                frame->add_output(targetSeq[i].pixelColor(x, y).red());
            }
        }
    }

    return ret;
}

QImage ProtobufConnector::fromProtobuf(std::string msg) {
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

int ProtobufConnector::pointToLineIndex(QPoint linePoint, int width) {
	int ret;
	if (linePoint.x() % 2 == 0) { // horizontal line
		ret = (linePoint.x() / 2 - 1) + (linePoint.y() / 2) * width + ((linePoint.y() - 1) / 2) * (width + 1);
	} else { // vertical line
		ret = (linePoint.x() / 2) + (linePoint.y() / 2) * width + (linePoint.y()/2 -1) * (width+1);
	}
    return ret;
}

bool ProtobufConnector::sendString(zmq::socket_t &socket, std::string msg) {
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

std::string ProtobufConnector::recvString(zmq::socket_t &socket) {
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

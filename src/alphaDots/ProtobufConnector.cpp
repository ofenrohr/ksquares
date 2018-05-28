//
// Created by ofenrohr on 28.10.17.
//

#include "ProtobufConnector.h"
#include "ExternalProcess.h"

#include <QDebug>
#include <settings.h>
#include <AlphaDotsModel.pb.h>
#include <DotsAndBoxesImage.pb.h>
#include <TrainingExample.pb.h>

using namespace AlphaDots;

QList<ModelInfo> ProtobufConnector::cachedModelList;
QMutex ProtobufConnector::modelListMutex;

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

QList<ModelInfo> ProtobufConnector::getModelList(bool locked) {
    if (!locked) {
        modelListMutex.lock();
    }
    if (!cachedModelList.isEmpty()) {
        if (!locked) {
            modelListMutex.unlock();
        }
        return cachedModelList;
    }
    cachedModelList.clear();


    QString process = QStringLiteral("/usr/bin/python2.7");
    QStringList processArgs;
    processArgs << Settings::alphaDotsDir() + QStringLiteral("/modelServer/modelList.py");
    ExternalProcess modelListProc(process, processArgs);
    if (!modelListProc.startExternalProcess()) {
        qDebug() << "ERROR: failed to start " << process << processArgs;
        if (!locked) {
            modelListMutex.unlock();
        }
        return cachedModelList;
    }

    try {
        zmq::context_t context(1);
        zmq::socket_t socket(context, ZMQ_REQ);
        socket.connect("tcp://127.0.0.1:13452");

        sendString(socket, "get");

        bool ok = false;
        std::string response = recvString(socket, &ok);
        if (!ok) {
            qDebug() << "ERROR: failed to receive model list";
            if (!locked) {
                modelListMutex.unlock();
            }
            return cachedModelList;
        }
        ModelList modelList;
        modelList.ParseFromString(response);

        for (int i = 0; i < modelList.models().size(); i++) {
            ProtoModel model = modelList.models().Get(i);
            cachedModelList.append(ModelInfo(QString::fromStdString(model.name()), QString::fromStdString(model.desc()),
                                 QString::fromStdString(model.path()), QString::fromStdString(model.type()), QString::fromStdString(model.ai())));
        }

    } catch (zmq::error_t &err) {
        qDebug() << "ERROR receiving list: " << err.num() << err.what();
    }

    modelListProc.stopExternalProcess();

    if (!locked) {
        modelListMutex.unlock();
    }
    return cachedModelList;
}

ModelInfo ProtobufConnector::getModelByName(QString name) {
    modelListMutex.lock();
    QList<ModelInfo> modelList = getModelList(true);
    for (auto model : modelList) {
        if (model.name() == name) {
            modelListMutex.unlock();
            return model;
        }
    }
    qDebug() << "ERROR: failed to find model with name " << name;
    ModelInfo ret = modelList[0];
    modelListMutex.unlock();
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
        if (!socket.send(request)) {
            qDebug() << "ERROR: failed to send message via zmq" << errno;
        }
    } catch (zmq::error_t &ex) {
        qDebug() << "zmq send error: " << ex.num();
        qDebug() << "msg: " << ex.what();
        return false;
    }
    return true;
}

std::string ProtobufConnector::recvString(zmq::socket_t &socket, bool *ok) {
	zmq::message_t reply;
    *ok = true;
    try {
        if (!socket.recv(&reply)) {
            qDebug() << "ERROR: failed to recv message via zmq" << errno;
            *ok = false;
            return "error";
        }
    } catch (zmq::error_t &ex) {
        qDebug() << "zmq recv error: " << ex.num();
        qDebug() << "msg: " << ex.what();
        *ok = false;
        return "error";
    }
    std::string rpl = std::string(static_cast<char*>(reply.data()), reply.size());
    return rpl;
}

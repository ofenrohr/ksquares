//
// Created by ofenrohr on 28.10.17.
//

#include "ProtobufConnector.h"
#include "ExternalProcess.h"
#include "ModelManager.h"

#include <QDebug>
#include <settings.h>
#include <QtCore/QThread>
#include <QtWidgets/QMessageBox>
#include <QtCore/QDir>

using namespace AlphaDots;

ProtobufConnector &ProtobufConnector::getInstance() {
    static ProtobufConnector instance;
    return instance;
}

ProtobufConnector::ProtobufConnector() :
    modelListMutex(QMutex::Recursive),
    batchImgMutex(QMutex::NonRecursive),
    batchSize(1),
    batchCnt(0),
    batchPredictionState(0),
    batchPredictMode(false)
{
    cachedModelList.clear();
}

DotsAndBoxesImage ProtobufConnector::dotsAndBoxesImageToProtobuf(const QImage &img) {
    DotsAndBoxesImage ret;

    copyDataToProtobuf(&ret, img);

    return ret;
}

void ProtobufConnector::copyDataToProtobuf(DotsAndBoxesImage *pb, const QImage &img) {

    pb->set_width(img.width());
    pb->set_height(img.height());
    bool sizeMatch = pb->pixels_size() == img.height()*img.width();
    if (!sizeMatch) {
        pb->clear_pixels();
    }
    for (int y = 0; y < img.height(); y++) {
        for (int x = 0; x < img.width(); x++) {
            if (sizeMatch) {
                pb->set_pixels(y*img.width()+x, img.pixelColor(x, y).red());
            } else {
                pb->add_pixels(img.pixelColor(x, y).red());
            }
        }
    }

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

QList<ModelInfo> ProtobufConnector::getModelList(bool useLocking) {
    QMutexLocker locker(&modelListMutex);
    if (!cachedModelList.isEmpty()) {
        return cachedModelList;
    }

    QDir dir(Settings::alphaDotsDir());
    if (!dir.exists()) {
        return cachedModelList;
    }

    QString process = Settings::pythonExecutable();//QStringLiteral("/usr/bin/python2.7");
    QStringList processArgs;
    processArgs << Settings::alphaDotsDir() + QObject::tr("/modelServer/modelList.py");
    ExternalProcess modelListProc(process, processArgs);
    if (!modelListProc.startExternalProcess()) {
        qDebug() << "ERROR: failed to start " << process << processArgs;
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

    modelListProc.stopExternalProcess(false, false);

    return cachedModelList;
}

ModelInfo ProtobufConnector::getModelByName(QString name) {
    QMutexLocker locker(&modelListMutex);
    QList<ModelInfo> modelList = getModelList(true);
    for (auto model : modelList) {
        if (model.name() == name) {
            return model;
        }
    }
    qDebug() << "ERROR: failed to find model with name " << name;
    if (!modelList.empty()) {
        return modelList[0];
    }
    return ModelInfo();
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
            QMessageBox::critical(nullptr, QObject::tr("zmq error"), QObject::tr("failed to send message with zmq"));
        }
    } catch (zmq::error_t &ex) {
        qDebug() << "zmq send error: " << ex.num();
        qDebug() << "msg: " << ex.what();
        QMessageBox::critical(nullptr, QObject::tr("zmq error"), QObject::tr("failed to send message with zmq"));
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
            QMessageBox::critical(nullptr, QObject::tr("zmq error"), QObject::tr("failed to recv message with zmq"));
            return "error";
        }
    } catch (zmq::error_t &ex) {
        qDebug() << "zmq recv error: " << ex.num();
        qDebug() << "msg: " << ex.what();
        QMessageBox::critical(nullptr, QObject::tr("zmq error"), QObject::tr("failed to recv message with zmq"));
        *ok = false;
        return "error";
    }
    std::string rpl = std::string(static_cast<char*>(reply.data()), reply.size());
    return rpl;
}

PolicyValueData ProtobufConnector::batchPredict(zmq::socket_t &socket, QImage &inputimg) {
    bool debug = ModelManager::getInstance().getDebug();

    //DotsAndBoxesImage img = ProtobufConnector::dotsAndBoxesImageToProtobuf(inputimg);

    // wait for previous batch to finish...
    while (ProtobufConnector::batchPredictionState != 0) {
        QThread::usleep(10);
    }

    batchImgMutex.lock();
    /*
    if (ProtobufConnector::batchCnt == 0) {
        ProtobufConnector::firstImgInBatch = img;
    } else {
        // go through the linked list to the last element
        DotsAndBoxesImage *tmp = firstImgInBatch;
        while (tmp->has_nextimage()) {
            tmp = tmp->mutable_nextimage();
        }
        // copy new element
        auto *imgCpy = new DotsAndBoxesImage();
        imgCpy->CopyFrom(img);
        // append new element
        tmp->set_allocated_nextimage(imgCpy);
        assert(tmp->has_nextimage());
    }
     */
    int myIdx = batchCnt;

    copyDataToProtobuf(requestBatch[myIdx], inputimg);

    // if we are the last thread to call, send/recv the data
    if (batchCnt == batchSize-1) {
        // print protobuf message
        if (debug) {
            qDebug() << "request batch size: " << batchSize;
            bool done = false;
            DotsAndBoxesImage *tmp;
            tmp = firstImgInBatch;
            while (!done) {
                qDebug() << "w: " << tmp->width() << "h: " << tmp->height() << "pixels_size: " << tmp->pixels_size();

                if (tmp->has_nextimage()) {
                    tmp = tmp->mutable_nextimage();
                } else {
                    done = true;
                }
            }
        }
        // send prediction request
        if (!ProtobufConnector::sendString(socket, firstImgInBatch->SerializeAsString())) {
            qDebug() << "failed to send message to model server";
            QMessageBox::critical(nullptr, QObject::tr("Connection error"), QObject::tr("failed to send message to model server!"));
            QCoreApplication::exit(1);
        }

        // receive prediction from model server
        bool ok = false;
        std::string rpl = ProtobufConnector::recvString(socket, &ok);
        if (!ok) {
            qDebug() << "failed to receive message from model server";
            QMessageBox::critical(nullptr, QObject::tr("Connection error"), QObject::tr("failed to receive message from model server!"));
            QCoreApplication::exit(1);
        }
        //qDebug() << QString::fromStdString(rpl);

        // parse prediction
        firstPredictionInBatch->ParseFromString(rpl);
        PolicyValueData *tmp = firstPredictionInBatch;
        for (int i = 1; i < batchSize; i++) {
            responseBatch[i] = tmp->mutable_nextdata();
            assert(tmp->has_nextdata());
            tmp = tmp->mutable_nextdata();
        }

        // print protobuf response
        if (debug) {
            qDebug() << "prediction: ";
            PolicyValueData *pvd;
            pvd = firstPredictionInBatch;
            bool done = false;
            while (!done) {
                qDebug() << "v: " << pvd->value();

                if (pvd->has_nextdata()) {
                    pvd = pvd->mutable_nextdata();
                } else {
                    done = true;
                }
            }
        }

        // let other threads access the response
        batchPredictionState = 1;
    }

    // count up after we received the request (if we're the last thread)
    batchCnt++;

    batchImgMutex.unlock();

    // wait for other threads to add their sample, and get the result
    while (batchPredictionState != 1) {
        QThread::usleep(10);
    }

    PolicyValueData *ret = responseBatch[myIdx];

    return *ret;
}

void ProtobufConnector::releaseBatchSample() {
    if (!batchPredictMode) {
        return;
    }
    QMutexLocker locker(&batchImgMutex);
    batchCnt--;
    if (batchCnt == 0) {
        batchPredictionState = 0;
    }

}

void ProtobufConnector::setBatchSize(int size) {
    QMutexLocker locker(&batchImgMutex);
    batchSize = size;
    batchCnt = 0;
    requestBatch.clear();
    for (int i = 0; i < batchSize; i++) {
        requestBatch.append(new DotsAndBoxesImage());
        if (i > 0) {
            requestBatch[i - 1]->set_allocated_nextimage(requestBatch[i]);
        }
    }
    firstImgInBatch = requestBatch[0];
    responseBatch.clear();
    for (int i = 0; i < batchSize; i++) {
        responseBatch.append(new PolicyValueData());
        if (i > 0) {
            responseBatch[i - 1]->set_allocated_nextdata(responseBatch[i]);
        }
    }
    firstPredictionInBatch = responseBatch[0];
}

void ProtobufConnector::setBatchPredict(bool mode) {
    batchPredictMode = mode;
}

bool ProtobufConnector::getBatchPredict() {
    return batchPredictMode;
}

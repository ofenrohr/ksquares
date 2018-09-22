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
#include <zmq.hpp>
#include <QtCore/QTimer>
#include <ModelServer.pb.h>
#include <aifunctions.h>

using namespace AlphaDots;

int ProtobufConnector::TIMEOUT = 30 * 1000;

ProtobufConnector &ProtobufConnector::getInstance() {
    static ProtobufConnector instance;
    return instance;
}

ProtobufConnector::ProtobufConnector() :
    context(1),
    modelListMutex(QMutex::Recursive),
    modelListProcMutex(QMutex::NonRecursive),
    batchImgMutex(QMutex::NonRecursive),
    batchSize(1),
    batchCnt(0),
    batchPredictionState(0),
    batchPredictMode(false)
{
    modelListProc = nullptr;
}

ProtobufConnector::~ProtobufConnector() {
    qDebug() << "~ProtobufConnector";
    if (modelListProc != nullptr) {
        modelListProc->stopExternalProcess(false, true, false);
        delete modelListProc;
    }
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
    assert(img.format() == QImage::Format_ARGB32);
    for (int y = 0; y < img.height(); y++) {
        QRgb *scanline = (QRgb*) img.constScanLine(y);
        for (int x = 0; x < img.width(); x++) {
            QRgb pixel = scanline[x];
            if (sizeMatch) {
                pb->set_pixels(y*img.width()+x, qRed(pixel));
                //pb->set_pixels(y*img.width()+x, img.pixelColor(x, y).red());
            } else {
                pb->add_pixels(qRed(pixel));
                //pb->add_pixels(img.pixelColor(x, y).red());
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

QImage ProtobufConnector::protobufDotsAndBoxesImageToQImage(const DotsAndBoxesImage &img) {
    QImage ret(img.width(), img.height(), QImage::Format_ARGB32);
    for (int i = 0; i < img.width() * img.height(); i++) {
        int c = img.pixels().Get(i);
        int x = i % img.width();
        int y = i / img.width();
        ret.setPixel(x, y, qRgb(c,c,c));
    }
    return ret;
}

bool ProtobufConnector::ensureModelListServerRunning() {
    QMutexLocker locker(&modelListProcMutex);

    if (modelListProc != nullptr) {
        if (!modelListProc->isRunning()) {
            if (!modelListProc->startExternalProcess()) {
                qDebug() << "ERROR: failed to restart model list process";
                return false;
            }
            // restart of model list process successful
            return true;
        } else {
            // model list is already running
            return true;
        }
    }

    QDir dir(Settings::alphaDotsDir());
    if (!dir.exists()) {
        qDebug() << "ERROR: alphaDots dir does not exist!";
        return false;
    }

    QString process = Settings::pythonExecutable();
    QStringList processArgs;
    processArgs << Settings::alphaDotsDir() + "/modelServer/modelList.py";
    QString workingDir = Settings::alphaDotsDir() + "/modelServer/";
    modelListProc = new ExternalProcess(process, processArgs, workingDir);
    connect(modelListProc, SIGNAL(processFinished()), this, SLOT(modelListServerFinished()));
    if (!modelListProc->startExternalProcess()) {
        qDebug() << "ERROR: failed to start " << process << processArgs;
        return false;
    }

    return true;
}

void ProtobufConnector::modelListServerFinished() {
    qDebug() << "Model List Server finished!";
    delete modelListProc;
    modelListProc = nullptr;
}

QList<ModelInfo> ProtobufConnector::getModelList() {
    QMutexLocker locker(&modelListMutex);

    if (!ensureModelListServerRunning()) {
        qDebug() << "ERROR: no model list server running, returning empty model list!";
        return QList<ModelInfo>();
    }

    QDir dir(Settings::alphaDotsDir());
    if (!dir.exists()) {
        qDebug() << "ERROR: alpha dots not configured - returning empty model list!";
        return QList<ModelInfo>();
    }

    try {
        zmq::socket_t socket(context, ZMQ_REQ);
        socket.connect("tcp://127.0.0.1:13452");

        ModelListRequest modelListRequest;
        modelListRequest.set_action(ModelListRequest::GET);
        sendString(socket, modelListRequest.SerializeAsString());

        bool ok = false;
        std::string response = recvString(socket, &ok);
        if (!ok) {
            qDebug() << "ERROR: failed to receive model list (zmq failed)";
            socket.close();
            return QList<ModelInfo>();
        }
        ModelList modelList;
        modelList.ParseFromString(response);

        QList<ModelInfo> retModelList;
        for (int i = 0; i < modelList.models().size(); i++) {
            ProtoModel model = modelList.models().Get(i);
            ModelInfo retModel(QString::fromStdString(model.name()), QString::fromStdString(model.desc()),
                                 QString::fromStdString(model.path()), QString::fromStdString(model.type()),
                                 QString::fromStdString(model.ai()));
            retModelList.append(retModel);
            /*
            cachedModelList.append(ModelInfo(QString::fromStdString(model.name()), QString::fromStdString(model.desc()),
                                 QString::fromStdString(model.path()), QString::fromStdString(model.type()), QString::fromStdString(model.ai())));
             */
        }
        return retModelList;

    } catch (zmq::error_t &err) {
        qDebug() << "ERROR receiving list: " << err.num() << err.what();
    }

    qDebug() << "ERROR: failed to get model list, returning empty list";
    return QList<ModelInfo>();
}

bool ProtobufConnector::addModelToList(ModelInfo model) {
    QMutexLocker locker(&modelListMutex);
    qDebug() << "adding model to list" << model.name();

    if (!ensureModelListServerRunning()) {
        qDebug() << "ERROR: model list server not running, can not add model!";
        return false;
    }

    try {
        zmq::socket_t socket(context, ZMQ_REQ);
        socket.connect("tcp://127.0.0.1:13452");

        ModelListRequest modelListRequest;
        modelListRequest.set_action(ModelListRequest::ADD);
        modelListRequest.set_allocated_model(model.toProtobuf());
        sendString(socket, modelListRequest.SerializeAsString());

        bool ok = false;
        std::string response = recvString(socket, &ok);
        if (!ok) {
            qDebug() << "ERROR: failed to add model to list (zmq failed)";
            socket.close();
            return false;
        }
        ModelList modelList;
        modelList.ParseFromString(response);
    } catch (zmq::error_t &err) {
        qDebug() << "ERROR adding model to list: " << err.num() << err.what();
        return false;
    }

    return true;
}

ModelInfo ProtobufConnector::getModelByName(QString name) {
    QMutexLocker locker(&modelListMutex);
    QList<ModelInfo> modelList = getModelList();
    for (auto model : modelList) {
        if (model.name() == name) {
            return model;
        }
    }
    bool ok = false;
    aiFunctions::parseAiLevel(name, &ok);
    if (ok) {
        return ModelInfo(name, "", "", "", name);
    }
    qDebug() << "ERROR: failed to find model with name " << name;
    assert(false);
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

    bool done = false;
    QTime timeoutTimer;
    timeoutTimer.start();
    while (!done) {
        try {
            if (socket.send(request, ZMQ_DONTWAIT)) {
                done = true;
            }
        } catch (zmq::error_t &ex) {
            qDebug() << "zmq send error: " << ex.num();
            qDebug() << "msg: " << ex.what();
            //QMessageBox::critical(nullptr, QObject::tr("zmq error"), QObject::tr("failed to send message with zmq"));
            return false;
        }
        if (timeoutTimer.elapsed() > TIMEOUT) {
            qDebug() << "zmq send timeout!";
            return false;
        }
    }
    return true;
}

std::string ProtobufConnector::recvString(zmq::socket_t &socket, bool *ok) {
	zmq::message_t reply;
    *ok = true;
    bool done = false;
    QTime timeoutTimer;
    timeoutTimer.start();
    while (!done) {
        try {
            if (socket.recv(&reply, ZMQ_DONTWAIT)) {
                done = true;
            }
        } catch (zmq::error_t &ex) {
            qDebug() << "zmq recv error: " << ex.num();
            qDebug() << "msg: " << ex.what();
            //QMessageBox::critical(nullptr, QObject::tr("zmq error"), QObject::tr("failed to recv message with zmq"));
            *ok = false;
            return "error";
        }
        if (timeoutTimer.elapsed() > TIMEOUT) {
            qDebug() << "zmq recv timeout!";
            *ok = false;
            return "timeout";
        }
    }
    std::string rpl = std::string(static_cast<char*>(reply.data()), reply.size());
    return rpl;
}

PolicyValueData ProtobufConnector::batchPredict(zmq::socket_t &socket, QImage &inputimg) {
    bool debug = ModelManager::getInstance().getDebug();

    // wait for previous batch to finish...
    while (ProtobufConnector::batchPredictionState != 0) {
        QThread::usleep(10);
    }

    batchImgMutex.lock();
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

void ProtobufConnector::requestStatus(zmq::socket_t &socket) {
    ModelServerRequest statusRequest;
    statusRequest.set_action(ModelServerRequest::STATUS);
    if (!ProtobufConnector::sendString(socket, statusRequest.SerializeAsString())) {
        qDebug() << "failed to request status!";
        return;
    }
    bool ok = false;
    std::string resp = ProtobufConnector::recvString(socket, &ok);
    if (!ok) {
        qDebug() << "failed to receive status!";
        return;
    }
    ModelServerResponse statusResponse;
    statusResponse.ParseFromString(resp);
    qDebug() << "STATUS: ";
    if (statusResponse.status() == ModelServerResponse::RESP_OK ) {
        qDebug() << "RESP_OK";
        qDebug().noquote() << QString::fromStdString(statusResponse.statusmessage());
    } else {
        qDebug() << "RESP_FAIL";
    }
}

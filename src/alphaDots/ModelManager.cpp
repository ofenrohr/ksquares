//
// Created by ofenrohr on 30.03.18.
//

#include <QtCore/QCoreApplication>
#include <QtCore/QThread>
#include <settings.h>
#include <QtWidgets/QMessageBox>
#include "ProcessManagement.pb.h"
#include "ModelServer.pb.h"
#include "ModelManager.h"
#include "ProtobufConnector.h"


using namespace AlphaDots;

ModelManager::ModelManager() :
    zmqContext(zmq::context_t(1)),
    mgmtSocket(zmq::socket_t(zmqContext, ZMQ_REQ))
{
    qDebug() << "Starting multi model server...";
    QStringList args;
    args << Settings::alphaDotsDir() + QStringLiteral("/modelServer/multiModelServer.py");
    //args << QStringLiteral("--debug");
    metaModelManager = ExternalProcess::Ptr(new ExternalProcess(Settings::pythonExecutable(), args, Settings::alphaDotsDir()));
    if (!metaModelManager->startExternalProcess()) {
        QMessageBox::critical(nullptr, tr("AlphaDots Error"), tr("Failed to start python model manager!"));
        return;
    }
    qDebug() << "Connecting to tcp://127.0.0.1:12352";
    mgmtSocket.connect("tcp://127.0.0.1:12352");
    qDebug() << "Connection established";
}

int ModelManager::sendStartRequest(QString name, int width, int height, bool gpu) {
    qDebug() << "ModelManager->sendStartRequest: " << name << "," << width << "," << height << "," << gpu;
    QString modelKey = modelInfoToStr(name, width, height, gpu || useGPU);
    // prepare request
    ModelServerRequest srvRequest;
    srvRequest.set_action(ModelServerRequest::MANAGE);
    srvRequest.mutable_mgmtrequest()->set_model(name.toStdString());
    srvRequest.mutable_mgmtrequest()->set_width(width);
    srvRequest.mutable_mgmtrequest()->set_height(height);
    srvRequest.mutable_mgmtrequest()->set_key(modelKey.toStdString());
    srvRequest.mutable_mgmtrequest()->set_action(ProcessManagementRequest::START);
    srvRequest.mutable_mgmtrequest()->set_gpu(gpu || useGPU);

    assert(srvRequest.mgmtrequest().model() == name.toStdString());

    // send request
    if (!ProtobufConnector::sendString(mgmtSocket, srvRequest.SerializeAsString())) {
        QMessageBox::critical(nullptr, tr("AlphaDots error"), tr("Failed to send command to start model server!"));
        return -1;
    }

    // receive response
    bool ok = false;
    std::string zmqResp = ProtobufConnector::recvString(mgmtSocket, &ok);
    if (!ok) {
        QMessageBox::critical(nullptr, tr("AlphaDots error"), tr("Failed to start model server!"));
        return -1;
    }
    //ProcessManagementResponse resp;
    ModelServerResponse srvResponse;
    srvResponse.ParseFromString(zmqResp);
    if (srvResponse.status() != ModelServerResponse::RESP_OK) {
        qDebug() << "ERROR: Starting model server failed: " << QString::fromStdString(srvResponse.errormessage());
        assert(false);
    }
    assert(modelKey.toStdString() == srvResponse.mgmtresponse().key());

    // return model server port
    return srvResponse.mgmtresponse().port();
}

int ModelManager::sendStopRequest(ModelProcess::Ptr process) {
    // prepare request
    ModelServerRequest srvRequest;
    srvRequest.set_action(ModelServerRequest::MANAGE);
    srvRequest.mutable_mgmtrequest()->set_model(process->model().toStdString());
    srvRequest.mutable_mgmtrequest()->set_width(process->width());
    srvRequest.mutable_mgmtrequest()->set_height(process->height());
    srvRequest.mutable_mgmtrequest()->set_key(process->key().toStdString());
    srvRequest.mutable_mgmtrequest()->set_action(ProcessManagementRequest::STOP);
    srvRequest.mutable_mgmtrequest()->set_gpu(process->gpu() || useGPU);

    // send request
    if (!ProtobufConnector::sendString(mgmtSocket, srvRequest.SerializeAsString())) {
        QMessageBox::critical(nullptr, tr("AlphaDots error"), tr("Failed to send command to stop model server!"));
        return -1;
    }

    // receive response
    bool ok = false;
    std::string zmqResp = ProtobufConnector::recvString(mgmtSocket, &ok);
    if (!ok) {
        QMessageBox::critical(nullptr, tr("AlphaDots error"), tr("Failed to stop model server!"));
        return -1;
    }
    ModelServerResponse resp;
    resp.ParseFromString(zmqResp);
    if(resp.status() != ModelServerResponse::RESP_OK) {
        qDebug() << "ERROR: Starting model server failed: " << QString::fromStdString(resp.errormessage());
        assert(false);
    }
    assert(process->key().toStdString() == resp.mgmtresponse().key());

    // return model server port
    return resp.mgmtresponse().port();
}

int ModelManager::ensureProcessRunning(const QString modelName, int width, int height, bool gpu) {
    ModelProcess::Ptr process = getProcess(modelName, width, height, gpu || useGPU);
    return process->port();
}

ModelProcess::Ptr ModelManager::getProcess(const QString modelName, int width, int height, bool gpu) {
    QMutexLocker locker(&getProcessMutex);
    QString modelKey = modelInfoToStr(modelName, width, height, gpu || useGPU);
    if (!processMap.contains(modelKey)) {
        //if (maxConcurrentProcesses > 0 && maxConcurrentProcesses <= processMap.keys().size()) {
        //    while (maxConcurrentProcesses <= processMap.keys().size() && !processMap.contains(modelKey)) {
        while ((gpu||useGPU) && maxConcurrentProcesses > 0 && activeGPUprocesses() >= maxConcurrentProcesses && !processMap.contains(modelKey)) {
            // TODO: warn if we can't get a gpu after 2 minutes
            // wait for process to be released before starting the next process
            locker.unlock();
            cleanUp();
            //qDebug() << "waiting for other ModelProcess to finish...";
            QCoreApplication::processEvents();
            QThread::msleep(100);
            locker.relock();
        }
        if (processMap.contains(modelKey)) {
            // another thread has started the process while this thread was waiting
            processClaims[modelKey] += 1;
        } else {
            // start the process
            qDebug() << "sending model start request...";
            int port = sendStartRequest(modelName, width, height, gpu || useGPU);
            qDebug() << "model starting on port " << QString::number(port);
            ModelProcess::Ptr process = ModelProcess::Ptr(new ModelProcess(modelName, width, height, port, gpu || useGPU, modelKey));
            processMap[modelKey] = process;
            processClaims[modelKey] = 1;
        }
    } else {
        processClaims[modelKey] += 1;
    }
    //qDebug() << "claims on " << modelKey << " at " << processClaims[modelKey];
    return processMap[modelKey];
}

void ModelManager::freeClaimOnProcess(QString modelName, int width, int height, bool gpu) {
    QMutexLocker locker(&getProcessMutex);
    QString modelKey = modelInfoToStr(modelName, width, height, gpu || useGPU);
    processClaims[modelKey] -= 1;
    //qDebug() << "claims on " << modelKey << " at " << processClaims[modelKey];
    /*
    if (processClaims[modelKey] == 0 && maxConcurrentProcesses > 0) {
        if (debug) {
            qDebug() << "Releasing ModelProcess...";
        }
    }
     */
}

void ModelManager::cleanUp() {
    QMutexLocker locker(&getProcessMutex);
    if (maxConcurrentProcesses <= 0) {
        return;
    }
    for (const QString &modelKey : processMap.keys()) {
        if ((processMap[modelKey]->gpu() || useGPU) && processClaims[modelKey] == 0) {
            if (debug) {
                qDebug() << "stopping " << modelKey;
            }
            sendStopRequest(processMap[modelKey]);
            processMap.remove(modelKey);
        }
    }
}

int ModelManager::activeGPUprocesses() {
    int processCnt = 0;
    for (const auto &process : processMap.values()) {
        if (process->gpu() || useGPU) {
            processCnt++;
        }
    }
    return processCnt;
}

QString ModelManager::modelInfoToStr(QString modelName, int width, int height, bool gpu) {
    return modelName + QStringLiteral("-") + QString::number(width) + QStringLiteral("x") + QString::number(height) +
           (gpu ? QStringLiteral("-GPU") : QStringLiteral(""));
}

void ModelManager::allowGPU(bool allowGPU) {
    useGPU = allowGPU;
    if (allowGPU) {
        qDebug() << "GPU active: limiting maximum number of concurrent ModelProcess instances to 1";
        setMaximumConcurrentProcesses(1);
    } else {
        setMaximumConcurrentProcesses(0);
    }
}

void ModelManager::stopAll(bool wait) {
    for (const auto &process : processMap) {
        sendStopRequest(process);
    }
    processMap.clear();
}

void ModelManager::setDebug(bool mode) {
    debug = mode;
}

void ModelManager::setMaximumConcurrentProcesses(int max) {
    maxConcurrentProcesses = max;
}


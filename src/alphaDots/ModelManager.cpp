//
// Created by ofenrohr on 30.03.18.
//

#include <QtCore/QCoreApplication>
#include <QtCore/QThread>
#include <settings.h>
#include <QtWidgets/QMessageBox>
#include "ProcessManagement.pb.h"
#include "ModelManager.h"
#include "ProtobufConnector.h"


using namespace AlphaDots;

ModelManager::ModelManager() :
    zmqContext(zmq::context_t(1)),
    mgmtSocket(zmq::socket_t(zmqContext, ZMQ_REQ))
{
    qDebug() << "Starting meta model server...";
    QStringList args;
    args << Settings::alphaDotsDir() + QStringLiteral("/modelServer/metaModelServer.py");
    if (debug) {
        args << QStringLiteral("--debug");
    }
    metaModelManager = ExternalProcess::Ptr(new ExternalProcess(Settings::pythonExecutable(), args, Settings::alphaDotsDir()));
    if (!metaModelManager->startExternalProcess()) {
        QMessageBox::critical(nullptr, tr("AlphaDots Error"), tr("Failed to start python model manager!"));
        return;
    }
    qDebug() << "Connecting to tcp://127.0.0.1:12353";
    mgmtSocket.connect("tcp://127.0.0.1:12353");
    qDebug() << "Connection established";
}

int ModelManager::sendStartRequest(QString name, int width, int height, bool gpu) {
    QString modelKey = modelInfoToStr(name, width, height, gpu || useGPU);
    // prepare request
    ProcessManagementRequest mgmtRequest;
    mgmtRequest.set_model(name.toStdString());
    mgmtRequest.set_width(width);
    mgmtRequest.set_height(height);
    mgmtRequest.set_key(modelKey.toStdString());
    mgmtRequest.set_action(ProcessManagementRequest::START);
    mgmtRequest.set_gpu(gpu || useGPU);

    // send request
    if (!ProtobufConnector::sendString(mgmtSocket, mgmtRequest.SerializeAsString())) {
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
    ProcessManagementResponse resp;
    resp.ParseFromString(zmqResp);
    assert(modelKey.toStdString() == resp.key());

    // return model server port
    return resp.port();
}

int ModelManager::sendStopRequest(ModelProcess::Ptr process) {
    // prepare request
    ProcessManagementRequest mgmtRequest;
    mgmtRequest.set_model(process->model().toStdString());
    mgmtRequest.set_width(process->width());
    mgmtRequest.set_height(process->height());
    mgmtRequest.set_key(process->key().toStdString());
    mgmtRequest.set_action(ProcessManagementRequest::STOP);
    mgmtRequest.set_gpu(process->gpu() || useGPU);

    // send request
    if (!ProtobufConnector::sendString(mgmtSocket, mgmtRequest.SerializeAsString())) {
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
    ProcessManagementResponse resp;
    resp.ParseFromString(zmqResp);
    assert(process->key().toStdString() == resp.key());

    // return model server port
    return resp.port();
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
           (gpu || useGPU ? QStringLiteral("-GPU") : QStringLiteral(""));
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


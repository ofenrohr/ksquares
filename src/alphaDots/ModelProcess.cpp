//
// Created by ofenrohr on 30.03.18.
//

#include "ModelProcess.h"
#include <qdebug.h>
#include <settings.h>
#include <QtCore/QTimer>

using namespace AlphaDots;

ModelProcess::ModelProcess(QString model, int boxesWidth, int boxesHeight, int port, bool allowGPU, bool debug, QString logdest, QString modelKey) :
    width(boxesWidth),
    height(boxesHeight),
    modelPort(port),
    processKey(modelKey)
{
    qDebug() << "starting ModelProcess(" << model << "," << width << "," << height << "," << modelPort << ")";

    //qDebug() << "Starting: modelServer.py --model" << model;
    QStringList args;
    args << Settings::alphaDotsDir() + QStringLiteral("/modelServer/modelServer.py")
         << QStringLiteral("--model")
         << model
         << QStringLiteral("--width")
         << QString::number(width*2 + 3)
         << QStringLiteral("--height")
         << QString::number(height*2 + 3)
         << QStringLiteral("--port")
         << QString::number(port);
    if (!logdest.isEmpty()) {
        args << QStringLiteral("--logdest")
             << logdest;
    }
    if (debug) {
        args << QStringLiteral("--debug");
    }
    modelServer = new ExternalProcess(Settings::pythonExecutable(), args);
    if (!allowGPU) {
        modelServer->addEnvironmentVariable(QStringLiteral("CUDA_VISIBLE_DEVICES"), QStringLiteral("-1"));
    }
    if (!modelServer->startExternalProcess()) {
        qDebug() << "ERROR: can't start model server!";
        modelPort = -1;
    }
    modelServer->processEvents();
}

ModelProcess::~ModelProcess() {
    qDebug() << "~ModelProcess";
    //delete(modelServer);
    //modelServer->deleteLater();
}

bool ModelProcess::isRunning() {
    return modelServer->isRunning();
}

int ModelProcess::port() {
    return modelPort;
}

void ModelProcess::stop(bool wait) {
    connect(modelServer, SIGNAL(processFinished()), this, SLOT(processFinishedSlot()));
    //modelServer->stopProcessTerm();
    QTimer::singleShot(0, modelServer, SLOT(stopProcessTerm()));
    /*
    //connect(modelServer, SIGNAL(processFinished()), this, SLOT(processFinished()));
    QTimer::singleShot(0, modelServer, SLOT(stopProcessTerm()));
    //modelServer->stopExternalProcess(true, false, wait);
    QCoreApplication::processEvents();
    while (isRunning() && wait) {
        int ms = 500;
        struct timespec ts = {ms / 1000, (ms % 1000) * 1000 * 1000};
        nanosleep(&ts, NULL);
        QCoreApplication::processEvents();
        qDebug() << "waiting for process to stop...";
    }
     */
}

void ModelProcess::processFinishedSlot() {
    disconnect(modelServer, SIGNAL(processFinished()), this, SLOT(processFinishedSlot()));
    emit processFinishedSignal(processKey);
}

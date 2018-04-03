//
// Created by ofenrohr on 30.03.18.
//

#include "ModelProcess.h"
#include <qdebug.h>
#include <settings.h>

using namespace AlphaDots;

ModelProcess::ModelProcess(QString model, int boxesWidth, int boxesHeight, int port) :
    width(boxesWidth),
    height(boxesHeight),
    modelPort(port)
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
         << QString::number(port)
         //<< QStringLiteral("--debug")
            ;
    modelServer = new ExternalProcess(QStringLiteral("/usr/bin/python2.7"), args);
    modelServer->addEnvironmentVariable(QStringLiteral("CUDA_VISIBLE_DEVICES"), QStringLiteral("-1"));
    if (!modelServer->startExternalProcess()) {
        qDebug() << "ERROR: can't start model server!";
        modelPort = -1;
    }
}

ModelProcess::~ModelProcess() {
    qDebug() << "~ModelProcess";
    delete(modelServer);
}

bool ModelProcess::isRunning() {
    return modelServer->isRunning();
}

int ModelProcess::port() {
    return modelPort;
}

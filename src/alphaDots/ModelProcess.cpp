//
// Created by ofenrohr on 30.03.18.
//

#include "ModelProcess.h"
#include <qdebug.h>

using namespace AlphaDots;

ModelProcess::ModelProcess(QString model, int boxesWidth, int boxesHeight, int port, bool gpu, QString modelKey) :
    _model(model),
    _width(boxesWidth),
    _height(boxesHeight),
    _modelPort(port),
    _gpu(gpu),
    _processKey(modelKey)
{
    qDebug() << "starting ModelProcess(" << _model << "," << _width << "," << _height << "," << _processKey << ")";

}

ModelProcess::~ModelProcess() {
    qDebug() << "~ModelProcess";
}

QString ModelProcess::model() {
    return _model;
}

int ModelProcess::port() {
    return _modelPort;
}

bool ModelProcess::gpu() {
    return _gpu;
}

int ModelProcess::width() {
    return _width;
}

int ModelProcess::height() {
    return _height;
}

QString ModelProcess::key() {
    return _processKey;
}

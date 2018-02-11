//
// Created by ofenrohr on 11.02.18.
//

#include "ModelInfo.h"

using namespace AlphaDots;

ModelInfo::ModelInfo(QString name, QString desc, QString path) {
    _name = name;
    _desc = desc;
    _path = path;
}

QString ModelInfo::name() {
    return _name;
}

QString ModelInfo::desc() {
    return _desc;
}

QString ModelInfo::path() {
    return _path;
}

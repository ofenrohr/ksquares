//
// Created by ofenrohr on 11.02.18.
//

#include "ModelInfo.h"

using namespace AlphaDots;

ModelInfo::ModelInfo() {
    _name = _desc = _path = _type = QStringLiteral("");
}

ModelInfo::ModelInfo(QString name, QString desc, QString path, QString type) {
    _name = name;
    _desc = desc;
    _path = path;
    _type = type;
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

QString ModelInfo::type() {
    return _type;
}

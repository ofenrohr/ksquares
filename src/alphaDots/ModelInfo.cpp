//
// Created by ofenrohr on 11.02.18.
//

#include "ModelInfo.h"

using namespace AlphaDots;

ModelInfo::ModelInfo() {
    _name = _desc = _path = _type = _ai = QStringLiteral("");
    _valid = false;
}

ModelInfo::ModelInfo(QString name, QString desc, QString path, QString type, QString ai) {
    _name = name;
    _desc = desc;
    _path = path;
    _type = type;
    _ai = ai;
    _valid = true;
}

QString ModelInfo::name() const {
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

QString ModelInfo::ai() {
    return _ai;
}

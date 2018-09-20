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

bool ModelInfo::valid() {
    return _valid;
}

ProtoModel *ModelInfo::toProtobuf() {
    ProtoModel *ret = new ProtoModel();
    ret->set_name(_name.toStdString());
    ret->set_desc(_desc.toStdString());
    ret->set_path(_path.toStdString());
    ret->set_type(_type.toStdString());
    ret->set_ai(_ai.toStdString());
    return ret;
}

bool ModelInfo::operator==(const ModelInfo &rhs) const {
    return _name == rhs._name &&
           _desc == rhs._desc &&
           _path == rhs._path &&
           _type == rhs._type &&
           _ai == rhs._ai &&
           _valid == rhs._valid;
}

bool ModelInfo::operator!=(const ModelInfo &rhs) const {
    return !(rhs == *this);
}

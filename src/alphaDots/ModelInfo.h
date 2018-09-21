//
// Created by ofenrohr on 11.02.18.
//

#ifndef KSQUARES_MODELINFO_H
#define KSQUARES_MODELINFO_H

#include <QtCore/QString>
#include <AlphaDotsModel.pb.h>


namespace AlphaDots {
    class ModelInfo {
    public:
        ModelInfo();
        ModelInfo(QString name, QString desc, QString path, QString type, QString ai);

        QString name() const;
        QString desc() const;
        QString path() const;
        QString type() const;
        QString ai() const;
        bool valid() const;

        void setValid(bool valid) {_valid=valid;}
        void setName(QString name) {_name=name;}

        ProtoModel *toProtobuf();

        bool operator==(const ModelInfo &rhs) const;

        bool operator!=(const ModelInfo &rhs) const;

    protected:
        QString _name;
        QString _desc;
        QString _path;
        QString _type;
        QString _ai;
        bool _valid;
    };
}


#endif //KSQUARES_MODELINFO_H

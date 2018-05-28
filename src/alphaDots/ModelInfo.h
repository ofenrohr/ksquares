//
// Created by ofenrohr on 11.02.18.
//

#ifndef KSQUARES_MODELINFO_H
#define KSQUARES_MODELINFO_H

#include <QtCore/QString>


namespace AlphaDots {
    class ModelInfo {
    public:
        ModelInfo();
        ModelInfo(QString name, QString desc, QString path, QString type, QString ai);

        QString name() const;
        QString desc();
        QString path();
        QString type();
        QString ai();

        void setName(QString name) {_name=name;}

    protected:
        QString _name;
        QString _desc;
        QString _path;
        QString _type;
        QString _ai;
    };
}


#endif //KSQUARES_MODELINFO_H

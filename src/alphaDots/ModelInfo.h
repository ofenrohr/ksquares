//
// Created by ofenrohr on 11.02.18.
//

#ifndef KSQUARES_MODELINFO_H
#define KSQUARES_MODELINFO_H

#include <QtCore/QString>


namespace AlphaDots {
    class ModelInfo {
    public:
        ModelInfo(QString name, QString desc, QString path);

        QString name();

        QString desc();

        QString path();

    protected:
        QString _name;
        QString _desc;
        QString _path;
    };
}


#endif //KSQUARES_MODELINFO_H

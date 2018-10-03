//
// Created by ofenrohr on 24.09.18.
//

#ifndef KSQUARES_REPORTLOGGER_H
#define KSQUARES_REPORTLOGGER_H


#include <QObject>
#include <QSharedPointer>
#include <QFile>
#include <QTextStream>
#include <QMutex>

namespace AlphaDots {
    class ReportLogger : public QObject {
        Q_OBJECT
    public:
        typedef QSharedPointer<ReportLogger> Ptr;

        explicit ReportLogger(QString &filepath);

        void log(QString message);
        void logTable(QList<QList<QString>> table);
    private:
        QMutex logMutex;
        QString reportFilePath;
    };
}


#endif //KSQUARES_REPORTLOGGER_H

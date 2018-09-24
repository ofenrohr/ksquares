//
// Created by ofenrohr on 24.09.18.
//

#include "ReportLogger.h"

#include <QDebug>
#include <QMutexLocker>

using namespace AlphaDots;

ReportLogger::ReportLogger(QString &filepath) {
    reportFilePath = filepath;
}

void ReportLogger::log(QString message) {
    QMutexLocker locker(&logMutex);
    QFile reportFile(reportFilePath);
    if (!reportFile.open(QIODevice::WriteOnly | QIODevice::Append)) {
        qDebug() << "ERROR: failed to open (create) report file " << reportFilePath;
        QTextStream report(stdout, QIODevice::WriteOnly);
        report << message;
    } else {
        QTextStream report(&reportFile);
        report << message;
    }
}

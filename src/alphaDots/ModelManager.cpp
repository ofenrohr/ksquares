//
// Created by ofenrohr on 30.03.18.
//

#include "ModelManager.h"

#ifdef Q_OS_WIN
#include <windows.h> // for Sleep
#endif

using namespace AlphaDots;

int ModelManager::ensureProcessRunning(const QString modelName, int width, int height) {
    ModelProcess::Ptr process = getProcess(modelName, width, height);
    if (!process->isRunning()) {
        qDebug() << "failed to start ModelProcess" << QString::number(process->port());
        return -1;
    }
    return process->port();
}

ModelProcess::Ptr ModelManager::getProcess(const QString modelName, int width, int height) {
    QMutexLocker locker(&getProcessMutex);
    QString modelKey = modelInfoToStr(modelName, width, height);
    if (!processMap.contains(modelKey)) {
        qDebug() << "starting new ModelProcess on port " << QString::number(port);
        ModelProcess::Ptr process = ModelProcess::Ptr(new ModelProcess(modelName, width, height, port));
        processMap[modelKey] = process;
        port++;
    }
    return processMap[modelKey];
}

QString ModelManager::modelInfoToStr(QString modelName, int width, int height) {
    return modelName + QStringLiteral("-") + QString::number(width) + QStringLiteral("x") + QString::number(height);
}

/*
void ModelManager::sleep(int ms) {
#ifdef Q_OS_WIN
        Sleep(uint(ms));
#else
        struct timespec ts = { ms / 1000, (ms % 1000) * 1000 * 1000 };
        nanosleep(&ts, NULL);
#endif
}
 */

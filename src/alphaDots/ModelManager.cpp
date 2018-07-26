//
// Created by ofenrohr on 30.03.18.
//

#include <QtCore/QCoreApplication>
#include <QtCore/QThread>
#include "ModelManager.h"

#ifdef Q_OS_WIN
#include <windows.h> // for Sleep
#endif

using namespace AlphaDots;

int ModelManager::ensureProcessRunning(const QString modelName, int width, int height) {
    ModelProcess *process = getProcess(modelName, width, height);
    if (!process->isRunning()) {
        qDebug() << "failed to start ModelProcess" << QString::number(process->port());
        return -1;
    }
    return process->port();
}

ModelProcess *ModelManager::getProcess(const QString modelName, int width, int height) {
    QMutexLocker locker(&getProcessMutex);
    QString modelKey = modelInfoToStr(modelName, width, height);
    if (!processMap.contains(modelKey)) {
        if (maxConcurrentProcesses > 0 && maxConcurrentProcesses <= processMap.keys().size()) {
            while (maxConcurrentProcesses <= processMap.keys().size() && !processMap.contains(modelKey)) {
                // wait for process to be released before starting the next process
                locker.unlock();
                qDebug() << "waiting for other ModelProcess to finish...";
                QCoreApplication::processEvents();
                QThread::msleep(1000);
                locker.relock();
            }
        }
        if (processMap.contains(modelKey)) {
            // another thread has started the process while this thread was waiting
            processClaims[modelKey] += 1;
        } else {
            // start the process
            qDebug() << "starting new ModelProcess on port " << QString::number(port);
            ModelProcess *process = new ModelProcess(modelName, width, height, port, useGPU, debug, logDest, modelKey);
            processMap[modelKey] = process;
            processClaims[modelKey] = 1;
            port++;
        }
    } else {
        processClaims[modelKey] += 1;
    }
    return processMap[modelKey];
}

void ModelManager::freeClaimOnProcess(QString modelName, int width, int height) {
    QMutexLocker locker(&getProcessMutex);
    QString modelKey = modelInfoToStr(modelName, width, height);
    processClaims[modelKey] -= 1;
    if (processClaims[modelKey] == 0 && maxConcurrentProcesses > 0) {
        qDebug() << "Releasing ModelProcess...";
        connect(processMap[modelKey], SIGNAL(processFinishedSignal(QString)), this, SLOT(processFinished(QString)));
        processMap[modelKey]->stop(false);
    }
}

QString ModelManager::modelInfoToStr(QString modelName, int width, int height) {
    return modelName + QStringLiteral("-") + QString::number(width) + QStringLiteral("x") + QString::number(height);
}

void ModelManager::sleep(int ms) {
#ifdef Q_OS_WIN
        Sleep(uint(ms));
#else
        struct timespec ts = { ms / 1000, (ms % 1000) * 1000 * 1000 };
        nanosleep(&ts, NULL);
#endif
}

void ModelManager::allowGPU(bool allowGPU) {
    useGPU = allowGPU;
    if (allowGPU) {
        qDebug() << "GPU active: limiting maximum number of concurrent ModelProcess instances to 1";
        setMaximumConcurrentProcesses(1);
    } else {
        setMaximumConcurrentProcesses(0);
    }
}

void ModelManager::stopAll(bool wait) {
    for (const auto &process : processMap) {
        process->stop(wait);
    }
    processMap.clear();
}

void ModelManager::setDebug(bool mode) {
    debug = mode;
}

void ModelManager::setMaximumConcurrentProcesses(int max) {
    maxConcurrentProcesses = max;
}

void ModelManager::processFinished(QString modelKey) {
    //oldProcesses.append(processMap[modelKey]);
    disconnect(processMap[modelKey], SIGNAL(processFinishedSignal(QString)), this, SLOT(processFinished(QString)));
    //processMap[modelKey]->deleteLater();
    processMap.remove(modelKey);
}

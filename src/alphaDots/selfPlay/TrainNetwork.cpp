//
// Created by ofenrohr on 16.09.18.
//

#include <alphaDots/ProtobufConnector.h>
#include <QtWidgets/QMessageBox>
#include <QTimer>
#include <QFileInfo>
#include <QRegularExpression>
#include <QThread>
#include <QtCore/QDir>
#include "TrainNetwork.h"
#include "settings.h"

using namespace AlphaDots;

TrainNetwork::TrainNetwork(int epochs, bool gpu, bool doUpload, QString datasetDestDir) {
    trainEpochs = epochs;
    trainOnGPU = gpu;
    upload = doUpload;
    iteration = 0;
    trainingProcess = nullptr;
    datasetDest = datasetDestDir;
}

void TrainNetwork::startTraining(QString datasetPath, int trainIteration, QString initModelPath,
                                 QString targetModelPath, QString iterationModelDir) {
    iteration = trainIteration;
    // start training on new data
    QString processPath = Settings::pythonExecutable();
    QStringList processArgs;
    processArgs
            << Settings::alphaDotsDir() + tr("/modelServer/models/alphaZero/alphaZeroV10.py")
            << tr("--dataset")
            << datasetPath
            << tr("--iteration")
            << QString::number(iteration)
            << tr("--epochs")
            << QString::number(trainEpochs)
            << tr("--initmodel")
            << Settings::alphaDotsDir() + tr("/modelServer/models/") + initModelPath
            << tr("--targetmodel")
            << Settings::alphaDotsDir() + tr("/modelServer/models/") + targetModelPath
            << tr("--itermodeldest")
            << iterationModelDir
        //<< tr("--logdest")
        //<< logdest
            ;
    if (upload) {
        processArgs << tr("--upload");
    }
    QString processWorkingDirectory = Settings::alphaDotsDir() + tr("/modelServer/models/alphaZero");
    if (trainingProcess == nullptr) {
        trainingProcess = new ExternalProcess(processPath, processArgs, processWorkingDirectory);
    } else {
        if (trainingProcess->isRunning()) {
            /*
            QMessageBox::critical(this, tr("SelfPlay error"),
                                  tr("Training takes longer than generating data. sth seems wrong! Waiting for training to finish..."));
             */
            while (trainingProcess->isRunning()) {
                QThread::sleep(1);
                QCoreApplication::processEvents();
            }
        }
        disconnect(trainingProcess, SIGNAL(processFinished()), this, SLOT(trainingFinished()));
        trainingProcess->deleteLater();
        trainingProcess = new ExternalProcess(processPath, processArgs, processWorkingDirectory);
    }
    if (!trainOnGPU) {
        // disable gpu, training on very little data -> cpu is enough
        trainingProcess->addEnvironmentVariable(tr("CUDA_VISIBLE_DEVICES"), tr("-1"));
    }
    //connect(trainingProcess, SIGNAL(processFinished()), this, SLOT(trainingFinished()));
    trainingStartTime = QDateTime::currentDateTime();
    if (!trainingProcess->startExternalProcess()) {
        QMessageBox::critical(nullptr, tr("SelfPlay error"),
                              tr("Failed to start external process for training!"));
    }
    statusStr = tr("training...");
    QFileInfo fi(targetModelPath);
    trainingLogBasename = fi.baseName();
    QTimer::singleShot(100, this, SLOT(updateTrainingInfo()));
}

void TrainNetwork::updateTrainingInfo() {
    QDir logDir(Settings::alphaDotsDir() + tr("/modelServer/models/alphaZero/logs/"));

    // prepare regular expressions
    QRegularExpression epochRegex(tr("Epoch (\\d+/\\d+)"));
    // 4/8 [==============>...............] - ETA: 4s - loss: 2.1109 - policy_loss: 1.8616 - value_loss: 0.2096
    QRegularExpression etaRegex(tr("ETA: (.*?) -"));
    QRegularExpression lossRegex(tr("loss: (.*?) - policy_loss: (.*?) - value_loss: (.*?)$"));

    // search log file
    QStringList nameFilters;
    nameFilters << trainingLogBasename + tr("*log");
    auto entryList = logDir.entryList(nameFilters, QDir::Files | QDir::NoDotAndDotDot, QDir::Time);
    if (!entryList.empty()) {
        // check log file
        QString trainingLogPath = logDir.absolutePath() + tr("/") + entryList.first();
        QFile trainingLog(trainingLogPath);
        if (trainingLog.exists()) {
            if (QFileInfo(trainingLog).created() >= trainingStartTime) {
                // link to log file
                logLink = tr("<a href=\"") + trainingLogPath + tr("\">") + entryList.first() + tr("</a>");
                // open log file
                if (trainingLog.open(QIODevice::ReadOnly)) {
                    // read log file
                    QTextStream logStream(&trainingLog);
                    QString logStr = logStream.readAll();

                    // match epoch
                    auto matchIter = epochRegex.globalMatch(logStr);
                    while (matchIter.hasNext()) {
                        auto match = matchIter.next();
                        if (match.isValid()) {
                            if (match.capturedTexts().size() == 2) {
                                epochStr = match.captured(1);
                                //qDebug() << "MATCH!";
                            } else {
                                epochStr = QString::number(match.capturedTexts().size());
                            }
                        }
                    }

                    // match eta
                    auto etaIter = etaRegex.globalMatch(logStr);
                    while (etaIter.hasNext()) {
                        auto match = etaIter.next();
                        if (match.isValid()) {
                            if (match.capturedTexts().size() == 2) {
                                etaStr = match.captured(1);
                            }
                        }
                    }

                    // match loss
                    auto lossIter = lossRegex.globalMatch(logStr);
                    while (lossIter.hasNext()) {
                        auto match = lossIter.next();
                        if (match.isValid()) {
                            if (match.capturedTexts().size() == 4) {
                                lossStr = match.captured(1);
                                policyLossStr = match.captured(2);
                                valueLossStr = match.captured(3);
                            }
                        }
                    }
                } else {
                    qDebug() << "failed to open training log!";
                }
            } else {
                qDebug() << "current training log doesn't exist yet";
            }
        } else {
            qDebug() << "training log doesn't exist!" << entryList.first();
        }
    } else {
        qDebug() << "empty entry list!";
    }
    //while (it.hasNext()) {
    //    QFile f(it.next());
    //    f.open(QIODevice::ReadOnly);
    //QFile trainingLog(
    if (trainingProcess->isRunning()) {
        QTimer::singleShot(3000, this, SLOT(updateTrainingInfo()));
    } else {
        statusStr = tr("waiting for new data...");
        emit trainingFinished();
    }
    emit infoChanged();
}

bool TrainNetwork::trainingInProgress() const {
    if (trainingProcess == nullptr) {
        return false;
    }
    return trainingProcess->isRunning();
}

const QString &TrainNetwork::getLogLink() const {
    return logLink;
}

const QString &TrainNetwork::getEpochStr() const {
    return epochStr;
}

const QString &TrainNetwork::getEtaStr() const {
    return etaStr;
}

const QString &TrainNetwork::getLossStr() const {
    return lossStr;
}

const QString &TrainNetwork::getPolicyLossStr() const {
    return policyLossStr;
}

const QString &TrainNetwork::getValueLossStr() const {
    return valueLossStr;
}

const QString &TrainNetwork::getStatusStr() const {
    return statusStr;
}


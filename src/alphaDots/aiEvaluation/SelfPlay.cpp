//
// Created by ofenrohr on 02.05.18.
//

#include <QtCore/QTimer>
#include <QtCore/QThread>
#include <QDebug>
#include <alphaDots/ProtobufConnector.h>
#include <alphaDots/MLImageGenerator.h>
#include <alphaDots/datasets/StageFourDataset.h>
#include <alphaDots/ExternalProcess.h>
#include <settings.h>
#include <QtWidgets/QMessageBox>
#include <alphaDots/ModelManager.h>
#include <QtCore/QDirIterator>
#include "SelfPlay.h"
#include "SelfPlayWorker.h"

using namespace AlphaDots;

SelfPlay::SelfPlay(QString datasetDest, int threads, QString &initialModelName, QString &targetModel,
                   int iterations, int gamesPerIteration, QString &logdir, int epochs, bool gpuTraining,
                   DatasetType dataset, bool doUpload) :
    KXmlGuiWindow(),
    m_view(new QWidget())
{
    qDebug() << "SelfPlay()";

    datasetDirectory = datasetDest;
    targetModelName = targetModel;
    logdest = logdir;
    threadCnt = threads;
    datasetType = dataset;
    upload = doUpload;

    currentModel = ProtobufConnector::getInstance().getModelByName(initialModelName);
    iteration = -1;
    iterationCnt = iterations;
    iterationSize = gamesPerIteration;
    gamesCompleted = 0;

    availableBoardSizes.clear();
    availableBoardSizes.append(QPoint(4,3));
    availableBoardSizes.append(QPoint(5,4));
    availableBoardSizes.append(QPoint(6,5));
    availableBoardSizes.append(QPoint(7,5));
    availableBoardSizes.append(QPoint(8,8));
    availableBoardSizes.append(QPoint(14,7));
    availableBoardSizes.append(QPoint(14,14));
    availableBoardSizes.append(QPoint(10,9));

    currentBoardSize = availableBoardSizes[0];

    //input = nullptr;
    //output = nullptr;
    //value = nullptr;
    input = new std::vector<uint8_t>();//imgDataSize);
    output = new std::vector<uint8_t>();//imgDataSize);
    value = new std::vector<double >();//iterationSize);

    trainEpochs = epochs;
    trainOnGPU = gpuTraining;
    alphaZeroV10Training = nullptr;

    assert(iterationSize % threads == 0);


    QTimer::singleShot(0, this, &SelfPlay::initObject);
}

void SelfPlay::initObject() {
    qDebug() << "initObject()";

    setupUi(m_view);
    setCentralWidget(m_view);
    setupGUI();

    setupIteration();
    trainingStatusLbl->setText(tr("waiting for data..."));
}

void SelfPlay::updateInfo() {
    currentModelLabel->setText(currentModel.name());
    boardSizeLabel->setText(tr("%1 x %2").arg(currentBoardSize.x()).arg(currentBoardSize.y()));
    iterationLabel->setText(QString::number(iteration));
    progressBar->setMinimum(0);
    progressBar->setMaximum(iterationSize);
    progressBar->setValue(gamesCompleted);
    datasetGeneratorLbl->setText(datasetType == StageFour ? tr("Stage Four") : tr("Stage Four (no MCTS)"));
    lastInfoUpdate = QDateTime::currentDateTime();
}

void SelfPlay::updateTrainingInfo() {
    QDir logDir(Settings::alphaDotsDir() + tr("/modelServer/models/alphaZero/logs/"));

    // prepare label text with default values
    QString epochStr, etaStr, lossStr, policyLossStr, valueLossStr;

    // prepare regular expressions
    QRegularExpression epochRegex(tr("Epoch (\\d+/\\d+)"));
    // 4/8 [==============>...............] - ETA: 4s - loss: 2.1109 - policy_loss: 1.8616 - value_loss: 0.2096
    QRegularExpression etaRegex(tr("ETA: (.*?) -"));
    QRegularExpression lossRegex(tr("loss: (.*?) - policy_loss: (.*?) - value_loss: (.*?)$"));

    // search log file
    QStringList nameFilters;
    nameFilters << trainingLogBasename + tr("*");
    auto entryList = logDir.entryList(nameFilters, QDir::Files | QDir::NoDotAndDotDot, QDir::Time);
    if (entryList.size() > 0) {
        // check log file
        QString trainingLogPath = logDir.absolutePath() + tr("/") + entryList.first();
        QFile trainingLog(trainingLogPath);
        if (trainingLog.exists()) {
            if (QFileInfo(trainingLog).created() >= trainingStartTime) {
                // link to log file
                logLinkLbl->setText(tr("<a href=\"") + trainingLogPath + tr("\">") + entryList.first() + tr("</a>"));
                logLinkLbl->setTextFormat(Qt::RichText);
                logLinkLbl->setTextInteractionFlags(Qt::TextBrowserInteraction);
                logLinkLbl->setOpenExternalLinks(true);
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
    epochLbl->setText(epochStr);
    etaLbl->setText(etaStr);
    lossLbl->setText(lossStr);
    policyLossLbl->setText(policyLossStr);
    valueLossLbl->setText(valueLossStr);
    //while (it.hasNext()) {
    //    QFile f(it.next());
    //    f.open(QIODevice::ReadOnly);
    //QFile trainingLog(
    if (alphaZeroV10Training->isRunning()) {
        QTimer::singleShot(3000, this, SLOT(updateTrainingInfo()));
    } else {
        if (iteration >= iterationCnt) {
            QCoreApplication::exit(0);
        }
    }
}

void SelfPlay::setupIteration() {
    iteration++;
    gamesCompleted = 0;

    // prepare the data containers
    /*
    if (input != nullptr && output != nullptr && value != nullptr) {
        delete input;
        delete output;
        delete value;
    }
     */
    int w = MLImageGenerator::boxesToImgSize(currentBoardSize.x());
    int h = MLImageGenerator::boxesToImgSize(currentBoardSize.y());
    int imgDataSize = iterationSize * w * h;
    input->resize(imgDataSize);
    output->resize(imgDataSize);
    value->resize(iterationSize);

    // check and reset thread status
    for (const auto &i : threadRunning) {
        assert(!i);
    }
    threadRunning.clear();

    // start the threads
    assert(threadGenerators.empty());
    for (int i = 0; i < threadCnt; i++) {
        StageFourDataset *gen;
        switch (datasetType) {
            case StageFour:
                gen = new StageFourDataset(false,
                                           currentBoardSize.x(),
                                           currentBoardSize.y(),
                                           currentModel.name(),
                                           i,
                                           threadCnt);
                break;
            case StageFourNoMCTS:
                gen = new StageFourDataset(false,
                                           currentBoardSize.x(),
                                           currentBoardSize.y(),
                                           currentModel.name(),
                                           i,
                                           threadCnt,
                                           false);
                break;
            default:
                QMessageBox::critical(this, tr("Self-Play error"), tr("Selected dataset generator is not supported"));
                QCoreApplication::quit();
        }

        threadGenerators.append(gen);
        if (i == 0) {
            gen->startConverter(iterationSize, datasetDirectory, false);
        }
        gen->setInputData(input);
        gen->setPolicyData(output);
        gen->setValueData(value);

        auto *thread = new QThread();
        auto *worker = new SelfPlayWorker(gen, i, iterationSize / threadCnt, currentModel, input, output, value);
        worker->moveToThread(thread);
        connect(thread, SIGNAL(started()), worker, SLOT(process()));
        connect(worker, SIGNAL(finished(int)), thread, SLOT(quit()));
        connect(worker, SIGNAL(finished(int)), worker, SLOT(deleteLater()));
        connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
        connect(worker, SIGNAL(progress(int, int)), this, SLOT(recvProgress(int, int)));
        connect(worker, SIGNAL(finished(int)), this, SLOT(threadFinished(int)));
        thread->start();
        threadRunning.append(true);
    }

    updateInfo();
}

void SelfPlay::recvProgress(int progress, int thread) {
    QMutexLocker locker(&recvProgressMutex);
    gamesCompleted++;
    if (gamesCompleted == iterationSize ||
        lastInfoUpdate.addMSecs(100) < QDateTime::currentDateTime() ||
        gamesCompleted % threadCnt == 0
    ) {
        //lastInfoUpdate = QDateTime::currentDateTime();
        updateInfo();
    }
}

void SelfPlay::threadFinished(int thread) {
    QMutexLocker locker(&threadFinishedMutex);

    threadRunning[thread] = false;
    bool done = true;
    for (const auto &i : threadRunning) {
        done = !i && done;
    }
    if (done) {
        finishIteration();
        if (iteration < iterationCnt) {
            setupIteration();
        } else {
            qDebug() << "last iteration done... waiting for training to finish...";
        }
    }
}

void SelfPlay::trainingFinished() {
    trainingStatusLbl->setText(tr("waiting for new data..."));
}

void SelfPlay::finishIteration() {
    currentBoardSize = availableBoardSizes[qrand() % availableBoardSizes.size()];

    // stop model process to free up gpu
    // (do this before saving data so give it a little time to stop)
    ModelManager::getInstance().stopAll();

    // save data
    if (!threadGenerators[0]->stopConverter()) {
        QCoreApplication::quit();
    }
    QString datasetPath = threadGenerators[0]->getDatasetPath();
    for (const auto &gen : threadGenerators) {
        gen->deleteLater();
    }
    threadGenerators.clear();

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
            << Settings::alphaDotsDir() + tr("/modelServer/models/") + currentModel.path()
            << tr("--targetmodel")
            << Settings::alphaDotsDir() + tr("/modelServer/models/") + ProtobufConnector::getInstance().getModelByName(targetModelName).path()
            //<< tr("--logdest")
            //<< logdest
            ;
    if (upload) {
        processArgs << tr("--upload");
    }
    QString processWorkingDirectory = Settings::alphaDotsDir() + tr("/modelServer/models/alphaZero");
    if (alphaZeroV10Training == nullptr) {
        alphaZeroV10Training = new ExternalProcess(processPath, processArgs, processWorkingDirectory);
    } else {
        if (alphaZeroV10Training->isRunning()) {
            /*
            QMessageBox::critical(this, tr("SelfPlay error"),
                                  tr("Training takes longer than generating data. sth seems wrong! Waiting for training to finish..."));
             */
            while (alphaZeroV10Training->isRunning()) {
                QThread::sleep(1);
                QCoreApplication::processEvents();
            }
        }
        disconnect(alphaZeroV10Training, SIGNAL(processFinished()), this, SLOT(trainingFinished()));
        alphaZeroV10Training->deleteLater();
        alphaZeroV10Training = new ExternalProcess(processPath, processArgs, processWorkingDirectory);
    }
    if (!trainOnGPU) {
        // disable gpu, training on very little data -> cpu is enough
        alphaZeroV10Training->addEnvironmentVariable(tr("CUDA_VISIBLE_DEVICES"), tr("-1"));
    }
    connect(alphaZeroV10Training, SIGNAL(processFinished()), this, SLOT(trainingFinished()));
    trainingStartTime = QDateTime::currentDateTime();
    if (!alphaZeroV10Training->startExternalProcess()) {
        QMessageBox::critical(this, tr("SelfPlay error"),
                              tr("Failed to start external process for training!"));
    }
    trainingStatusLbl->setText(tr("training..."));
    if (iteration == 0) {
        currentModel = ProtobufConnector::getInstance().getModelByName(targetModelName);
        //currentModel.setName(currentModel.name()+tr(".")+QString::number(iteration));
    }
    QFileInfo fi(ProtobufConnector::getInstance().getModelByName(targetModelName).path());
    trainingLogBasename = fi.baseName();
    QTimer::singleShot(100, this, SLOT(updateTrainingInfo()));
}

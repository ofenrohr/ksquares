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
#include "SelfPlay.h"
#include "SelfPlayWorker.h"

using namespace AlphaDots;

SelfPlay::SelfPlay(QString datasetDest, int threads, QString initialModelName, int gamesPerIteration) :
    KXmlGuiWindow(),
    m_view(new QWidget())
{
    qDebug() << "SelfPlay()";

    datasetDirectory = datasetDest;
    threadCnt = threads;

    currentModel = ProtobufConnector::getModelByName(initialModelName);
    iteration = 0;
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

    input = nullptr;
    output = nullptr;
    value = nullptr;

    alphaZeroV10Training = ExternalProcess::Ptr(nullptr);

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
    boardSizeLabel->setText(QStringLiteral("%1 x %2").arg(currentBoardSize.x()).arg(currentBoardSize.y()));
    iterationLabel->setText(QString::number(iteration));
    progressBar->setMinimum(0);
    progressBar->setMaximum(iterationSize);
    progressBar->setValue(gamesCompleted);
}

void SelfPlay::setupIteration() {
    iteration++;
    gamesCompleted = 0;

    // prepare the data containers
    if (input != nullptr && output != nullptr && value != nullptr) {
        delete input;
        delete output;
        delete value;
    }
    int w = MLImageGenerator::boxesToImgSize(currentBoardSize.x());
    int h = MLImageGenerator::boxesToImgSize(currentBoardSize.y());
    int imgDataSize = iterationSize * w * h;
    input = new std::vector<uint8_t>(imgDataSize);
    output = new std::vector<uint8_t>(imgDataSize);
    value = new std::vector<double >(iterationSize);

    // check and reset thread status
    for (const auto &i : threadRunning) {
        assert(!i);
    }
    threadRunning.clear();

    // start the threads
    int examplesCnt = 64; // todo: parameterize
    QString datasetDestDir = QStringLiteral(""); // todo: parameterize

    threadGenerators.clear();
    for (int i = 0; i < threadCnt; i++) {
        StageFourDataset::Ptr gen = StageFourDataset::Ptr(new StageFourDataset(false,
                                                                               currentBoardSize.x(),
                                                                               currentBoardSize.y(),
                                                                               currentModel.name(),
                                                                               i,
                                                                               threadCnt));
        threadGenerators.append(gen);
        if (i == 0) {
            gen->startConverter(examplesCnt, datasetDestDir);
            input = gen->getInputData();
            output = gen->getPolicyData();
            value = gen->getValueData();
        } else {
            gen->setInputData(input);
            gen->setPolicyData(output);
            gen->setValueData(value);
        }

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
    updateInfo();
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
        setupIteration();
    }
}

void SelfPlay::trainingFinished() {
    trainingStatusLbl->setText(tr("waiting for new data..."));
}

void SelfPlay::finishIteration() {
    // stop model process to free up gpu
    // (do this before saving data so give it a little time to stop)
    ModelManager::getInstance().stopAll();

    // save data
    threadGenerators[0]->stopConverter();
    QString datasetPath = threadGenerators[0]->getDatasetPath();

    // start training on new data
    QString processPath = Settings::pythonExecutable();
    QStringList processArgs;
    processArgs
            << tr("modelServer/models/alphaZeroV10.py")
            << tr("--dataset")
            << datasetPath
            << tr("--iteration")
            << QString::number(iteration)
            << tr("--epochs")
            << tr("10")
            ;
    if (alphaZeroV10Training.isNull()) {
        alphaZeroV10Training = ExternalProcess::Ptr(new ExternalProcess(processPath, processArgs));
    } else {
        if (alphaZeroV10Training->isRunning()) {
            QMessageBox::critical(this, tr("SelfPlay error"),
                                  tr("Training takes longer than generating data. sth seems wrong! Waiting for training to finish..."));
            while (alphaZeroV10Training->isRunning()) {
                QThread::sleep(1000);
            }
        }
        disconnect(alphaZeroV10Training.data(), SIGNAL(processFinished()), this, SLOT(trainingFinished()));
        alphaZeroV10Training = ExternalProcess::Ptr(new ExternalProcess(processPath, processArgs));
    }
    connect(alphaZeroV10Training.data(), SIGNAL(processFinished()), this, SLOT(trainingFinished()));
    if (!alphaZeroV10Training->startExternalProcess()) {
        QMessageBox::critical(this, tr("SelfPlay error"),
                              tr("Failed to start external process for training!"));
    }
    trainingStatusLbl->setText(tr("training..."));
}

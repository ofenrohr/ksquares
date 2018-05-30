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

SelfPlay::SelfPlay(QString datasetDest, int threads, QString initialModelName, QString targetModel, int gamesPerIteration) :
    KXmlGuiWindow(),
    m_view(new QWidget())
{
    qDebug() << "SelfPlay()";

    datasetDirectory = datasetDest;
    targetModelName = targetModel;
    threadCnt = threads;

    currentModel = ProtobufConnector::getInstance().getModelByName(initialModelName);
    iteration = -1;
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
        StageFourDataset *gen = new StageFourDataset(false,
                                               currentBoardSize.x(),
                                               currentBoardSize.y(),
                                               currentModel.name(),
                                               i,
                                               threadCnt);
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
    if (iteration == 0) {
        currentModel = ProtobufConnector::getInstance().getModelByName(targetModelName);
        //currentModel.setName(currentModel.name()+tr(".")+QString::number(iteration));
    }
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
            << tr("10")
            ;
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
    // disable gpu, training on very little data -> cpu is enough
    alphaZeroV10Training->addEnvironmentVariable(QStringLiteral("CUDA_VISIBLE_DEVICES"), QStringLiteral("-1"));
    connect(alphaZeroV10Training, SIGNAL(processFinished()), this, SLOT(trainingFinished()));
    if (!alphaZeroV10Training->startExternalProcess()) {
        QMessageBox::critical(this, tr("SelfPlay error"),
                              tr("Failed to start external process for training!"));
    }
    trainingStatusLbl->setText(tr("training..."));
}

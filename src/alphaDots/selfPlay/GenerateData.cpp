//
// Created by ofenrohr on 16.09.18.
//

#include <alphaDots/ProtobufConnector.h>
#include <alphaDots/MLImageGenerator.h>
#include <QMessageBox>
#include <QtCore/QDateTime>
#include <alphaDots/ModelManager.h>
#include "GenerateData.h"
#include "SelfPlayWorker.h"

using namespace AlphaDots;

GenerateData::GenerateData(QString &initialModelName, QPoint &boardSize, int gamesPerIteraion, DatasetType dataset,
                           int threads, QString &datasetDestination, KSquares::AILevel generatorAiLevel) {
    input = new std::vector<uint8_t>();
    output = new std::vector<uint8_t>();
    value = new std::vector<double >();

    currentModel = ProtobufConnector::getInstance().getModelByName(initialModelName);
    currentBoardSize = boardSize;
    gamesCompleted = 0;
    iterationSize = gamesPerIteraion;
    datasetType = dataset;
    threadCnt = threads;
    datasetDirectory = datasetDestination;
    aiLevel = generatorAiLevel;
}

GenerateData::~GenerateData() {
    delete input;
    delete output;
    delete value;
}

void GenerateData::startIteration() {
    gamesCompleted = 0;

    // prepare the data containers
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
                // gen will be deleted by ~SelfPlayWorker
                gen = new StageFourDataset(false,
                                           currentBoardSize.x(),
                                           currentBoardSize.y(),
                                           currentModel.name(),
                                           i,
                                           threadCnt,
                                           true,
                                           aiLevel);
                break;
            case StageFourNoMCTS:
                // gen will be deleted by ~SelfPlayWorker
                gen = new StageFourDataset(false,
                                           currentBoardSize.x(),
                                           currentBoardSize.y(),
                                           currentModel.name(),
                                           i,
                                           threadCnt,
                                           false,
                                           aiLevel);
                break;
            default:
                QMessageBox::critical(nullptr, tr("Self-Play error"), tr("Selected dataset generator is not supported"));
                QCoreApplication::quit();
                return;
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
}


void GenerateData::recvProgress(int progress, int thread) {
    QMutexLocker locker(&recvProgressMutex);
    gamesCompleted++;
    if (gamesCompleted == iterationSize ||
        lastInfoUpdate.addMSecs(100) < QDateTime::currentDateTime() ||
        gamesCompleted % threadCnt == 0
            ) {
        //lastInfoUpdate = QDateTime::currentDateTime();
        //updateDataGenInfo();
        emit infoChanged();
        lastInfoUpdate = QDateTime::currentDateTime();
    }
}

void GenerateData::threadFinished(int thread) {
    QMutexLocker locker(&threadFinishedMutex);

    threadRunning[thread] = false;
    bool done = true;
    for (const auto &i : threadRunning) {
        done = !i && done;
    }
    if (done) {
        finishIteration();
    }
}

void GenerateData::finishIteration() {
    // stop model process to free up gpu
    // (do this before saving data to give it a little time to stop)
    ModelManager::getInstance().stopAll();

    // save data
    if (!threadGenerators[0]->stopConverter()) {
        QCoreApplication::quit();
    }
    datasetPath = threadGenerators[0]->getDatasetPath();
    for (const auto &gen : threadGenerators) {
        gen->deleteLater();
    }
    threadGenerators.clear();

    emit iterationFinished();
}

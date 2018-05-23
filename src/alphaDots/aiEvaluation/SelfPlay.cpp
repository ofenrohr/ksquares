//
// Created by ofenrohr on 02.05.18.
//

#include <QtCore/QTimer>
#include <QtCore/QThread>
#include <QDebug>
#include <alphaDots/ProtobufConnector.h>
#include <alphaDots/MLImageGenerator.h>
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

    assert(iterationSize % threads == 0);

    QTimer::singleShot(0, this, &SelfPlay::initObject);
}

void SelfPlay::initObject() {
    qDebug() << "initObject()";

    setupUi(m_view);
    setCentralWidget(m_view);
    setupGUI();

    setupIteration();
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
    for (int i = 0; i < threadCnt; i++) {
        auto *thread = new QThread();
        auto *worker = new SelfPlayWorker(i, iterationSize / threadCnt, currentModel, input, output, value);
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

void SelfPlay::threadFinished(int thread) {
    threadRunning[thread] = false;
    bool done = true;
    for (const auto &i : threadRunning) {
        done = !i && done;
    }
    finishIteration();
    setupIteration();
}

void SelfPlay::finishIteration() {
    // save data
}

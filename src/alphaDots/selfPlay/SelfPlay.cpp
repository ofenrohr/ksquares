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

SelfPlay::SelfPlay(QString &datasetDest, int threads, QString &initialModelName, QString &targetModel,
                   int iterations, int gamesPerIteration, int epochs, bool gpuTraining, DatasetType dataset,
                   bool doUpload, QList<QPoint> &boardSizes, bool waitForTrainingToFinish) :
    KXmlGuiWindow(),
    m_view(new QWidget())
{
    qDebug() << "SelfPlay()";

    datasetDirectory = datasetDest;
    targetModelName = targetModel;
    threadCnt = threads;
    upload = doUpload;
    waitForTraining = waitForTrainingToFinish;

    iteration = -1;
    iterationCnt = iterations;

    availableBoardSizes = boardSizes;

    dataGen = new GenerateData(initialModelName, availableBoardSizes[0], gamesPerIteration, dataset, threads,
                               datasetDest);

    trainNetwork = new TrainNetwork(epochs, gpuTraining, doUpload, datasetDest);

    assert(dataGen->gamesPerIteration() % threads == 0);


    QTimer::singleShot(0, this, &SelfPlay::initObject);
}

void SelfPlay::initObject() {
    qDebug() << "initObject()";

    setupUi(m_view);
    setCentralWidget(m_view);
    setupGUI();

    connect(dataGen, SIGNAL(infoChanged()), this, SLOT(updateDataGenInfo()));
    connect(dataGen, SIGNAL(iterationFinished()), this, SLOT(generateDataFinished()));
    connect(trainNetwork, SIGNAL(infoChanged()), this, SLOT(updateTrainingInfo()));
    connect(trainNetwork, SIGNAL(trainingFinished()), this, SLOT(trainingFinished()));

    setupIteration();
    trainingStatusLbl->setText(tr("waiting for data..."));
}

void SelfPlay::updateDataGenInfo() {
    currentModelLabel->setText(dataGen->modelName());
    boardSizeLabel->setText(dataGen->boardSizeStr());
    QString prettySizes;
    bool first = true;
    for (QPoint p : availableBoardSizes) {
        if (first) {
            first = false;
        } else {
            prettySizes.append(tr(", "));
        }
        prettySizes.append(QString::number(p.x()) + tr("x") + QString::number(p.y()));
    }
    availableBoardSizesLbl->setText(prettySizes);
    iterationLabel->setText(QString::number(iteration));
    progressBar->setMinimum(0);
    progressBar->setMaximum(dataGen->gamesPerIteration());
    progressBar->setValue(dataGen->completedGames());
    datasetGeneratorLbl->setText(dataGen->getDatasetType() == StageFour ? tr("Stage Four") : tr("Stage Four (no MCTS)"));
}

void SelfPlay::updateTrainingInfo() {
    trainingStatusLbl->setText(trainNetwork->getStatusStr());
    logLinkLbl->setText(trainNetwork->getLogLink());
    logLinkLbl->setTextFormat(Qt::RichText);
    logLinkLbl->setTextInteractionFlags(Qt::TextBrowserInteraction);
    logLinkLbl->setOpenExternalLinks(true);
    epochLbl->setText(trainNetwork->getEpochStr());
    etaLbl->setText(trainNetwork->getEtaStr());
    lossLbl->setText(trainNetwork->getLossStr());
    policyLossLbl->setText(trainNetwork->getPolicyLossStr());
    valueLossLbl->setText(trainNetwork->getValueLossStr());
}

void SelfPlay::setupIteration() {
    // wait for training to finish? TODO
    if (waitForTraining && iteration >= 0) {
        if (trainNetwork->trainingInProgress()) {
            QTimer::singleShot(1000, this, SLOT(setupIteration()));
            return;
        }
    }

    iteration++;

    dataGen->setBoardSize(availableBoardSizes[qrand() % availableBoardSizes.size()]);
    dataGen->startIteration();

    updateDataGenInfo();
}


void SelfPlay::trainingFinished() {
    finishIteration();
}

void SelfPlay::finishIteration() {
    if (iteration == 0) {
        ModelInfo nextModel = ProtobufConnector::getInstance().getModelByName(targetModelName);
        dataGen->setCurrentModel(nextModel);
        //currentModel.setName(currentModel.name()+tr(".")+QString::number(iteration));
    }
    iteration++;
    qDebug() << "new iteration: " << iteration;
    if (iteration >= iterationCnt) {
        qDebug() << "last iteration done! exiting...";
        QCoreApplication::exit(0);
    }
    setupIteration();
}

void SelfPlay::generateDataFinished() {
    /*
    if (iteration < iterationCnt) {
        // TODO add evaluation!
        setupIteration();
    } else {
        qDebug() << "last iteration done... waiting for training to finish...";
    }
     */
    qDebug() << "generating data finished";
    trainNetwork->startTraining(dataGen->getDatasetPath(), iteration, dataGen->getCurrentModel().path(),
                                ProtobufConnector::getInstance().getModelByName(targetModelName).path(),
                                datasetDirectory);
}

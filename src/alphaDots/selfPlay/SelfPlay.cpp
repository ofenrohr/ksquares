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
                   bool doUpload, QList<QPoint> &boardSizes) :
    KXmlGuiWindow(),
    m_view(new QWidget())
{
    qDebug() << "[SelfPlay] SelfPlay()";

    datasetDirectory = datasetDest;
    targetModelName = targetModel;
    threadCnt = threads;
    upload = doUpload;

    iteration = 0;
    iterationCnt = iterations;

    availableBoardSizes = boardSizes;

    dataGen = new GenerateData(initialModelName, availableBoardSizes[0], gamesPerIteration, dataset, threads,
                               datasetDest);
    bestModel = dataGen->getCurrentModel();

    trainNetwork = new TrainNetwork(epochs, gpuTraining, doUpload, datasetDest);

    evaluateNetwork = new EvaluateNetwork(bestModel, threadCnt*2, threadCnt);

    assert(dataGen->gamesPerIteration() % threads == 0);


    QTimer::singleShot(0, this, &SelfPlay::initObject);
}

SelfPlay::~SelfPlay() {
    dataGen->deleteLater();
    trainNetwork->deleteLater();
    evaluateNetwork->deleteLater();
}

void SelfPlay::initObject() {
    qDebug() << "[SelfPlay] initObject()";

    setupUi(m_view);
    setCentralWidget(m_view);
    setupGUI();

    // connect self-play pipeline
    connect(dataGen, SIGNAL(infoChanged()), this, SLOT(updateDataGenInfo()));
    connect(dataGen, SIGNAL(iterationFinished()), this, SLOT(generateDataFinished()));
    connect(trainNetwork, SIGNAL(infoChanged()), this, SLOT(updateTrainingInfo()));
    connect(trainNetwork, SIGNAL(trainingFinished()), this, SLOT(trainingFinished()));
    connect(evaluateNetwork, SIGNAL(infoChanged()), this, SLOT(updateEvaluationInfo()));
    connect(evaluateNetwork, SIGNAL(evaluationFinished()), this, SLOT(evaluationFinished()));

    // set gui elements
    resultsTable->setModel(evaluateNetwork->getResultModel());
    resultsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    // set things in motion
    setupIteration();
}

void SelfPlay::setupIteration() {
    iteration++;

    dataGen->setCurrentModel(bestModel);
    dataGen->setBoardSize(availableBoardSizes[qrand() % availableBoardSizes.size()]);
    dataGen->startIteration();

    updateDataGenInfo();
    updateTrainingInfo();
    updateEvaluationInfo();
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

void SelfPlay::generateDataFinished() {
    qDebug() << "[SelfPlay] generating data finished";
    ModelInfo targetModel = ProtobufConnector::getInstance().getModelByName(targetModelName);

    QFileInfo targetModelPath(targetModel.path());

    contendingModel = ModelInfo(
            targetModelName + "_" + QString::number(iteration),
            targetModel.desc(),
            targetModelPath.dir().path() + "/" + targetModelPath.fileName().replace(QRegExp("\\.{0,1}\\d*\\.h5"), "." + QString::number(iteration) + ".h5"),
            targetModel.type(),
            targetModel.ai());
    ProtobufConnector::getInstance().addModelToList(contendingModel);
    qDebug() << "[SelfPlay] starting training";
    trainNetwork->startTraining(dataGen->getDatasetPath(), iteration, dataGen->getCurrentModel().path(),
                                contendingModel.path());
}

void SelfPlay::trainingFinished() {
    qDebug() << "[SelfPlay] training network finished";

    evaluateNetwork->startEvaluation(contendingModel);
}

void SelfPlay::updateEvaluationInfo() {
    bestModelLbl->setText(evaluateNetwork->getBestModel().name());
    evalModelLbl->setText(evaluateNetwork->getContendingModel().name());
}

void SelfPlay::evaluationFinished() {
    qDebug() << "[SelfPlay] evaluation finished";
    bestModel = evaluateNetwork->getBestModel();
    qDebug() << "best model: " << bestModel.name();
    finishIteration();
}

void SelfPlay::finishIteration() {
    dataGen->setCurrentModel(bestModel);
    qDebug() << "[SelfPlay] new iteration: " << iteration;
    if (iteration >= iterationCnt) {
        qDebug() << "[SelfPlay] last iteration done! exiting...";
        QCoreApplication::exit(0);
    } else {
        setupIteration();
    }
}


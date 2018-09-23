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
#include <alphaDots/AlphaDotsExceptions.h>
#include <aiAlphaZeroMCTS.h>
#include "SelfPlay.h"
#include "SelfPlayWorker.h"

using namespace AlphaDots;

SelfPlay::SelfPlay(QString &datasetDest, int threads, QString &initialModelName, QString &targetModel,
                   int iterations, int gamesPerIteration, int epochs, bool gpuTraining, DatasetType dataset,
                   bool doUpload, QList<QPoint> &boardSizes, int evalGames, bool noEval) :
    KXmlGuiWindow(),
    m_view(new QWidget())
{
    qDebug() << "[SelfPlay] SelfPlay()";

    datasetDirectory = datasetDest;
    targetModelName = targetModel;
    threadCnt = threads;
    upload = doUpload;
    disableEvaluation = noEval;

    iteration = 0;
    iterationCnt = iterations;

    availableBoardSizes = boardSizes;

    dataGen = new GenerateData(initialModelName, availableBoardSizes[0], gamesPerIteration, dataset, threads,
                               datasetDest);
    bestModel = dataGen->getCurrentModel();

    trainNetwork = new TrainNetwork(epochs, gpuTraining, doUpload, datasetDest);

    evaluateNetwork = new EvaluateNetwork(bestModel, evalGames, threadCnt);

    assert(dataGen->gamesPerIteration() % threads == 0);

    // init rng
    rng = gsl_rng_alloc(gsl_rng_taus);
    gsl_rng_set(rng, std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count());

    QTimer::singleShot(0, this, &SelfPlay::initObject);
}

SelfPlay::~SelfPlay() {
    dataGen->deleteLater();
    trainNetwork->deleteLater();
    evaluateNetwork->deleteLater();
    gsl_rng_free(rng);
}

void SelfPlay::initObject() {
    qDebug() << "[SelfPlay] initObject()";

    // setup gui stuff
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

    if (disableEvaluation) {
        evalBox->setVisible(false);
        evaluateLbl->setVisible(false);
    }

    // set things in motion
    setupIteration();
}

void SelfPlay::updateOverview() {
    switch (mode) {
        case GENERATE:
            generateLbl->setText("<span style=\"text-decoration: underline;\">Generate</span>");
            trainLbl->setText("Train");
            evaluateLbl->setText("Evaluate");
            break;
        case TRAIN:
            generateLbl->setText("Generate");
            trainLbl->setText("<span style=\"text-decoration: underline;\">Train</span>");
            evaluateLbl->setText("Evaluate");
            break;
        case EVALUATE:
            generateLbl->setText("Generate");
            trainLbl->setText("Train");
            evaluateLbl->setText("<span style=\"text-decoration: underline;\">Evaluate</span>");
            break;
        default:
            qDebug() << "unknown mode!";
            assert(false);
    }
}

void SelfPlay::setupIteration() {
    iteration++;

    mode = GENERATE;

    aiAlphaZeroMCTS::use_probabilistic_final_move_selection = true;

    dataGen->setCurrentModel(bestModel);
    //dataGen->setBoardSize(availableBoardSizes[gsl_rng_uniform_int(rng, availableBoardSizes.size())]);
    dataGen->setBoardSize(availableBoardSizes[iteration % availableBoardSizes.size()]);
    dataGen->startIteration();

    updateOverview();
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

void SelfPlay::generateDataFinished() {
    qDebug() << "[SelfPlay] generating data finished";
    ModelInfo targetModel;
    try {
        targetModel = ProtobufConnector::getInstance().getModelByName(targetModelName);
    } catch (ModelNotFoundException &ex) {
        qDebug() << "[SelfPlay] target model does not exist! creating a new model!";
        if (targetModelName.trimmed().isEmpty()) {
            qDebug() << "[SelfPlay] target model name is empty! creating new name";
            targetModelName = QUuid::createUuid().toString(QUuid::WithoutBraces);
            qDebug() << "[SelfPlay] target model name: " << targetModelName;
        }
        QFileInfo targetModelPathFI(bestModel.path());
        QString targetModelPath = targetModelPathFI.dir().path() + "/" + targetModelName + ".h5";
        targetModel = ModelInfo(targetModelName, "Created by self-play mode in KSquares", targetModelPath, bestModel.type(), bestModel.ai());
        ProtobufConnector::getInstance().addModelToList(targetModel);
    }

    QFileInfo targetModelPath(targetModel.path());

    mode = TRAIN;
    updateOverview();

    contendingModel = ModelInfo(
            targetModelName + "_" + QString::number(iteration),
            targetModel.desc(),
            targetModelPath.dir().path() + "/" + targetModelPath.fileName().replace(QRegExp("(\\.\\d+){0,1}\\.h5"), "." + QString::number(iteration) + ".h5"),
            targetModel.type(),
            targetModel.ai());
    ProtobufConnector::getInstance().addModelToList(contendingModel);
    qDebug() << "[SelfPlay] starting training";
    trainNetwork->startTraining(dataGen->getDatasetPath(), iteration, dataGen->getCurrentModel().path(),
                                contendingModel.path());
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

void SelfPlay::trainingFinished() {
    qDebug() << "[SelfPlay] training network finished";

    mode = EVALUATE;
    updateOverview();

    aiAlphaZeroMCTS::use_probabilistic_final_move_selection = false;

    if (disableEvaluation) {
        qDebug() << "[SelfPlay] skipping evaluation";
        bestModel = contendingModel;
        finishIteration();
    } else {
        evaluateNetwork->startEvaluation(contendingModel);
    }
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


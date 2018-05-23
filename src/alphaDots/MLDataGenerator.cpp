//
// Created by ofenrohr on 10/15/17.
//

#include "MLDataGenerator.h"

#include <QtCore/QTimer>
#include <QtCore/QUuid>
#include <alphaDots/datasets/DatasetGenerator.h>
#include <alphaDots/datasets/FirstTryDataset.h>
#include <alphaDots/datasets/StageOneDataset.h>
#include <alphaDots/datasets/BasicStrategyDataset.h>
#include <alphaDots/datasets/SequenceDataset.h>
#include <alphaDots/datasets/TrainingSequenceDataset.h>
#include <QtWidgets/QMessageBox>
#include <alphaDots/datasets/StageTwoDataset.h>
#include <alphaDots/datasets/StageThreeDataset.h>
#include <cmath>
#include <alphaDots/datasets/StageFourDataset.h>
#include "aiEasyMediumHard.h"
#include "MLDataGeneratorWorkerThread.h"
#include "ExternalProcess.h"
#include "MLImageGenerator.h"

using namespace AlphaDots;

MLDataGenerator::MLDataGenerator(DatasetType datasetType, int width, int height) : KXmlGuiWindow(), m_view(new QWidget()) {
    threadCnt = 4;
    examplesCnt = -1;
    generateDatasetType = datasetType;
    datasetWidth = width;
    datasetHeight = height;
    datasetDestDir = QStringLiteral("./");
    initConstructor();
}

MLDataGenerator::MLDataGenerator(long samples, DatasetType datasetType, int width, int height, QString destDir, int threads) : KXmlGuiWindow(), m_view(new QWidget()) {
    qDebug() << "auto generate mode";
    examplesCnt = samples;
    generateDatasetType = datasetType;
    datasetWidth = width;
    datasetHeight = height;
    datasetDestDir = destDir;
    threadCnt = threads;
    initConstructor();
}

MLDataGenerator::~MLDataGenerator() {}

void MLDataGenerator::initConstructor() {
    qDebug() << "setting up dataset generator";
    qDebug() << " |-> samples = " << examplesCnt;
    qDebug() << " |-> threads = " << threadCnt;
    qDebug() << " |-> dataset type = " << generateDatasetType;
    qDebug() << " |-> width = " << datasetWidth;
    qDebug() << " |-> height = " << datasetHeight;
    qDebug() << " |-> destination directory = " << datasetDestDir;
    gbs = nullptr;
    threadProgr.clear();

    for (int i = 0; i < threadCnt; i++) {
        threadProgr.append(0);
    }

    setupUi(m_view);
    setCentralWidget(m_view);
    setupGUI();

    // next button
    connect(nextBtn, SIGNAL(clicked()), this, SLOT(nextBtnClicked()));
    connect(generatorSelector, SIGNAL(currentIndexChanged(int)), this, SLOT(selectGenerator(int)));

    // turn slider
    connect(turnSlider, SIGNAL(valueChanged(int)), this, SLOT(turnSliderChanged(int)));

    QTimer::singleShot(0, this, &MLDataGenerator::initObject);
}

void MLDataGenerator::selectGenerator(int gen) {
    qDebug() << "selectGenerator(" << gen << ")";
    switch (gen) {
        case 0:
            guiGenerator = DatasetGenerator::Ptr(new FirstTryDataset(datasetWidth, datasetHeight, QStringLiteral("")));
            qDebug() << "selected first try generator";
            break;
        case 1:
            guiGenerator = DatasetGenerator::Ptr(new StageOneDataset(true, datasetWidth, datasetHeight));
            qDebug() << "selected stage one generator";
            break;
        case 2:
            guiGenerator = DatasetGenerator::Ptr(new BasicStrategyDataset(true, datasetWidth, datasetHeight));
            qDebug() << "selected basic strategy generator";
            break;
        case 3:
            guiGenerator = DatasetGenerator::Ptr(new SequenceDataset(true, datasetWidth, datasetHeight));
            qDebug() << "selected sequence generator";
            break;
        case 4:
            guiGenerator = DatasetGenerator::Ptr(new TrainingSequenceDataset(true, datasetWidth, datasetHeight));
            qDebug() << "selected training sequence generator";
            break;
        case 5:
            guiGenerator = DatasetGenerator::Ptr(new StageThreeDataset(true, datasetWidth, datasetHeight));
            qDebug() << "selected stage three generator";
            break;
        case 6:
            guiGenerator = DatasetGenerator::Ptr(new StageFourDataset(true, datasetWidth, datasetHeight, tr("AlphaZeroV7")));
            qDebug() << "selected stage four generator";
            break;
        default:
            break;
    }
}

void MLDataGenerator::initObject() {
    int generatorIndex = 0;
    switch (generateDatasetType) {
        case FirstTry: generatorIndex = 0; break;
        case StageOne: generatorIndex = 1; break;
        case BasicStrategy: generatorIndex = 2; break;
        case LSTM: generatorIndex = 3; break;
        case LSTM2: generatorIndex = 4; break;
        case StageTwo: generatorIndex = 1; break;
        case StageThree: generatorIndex = 5; break;
        case StageFour: generatorIndex = 6; break;
        default:
            QMessageBox::critical(this, tr("Error"), tr("[MLDataGenerator] Unknown dataset type"));
            generatorIndex = 0;
    }
    selectGenerator(generatorIndex);
    generatorSelector->setCurrentIndex(generatorIndex);

    generateGUIexample();

    setupGeneratorThreads();
}

DatasetGenerator::Ptr MLDataGenerator::getDatasetGenerator(int thread) {
    switch (generateDatasetType) {
        case FirstTry:
            return FirstTryDataset::Ptr(new FirstTryDataset(datasetWidth, datasetHeight, datasetDestDir));
        case StageOne:
            return StageOneDataset::Ptr(new StageOneDataset(false, datasetWidth, datasetHeight));
        case BasicStrategy:
            return BasicStrategyDataset::Ptr(new BasicStrategyDataset(false, datasetWidth, datasetHeight));
        case LSTM:
            return SequenceDataset::Ptr(new SequenceDataset(false, datasetWidth, datasetHeight));
        case LSTM2:
            return TrainingSequenceDataset::Ptr(new TrainingSequenceDataset(false, datasetWidth, datasetHeight));
        case StageTwo:
            return StageTwoDataset::Ptr(new StageTwoDataset(false, datasetWidth, datasetHeight, thread, threadCnt));
        case StageThree:
            return StageThreeDataset::Ptr(new StageThreeDataset(false, datasetWidth, datasetHeight, thread, threadCnt));
        case StageFour:
            return StageFourDataset::Ptr(new StageFourDataset(false, datasetWidth, datasetHeight, tr("AlphaZeroV7"), thread, threadCnt));
    }
    return DatasetGenerator::Ptr(nullptr);
}

void MLDataGenerator::setupGeneratorThreads() {
    qDebug() << examplesCnt;
    if (threadGenerators.count() > 0) {
        qDebug() << "ERROR: thread list is not empty!";
        return;
    }
    if (examplesCnt > 0) {
        nextBtn->setEnabled(false);
        progressBar->setValue(0);

        std::vector<uint8_t> *inputData = nullptr;
        std::vector<uint8_t> *policyData = nullptr;
        std::vector<double> *valueData = nullptr;

        for (int i = 0; i < threadCnt; i++) {
            DatasetGenerator::Ptr gen = getDatasetGenerator(i);
            threadGenerators.append(gen);
            if (i == 0) {
                gen->startConverter(examplesCnt, datasetDestDir);
                inputData = gen->getInputData();
                policyData = gen->getPolicyData();
                valueData = gen->getValueData();
            } else {
                gen->setInputData(inputData);
                gen->setPolicyData(policyData);
                gen->setValueData(valueData);
            }
            // https://mayaposch.wordpress.com/2011/11/01/how-to-really-truly-use-qthreads-the-full-explanation/
            auto *thread = new QThread;
            auto *worker = new MLDataGeneratorWorkerThread(examplesCnt / threadCnt, gen, i);
            worker->moveToThread(thread);
            //connect(worker, SIGNAL(error(QString)), this, SLOT(errorString(QString)));
            connect(thread, SIGNAL(started()), worker, SLOT(process()));
            connect(worker, SIGNAL(finished(int)), thread, SLOT(quit()));
            connect(worker, SIGNAL(finished(int)), worker, SLOT(deleteLater()));
            connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
            connect(worker, SIGNAL(progress(int, int)), this, SLOT(recvProgress(int, int)));
            connect(worker, SIGNAL(finished(int)), this, SLOT(dataGeneratorFinished(int)));
            //connect(generator.data(), SIGNAL(sendGUIsample(aiBoard::Ptr, QImage, QImage)), this, SLOT(setGUIgame(aiBoard::Ptr, QImage, QImage)));
            thread->start();
        }
        runningThreadCnt = threadCnt;
    } else {
        progressBar->setVisible(false);
    }

}

void MLDataGenerator::recvProgress(int progress, int thread) {
    threadProgr[thread] = progress;
    int sum = 0;
    for (int i = 0; i < threadProgr.count(); i++) {
        sum += threadProgr[i];
    }
    //qDebug() << threadProgr;
    progressBar->setValue(sum / threadCnt);
}

void MLDataGenerator::dataGeneratorFinished(int threadIdx) {
    threadGenerators[threadIdx]->cleanup();
    runningThreadCnt--;
    if (runningThreadCnt <= 0) {
        threadGenerators[0]->stopConverter();
        nextBtn->setEnabled(true);
        //progressBar->setVisible(false);
        qDebug() << "done generating data";
        QCoreApplication::quit();
    }
}

void MLDataGenerator::generateGUIexample() {
    statusLbl->setText(tr("generating example..."));

    frameCnt = aiFunctions::toLinesSize(datasetWidth, datasetHeight);
    displayFrame = -1;

    guiDataset = guiGenerator->generateDataset();
    aiBoard::Ptr board;
    value = -INFINITY;
    if (guiDataset.isValid()) {
        qDebug() << "generator dataset";
        board = guiDataset.getBoard();
        if (guiDataset.isSequence()) {
            const QList<QImage> &seq = guiDataset.getSequence();
            int frameIdx = rand() % seq.count();
            inputImage = seq.at(frameIdx);
            outputImage = seq.at(frameIdx);
            displayFrame = frameIdx;
            turnSlider->setEnabled(true);
        } else if (guiDataset.isTrainingSequence()) {
            const QList<QImage> &inputSeq = guiDataset.getInputSequence();
            const QList<QImage> &targetSeq = guiDataset.getTargetSequence();
            int frameIdx = rand() % inputSeq.count();
            inputImage = inputSeq.at(frameIdx);
            outputImage = targetSeq.at(frameIdx);
            displayFrame = frameIdx;
            turnSlider->setEnabled(true);
        } else if (guiDataset.isPolicyValue()) {
            inputImage = guiDataset.getInputImg();
            outputImage = guiDataset.getOutputImg();
            displayFrame = frameCnt - aiFunctions::getFreeLines(guiDataset.getBoard()->lines, frameCnt).count();
            value = guiDataset.getOutputVal();
            turnSlider->setEnabled(false);
        } else {
            inputImage = guiDataset.getInputImg();
            outputImage = guiDataset.getOutputImg();
            displayFrame = frameCnt - aiFunctions::getFreeLines(guiDataset.getBoard()->lines, frameCnt).count();
            turnSlider->setEnabled(false);
        }
    } else {
        QMessageBox::critical(this, tr("Error"), tr("Not using the selected dataset generator!"));
        qDebug() << "NOT generator dataset";
        // generate data
        board = generateRandomBoard(datasetWidth, datasetHeight, 5);

        displayFrame = 20;

        // make some more moves
        KSquaresAi::Ptr ai = KSquaresAi::Ptr(new aiEasyMediumHard(0, datasetWidth, datasetHeight, 2));
        makeAiMoves(board, ai, displayFrame);

        inputImage = MLImageGenerator::generateInputImage(board);
        outputImage = MLImageGenerator::generateOutputImage(board, ai);
    }


    // print board
    qDebug().noquote().nospace() << aiFunctions::boardToString(board);
    qDebug() << "value:" << value;

    // update ksquares board scene
    updateGameBoardScene(board);

    // display board images
    inputLbl->setMinimumSize(inputImage.width()*10, inputImage.height()*10);
    inputLbl->setPixmap(QPixmap::fromImage(inputImage).scaled(inputLbl->width(), inputLbl->height(), Qt::KeepAspectRatio));
    outputLbl->setPixmap(
            QPixmap::fromImage(outputImage).scaled(outputLbl->width(), outputLbl->height(), Qt::KeepAspectRatio));
    outputLbl->setMinimumSize(outputImage.width()*10, outputImage.height()*10);
    valueLbl->setText(QString::number(value));

    // display frame progress
    turnSlider->setMaximum(frameCnt-1);
    turnSlider->setValue(displayFrame);

    statusLbl->setText(tr(""));
}

void MLDataGenerator::updateGameBoardScene(aiBoard::Ptr board) {
    // draw stuff
    if (gbs != NULL) {
        delete gbs;
    }
    gbs = new GameBoardScene(datasetWidth, datasetHeight, false, this);
    // send board to game board view
    for (int i = 0; i < board->linesSize; i++) {
        if (board->lines[i]) {
            gbs->drawLine(i, QColor::fromRgb(0, 0, 0));
        }
    }
    for (int i = 0; i < board->width * board->height; i++) {
        if (board->squareOwners[i] >= 0) {
            gbs->drawSquare(i,
                            board->squareOwners[i] == 0 ? QColor::fromRgb(255, 0, 0) : QColor::fromRgb(0, 0, 255));
        }
    }
    gameStateView->setScene(gbs);

}

void MLDataGenerator::nextBtnClicked() {
    generateGUIexample();
}

void MLDataGenerator::turnSliderChanged(int turnIdx) {
    //qDebug() << "turnSliderChanged: " << turnIdx;
    displayFrame = turnIdx;
    turnLbl->setText(QString::number(displayFrame) + QStringLiteral(" / ") + QString::number(frameCnt));

    //updateGameBoardScene(guiDataset.getBoard());

    if (guiDataset.isTrainingSequence()) {
        const QList<QImage> &inputSeq = guiDataset.getInputSequence();
        const QList<QImage> &targetSeq = guiDataset.getTargetSequence();
        inputImage = inputSeq.at(displayFrame);
        outputImage = targetSeq.at(displayFrame);
        inputLbl->setPixmap(QPixmap::fromImage(inputImage).scaled(inputLbl->width(), inputLbl->height(), Qt::KeepAspectRatio));
        outputLbl->setPixmap(
                QPixmap::fromImage(outputImage).scaled(outputLbl->width(), outputLbl->height(), Qt::KeepAspectRatio));
    }

    if (guiDataset.isSequence()) {
        const QList<QImage> &seq = guiDataset.getSequence();
        if (displayFrame == 0) {
            displayFrame += 1;
        }
        inputImage = seq.at(displayFrame-1);
        outputImage = seq.at(displayFrame);
        inputLbl->setPixmap(QPixmap::fromImage(inputImage).scaled(inputLbl->width(), inputLbl->height(), Qt::KeepAspectRatio));
        outputLbl->setPixmap(
                QPixmap::fromImage(outputImage).scaled(outputLbl->width(), outputLbl->height(), Qt::KeepAspectRatio));
    }
}

aiBoard::Ptr MLDataGenerator::createEmptyBoard(int width, int height) {
    int linesSize = aiFunctions::toLinesSize(width, height);
    bool *lines = new bool[linesSize];
    for (int i = 0; i < linesSize; i++) {
        lines[i] = false;
    }
    QList<int> squareOwners;
    for (int i = 0; i < width*height; i++) {
        squareOwners.append(-1);
    }
    aiBoard::Ptr board = aiBoard::Ptr(new aiBoard(lines, linesSize, width, height, squareOwners, 0, 1));
    return board;
}

aiBoard::Ptr MLDataGenerator::generateRandomBoard(int width, int height, int safeMoves) {
    aiBoard::Ptr board = createEmptyBoard(width, height);
    // generate the board with auto fill
    QList<int> autoFillLines = aiController::autoFill(safeMoves, width, height);
    foreach (int line, autoFillLines) {
        board->doMove(line);
    }
    return board;
}

int MLDataGenerator::makeAiMove(aiBoard::Ptr board, KSquaresAi::Ptr ai) {
    int nextLine = ai->chooseLine(board->linesAsList(), board->squareOwners, QList<Board::Move>());//board->lines);
    board->doMove(nextLine);
    return nextLine;
}

QList<int> MLDataGenerator::makeAiMoves(aiBoard::Ptr board, KSquaresAi::Ptr ai, int freeLinesLeft) {
    QList<int> moves;
    int freeLines = aiFunctions::getFreeLines(board->lines, board->linesSize).count();
    int i = 0;
    while (freeLines - i > freeLinesLeft) {
        moves.append(makeAiMove(board, ai));
        i++;
    }
    return moves;
}

void MLDataGenerator::saveImage(QString dataSetName, QString instanceName, QString dest, QImage &img) {
    QString filename = dest + QStringLiteral("/") + dataSetName + instanceName + QStringLiteral(".png");
    img.save(filename);
}

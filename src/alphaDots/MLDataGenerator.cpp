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
#include "aiEasyMediumHard.h"
#include "MLDataGeneratorWorkerThread.h"
#include "ExternalProcess.h"

using namespace AlphaDots;

MLDataGenerator::MLDataGenerator() : KXmlGuiWindow(), m_view(new QWidget()) {
    examplesCnt = -1;
    generateDatasetType = FirstTry;
    datasetWidth = 5;
    datasetHeight = 4;
    datasetDestDir = QStringLiteral("./");
    initConstructor();
}

MLDataGenerator::MLDataGenerator(long samples, DatasetType datasetType, int width, int height, QString destDir) : KXmlGuiWindow(), m_view(new QWidget()) {
    qDebug() << "auto generate mode";
    examplesCnt = samples;
    generateDatasetType = datasetType;
    datasetWidth = width;
    datasetHeight = height;
    datasetDestDir = destDir;
    initConstructor();
}

MLDataGenerator::~MLDataGenerator() {}

void MLDataGenerator::initConstructor() {
    qDebug() << "setting up dataset generator";
    qDebug() << " |-> samples = " << examplesCnt;
    qDebug() << " |-> dataset type = " << generateDatasetType;
    qDebug() << " |-> width = " << datasetWidth;
    qDebug() << " |-> height = " << datasetHeight;
    qDebug() << " |-> destination directory = " << datasetDestDir;
    gbs = nullptr;
    selectGenerator(0);
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
        default:
            break;
    }
}

void MLDataGenerator::initObject() {
    generateGUIexample();

    setupGeneratorThreads();
}

DatasetGenerator::Ptr MLDataGenerator::getDatasetGenerator() {
    switch (generateDatasetType) {
        case FirstTry:
            return getFirstTryDatasetGenerator();
        case StageOne:
            return getStageOneDatasetGenerator();
        case BasicStrategy:
            return getBasicStrategyDatasetGenerator();
        case LSTM:
            return getSequenceDatasetGenerator();
        case LSTM2:
            return getTrainingSequenceDatasetGenerator();
    }
    return DatasetGenerator::Ptr(nullptr);
}

DatasetGenerator::Ptr MLDataGenerator::getFirstTryDatasetGenerator() {
    FirstTryDataset::Ptr generator = FirstTryDataset::Ptr(new FirstTryDataset(datasetWidth, datasetHeight, datasetDestDir));
    return generator;
}

DatasetGenerator::Ptr MLDataGenerator::getStageOneDatasetGenerator() {
    StageOneDataset::Ptr gen = StageOneDataset::Ptr(new StageOneDataset(false, datasetWidth, datasetHeight));
    return gen;
}

DatasetGenerator::Ptr MLDataGenerator::getBasicStrategyDatasetGenerator() {
    BasicStrategyDataset::Ptr gen = BasicStrategyDataset::Ptr(new BasicStrategyDataset(false, datasetWidth, datasetHeight));
    return gen;
}

DatasetGenerator::Ptr MLDataGenerator::getSequenceDatasetGenerator() {
    SequenceDataset::Ptr gen = SequenceDataset::Ptr(new SequenceDataset(false, datasetWidth, datasetHeight));
    return gen;
}

DatasetGenerator::Ptr MLDataGenerator::getTrainingSequenceDatasetGenerator() {
    TrainingSequenceDataset::Ptr gen = TrainingSequenceDataset::Ptr(new TrainingSequenceDataset(false, datasetWidth, datasetHeight));
    return gen;
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

        for (int i = 0; i < threadCnt; i++) {
            DatasetGenerator::Ptr gen = getDatasetGenerator();
            threadGenerators.append(gen);
            if (i == 0) {
                gen->startConverter(examplesCnt, datasetDestDir);
            }
            // https://mayaposch.wordpress.com/2011/11/01/how-to-really-truly-use-qthreads-the-full-explanation/
            QThread *thread = new QThread;
            MLDataGeneratorWorkerThread *worker = new MLDataGeneratorWorkerThread(examplesCnt / threadCnt, gen, i);
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
        nextBtn->setEnabled(true);
        //progressBar->setVisible(false);
        qDebug() << "done generating data";
    }
}

void MLDataGenerator::generateGUIexample() {
    frameCnt = aiFunctions::toLinesSize(datasetWidth, datasetHeight);
    displayFrame = -1;

    // draw stuff
    if (gbs != NULL) {
        delete gbs;
    }
    gbs = new GameBoardScene(datasetWidth, datasetWidth, false, this);

    guiDataset = guiGenerator->generateDataset();
    aiBoard::Ptr board;
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
        } else {
            inputImage = guiDataset.getInputImg();
            outputImage = guiDataset.getOutputImg();
            displayFrame = frameCnt - aiFunctions::getFreeLines(guiDataset.getBoard()->lines, frameCnt).count();
            turnSlider->setEnabled(false);
        }
    } else {
        qDebug() << "NOT generator dataset";
        // generate data
        board = generateRandomBoard(datasetWidth, datasetHeight, 5);

        displayFrame = 20;

        // make some more moves
        KSquaresAi::Ptr ai = KSquaresAi::Ptr(new aiEasyMediumHard(0, datasetWidth, datasetHeight, 2));
        makeAiMoves(board, ai, displayFrame);

        inputImage = generateInputImage(board);
        outputImage = generateOutputImage(board, ai);
    }


    // print board
    qDebug().noquote().nospace() << aiFunctions::boardToString(board);

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

    // display board images
    inputLbl->setPixmap(QPixmap::fromImage(inputImage).scaled(inputLbl->width(), inputLbl->height(), Qt::KeepAspectRatio));
    outputLbl->setPixmap(
            QPixmap::fromImage(outputImage).scaled(outputLbl->width(), outputLbl->height(), Qt::KeepAspectRatio));

    // display frame progress
    turnSlider->setMaximum(frameCnt-1);
    turnSlider->setValue(displayFrame);

    // save boards
    /*
    QUuid id = QUuid::createUuid();
    saveImage(QStringLiteral("firstTry_5x4"), id.toString() + QStringLiteral("input"), QStringLiteral("/home/ofenrohr/arbeit/master/data"),
              inputImage);
    saveImage(QStringLiteral("firstTry_5x4"), id.toString() + QStringLiteral("output_hardai"), QStringLiteral("/home/ofenrohr/arbeit/master/data"),
              outputImage);
    */
}

void MLDataGenerator::nextBtnClicked() {
    generateGUIexample();
}

void MLDataGenerator::turnSliderChanged(int turnIdx) {
    //qDebug() << "turnSliderChanged: " << turnIdx;
    displayFrame = turnIdx;
    turnLbl->setText(QString::number(displayFrame) + QStringLiteral(" / ") + QString::number(frameCnt));

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
    while (aiFunctions::getFreeLines(board->lines, board->linesSize).count() > freeLinesLeft) {
        moves.append(makeAiMove(board, ai));
    }
    return moves;
}

void MLDataGenerator::drawBackgroundAndDots(QImage &img, bool drawDots) {
    for (int y = 0; y < img.height(); y++) {
        for (int x = 0; x < img.width(); x++) {
            int color = MLImageBackground;

            if (x % 2 == 1 && y % 2 == 1 && drawDots) {
                color = MLImageDot;
            }

            img.setPixel(x,y, qRgb(color, color, color));
        }
    }
}

void MLDataGenerator::drawLineAt(QImage &img, int lineIdx, int w, int h) {
    QPoint p1,p2;
    if (!Board::indexToPoints(lineIdx, &p1, &p2, w, h, false)) {
        qDebug() << "fail!";
    }
    img.setPixel(p2.x()*2+(p2.y()-p1.y()), p2.y()*2+(p2.x()-p1.x()), qRgb(MLImageLine,MLImageLine,MLImageLine));
}

void MLDataGenerator::drawLines(QImage &img, aiBoard::Ptr board) {
    for (int i = 0; i < board->linesSize; i++) {
        if (board->lines[i]) {
            drawLineAt(img, i, board->width, board->height);
        }
    }
}

void MLDataGenerator::drawBoxes(QImage &img, aiBoard::Ptr board) {
    for (int i = 0; i < board->squareOwners.count(); i++) {
        int squareOwner = board->squareOwners[i];
        if (squareOwner >= 0) {
            int x,y,c;
            x = (i % board->width) * 2 + 2;
            y = (i / board->width) * 2 + 2;
            c = squareOwner == 0 ? MLImageBoxA : MLImageBoxB;
            img.setPixel(x,y, qRgb(c,c,c));
        }
    }
}

int MLDataGenerator::boxesToImgSize(int boxes) {
    return boxes * 2 + 3;
}

QImage MLDataGenerator::generateInputImage(aiBoard::Ptr board) {
    int imgWidth = boxesToImgSize(board->width); // 1px border
    int imgHeight = boxesToImgSize(board->height);

    QImage img(imgWidth, imgHeight, QImage::Format_ARGB32);

    drawBackgroundAndDots(img);
    drawLines(img, board);
    drawBoxes(img, board);

    return img;
}

QImage MLDataGenerator::generateOutputImage(aiBoard::Ptr board, KSquaresAi::Ptr ai) {
    int imgWidth = board->width*2+3; // 1px border
    int imgHeight = board->height*2+3;

    QImage img(imgWidth, imgHeight, QImage::Format_ARGB32);

    int nextLine = ai->chooseLine(board->linesAsList(), board->squareOwners, QList<Board::Move>());

    drawBackgroundAndDots(img, false);
    drawLineAt(img, nextLine, board->width, board->height);

    return img;
}

QImage MLDataGenerator::generateOutputImage(aiBoard::Ptr board, QList<int> lines, bool drawDots) {
    int imgWidth = board->width*2+3; // 1px border
    int imgHeight = board->height*2+3;

    QImage img(imgWidth, imgHeight, QImage::Format_ARGB32);

    drawBackgroundAndDots(img, drawDots);
    foreach (int line, lines) {
        drawLineAt(img, line, board->width, board->height);
    }

    return img;
}

void MLDataGenerator::saveImage(QString dataSetName, QString instanceName, QString dest, QImage &img) {
    QString filename = dest + QStringLiteral("/") + dataSetName + instanceName + QStringLiteral(".png");
    img.save(filename);
}

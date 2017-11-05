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
#include "aiEasyMediumHard.h"
#include "MLDataGeneratorWorkerThread.h"
#include "ExternalProcess.h"

using namespace AlphaDots;

MLDataGenerator::MLDataGenerator() : KXmlGuiWindow(), m_view(new QWidget()) {
    examplesCnt = -1;
    initConstructor();
}

MLDataGenerator::MLDataGenerator(long samples) : KXmlGuiWindow(), m_view(new QWidget()) {
    qDebug() << "auto generate mode";
    examplesCnt = samples;
    initConstructor();
}

MLDataGenerator::~MLDataGenerator() {}

void MLDataGenerator::initConstructor() {
    gbs = NULL;
    //guiGenerator = DatasetGenerator::Ptr(new FirstTryDataset());
    //guiGenerator = DatasetGenerator::Ptr(new StageOneDataset(true));
    guiGenerator = DatasetGenerator::Ptr(new BasicStrategyDataset(true, 5, 4));
    threadProgr.clear();
    threadCnt = 4;

    for (int i = 0; i < threadCnt; i++) {
        threadProgr.append(0);
    }

    setupUi(m_view);
    setCentralWidget(m_view);
    setupGUI();

    // next button
    connect(nextBtn, SIGNAL(clicked()), this, SLOT(nextBtnClicked()));

    QTimer::singleShot(0, this, &MLDataGenerator::initObject);
}

void MLDataGenerator::initObject() {
    generateGUIexample();

    //generateFirstTryDataset();
    //generateStageOneDataset();
    generateBasicStrategyDataset();
}

void MLDataGenerator::generateFirstTryDataset() {
    int width = 5;
    int height = 4;
    FirstTryDataset::Ptr generator = FirstTryDataset::Ptr(new FirstTryDataset(width, height, QStringLiteral("/home/ofenrohr/arbeit/master/data")));
    setupThread(generator);
}

void MLDataGenerator::generateStageOneDataset() {
    StageOneDataset::Ptr gen = StageOneDataset::Ptr(new StageOneDataset(false));
    gen->startConverter(5,4, examplesCnt);
    setupThread(gen);
}

void MLDataGenerator::generateBasicStrategyDataset() {
    BasicStrategyDataset::Ptr gen = BasicStrategyDataset::Ptr(new BasicStrategyDataset(false, 5,4));
    gen->startConverter(examplesCnt);
    setupThread(gen);
}

void MLDataGenerator::setupThread(DatasetGenerator::Ptr generator) {
    qDebug() << examplesCnt;
    if (examplesCnt > 0) {
        nextBtn->setEnabled(false);
        progressBar->setValue(0);

        for (int i = 0; i < threadCnt; i++) {
            // https://mayaposch.wordpress.com/2011/11/01/how-to-really-truly-use-qthreads-the-full-explanation/
            QThread *thread = new QThread;
            MLDataGeneratorWorkerThread *worker = new MLDataGeneratorWorkerThread(examplesCnt / threadCnt, generator, i);
            worker->moveToThread(thread);
            //connect(worker, SIGNAL(error(QString)), this, SLOT(errorString(QString)));
            connect(thread, SIGNAL(started()), worker, SLOT(process()));
            connect(worker, SIGNAL(finished()), thread, SLOT(quit()));
            connect(worker, SIGNAL(finished()), worker, SLOT(deleteLater()));
            connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
            connect(worker, SIGNAL(progress(int, int)), this, SLOT(recvProgress(int, int)));
            connect(thread, SIGNAL(finished()), this, SLOT(dataGeneratorFinished()));
            //connect(generator.data(), SIGNAL(sendGUIsample(aiBoard::Ptr, QImage, QImage)), this, SLOT(setGUIgame(aiBoard::Ptr, QImage, QImage)));
            thread->start();
        }
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
    qDebug() << threadProgr;
    progressBar->setValue(sum / threadCnt);
}

void MLDataGenerator::dataGeneratorFinished() {
    nextBtn->setEnabled(true);
    progressBar->setVisible(false);
    qDebug() << "done generating data";
}

void MLDataGenerator::generateGUIexample() {// setup
    int width = 5;
    int height = 4;

    // draw stuff
    if (gbs != NULL) {
        delete gbs;
    }
    gbs = new GameBoardScene(width, height, this);

    Dataset guiDataset = guiGenerator->generateDataset();
    aiBoard::Ptr board;
    if (guiDataset.isValid()) {
        qDebug() << "generator dataset";
        board = guiDataset.getBoard();
        inputImage = guiDataset.getInputImg();
        outputImage = guiDataset.getOutputImg();
    } else {
        qDebug() << "NOT generator dataset";
        // generate data
        board = generateRandomBoard(width, height, 5);

        // make some more moves
        KSquaresAi::Ptr ai = KSquaresAi::Ptr(new aiEasyMediumHard(0, width, height, 2));
        makeAiMoves(board, ai, 20);


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

aiBoard::Ptr MLDataGenerator::generateRandomBoard(int width, int height, int safeMoves) {
    // generate the board with auto fill
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

QImage MLDataGenerator::generateOutputImage(aiBoard::Ptr board, QList<int> lines) {
    int imgWidth = board->width*2+3; // 1px border
    int imgHeight = board->height*2+3;

    QImage img(imgWidth, imgHeight, QImage::Format_ARGB32);

    drawBackgroundAndDots(img, false);
    foreach (int line, lines) {
        drawLineAt(img, line, board->width, board->height);
    }

    return img;
}

void MLDataGenerator::saveImage(QString dataSetName, QString instanceName, QString dest, QImage &img) {
    QString filename = dest + QStringLiteral("/") + dataSetName + instanceName + QStringLiteral(".png");
    img.save(filename);
}

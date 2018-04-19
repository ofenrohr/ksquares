//
// Created by ofenrohr on 19.04.18.
//

#include <alphaDots/MLDataGenerator.h>
#include <alphaDots/MLImageGenerator.h>
#include <alphaDots/cnpy.h>
#include <QtCore/QDateTime>
#include <QtWidgets/QMessageBox>
#include <aiEasyMediumHard.h>
#include "StageThreeDataset.h"


using namespace AlphaDots;

StageThreeDataset::StageThreeDataset(bool gui, int w, int h, int thread, int threads) {
    isGUI = gui;
    width = w;
    height = h;

    sampleIdx = 0;
    threadIdx = thread;
    threadCnt = threads;

    qDebug() << "StageTwoDataset: ";
    qDebug() << " |-> thread id: " << threadIdx;
    qDebug() << " |-> threads: " << threadCnt;
}

StageThreeDataset::~StageThreeDataset() {
    cleanup();
}

void StageThreeDataset::cleanup() {
    if (!isGUI) {
        //converter->stopExternalProcess();
    }
}

void StageThreeDataset::startConverter(int samples, QString destinationDirectory) {
    sampleCnt = samples;
    destDir = destinationDirectory;

    int widthImg = MLImageGenerator::boxesToImgSize(width);
    int heightImg = MLImageGenerator::boxesToImgSize(height);
    dataSize = {
            sampleCnt,
            heightImg,
            widthImg
    };
    input = new std::vector<uint8_t>(sampleCnt*heightImg*widthImg);
    output = new std::vector<uint8_t>(sampleCnt*heightImg*widthImg);
}

void StageThreeDataset::stopConverter() {
    QString timeStr = QDateTime::currentDateTime().toString(QStringLiteral("-hh:mm-dd_MM_yyyy"));
    std::string filename = "/StageTwo-" + std::to_string(sampleCnt) + "-" + std::to_string(width) + "x" + std::to_string(height) + timeStr.toStdString() + ".npz";
    if (!cnpy::npz_save(destDir.toStdString()+filename, "x_train", &(*input)[0], dataSize, "w")) {
        QMessageBox::critical(nullptr, i18n("Error"), i18n("failed to save input data"));
    }
    if (!cnpy::npz_save(destDir.toStdString()+filename, "y_train", &(*output)[0], dataSize, "a")) {
        QMessageBox::critical(nullptr, i18n("Error"), i18n("failed to save output data"));
    }
}

Dataset StageThreeDataset::generateDataset() {
    // generate initial board
    aiBoard::Ptr board = MLDataGenerator::generateRandomBoard(width, height, 15);

    // make some more moves
    KSquaresAi::Ptr ai = KSquaresAi::Ptr(new aiEasyMediumHard(0, width, height, 2));
    QList<int> freeLines = ai->getFreeLines(board->lines, board->linesSize);

    // add hard ai moves (smart moves)
    int movesLeft = qrand() % freeLines.count() + 1;
    MLDataGenerator::makeAiMoves(board, ai, movesLeft);

    // do sth random (stupid moves)
    if (qrand() % 10 < 2) {
        freeLines = ai->getFreeLines(board->lines, board->linesSize);
        board->doMove(freeLines[qrand() % freeLines.count()]);
    }

    // generate images
    QImage inputImage = MLImageGenerator::generateInputImage(board);
    QImage outputImage = MLImageGenerator::generateOutputImage(board, ai);

    if (isGUI) {
        return Dataset(inputImage, outputImage, board);
    }

    // add to data
    int widthImg = MLImageGenerator::boxesToImgSize(width);
    int heightImg = MLImageGenerator::boxesToImgSize(height);
    if (input != nullptr && output != nullptr) {
        int sampleStart = (sampleIdx * threadCnt + threadIdx) * heightImg * widthImg;
        //qDebug() << "sampleStart: " << sampleStart;
        for (int y = 0; y < heightImg; y++) {
            for (int x = 0; x < widthImg; x++) {
                input->at(sampleStart + y * widthImg + x) = (uint8_t) inputImage.pixelColor(x,y).red();
                (*output)[sampleStart + y * widthImg + x] = (uint8_t) outputImage.pixelColor(x,y).red();
            }
        }
    };

    //qDebug() << ".";
    sampleIdx++;

    return Dataset(inputImage, outputImage, board);
}
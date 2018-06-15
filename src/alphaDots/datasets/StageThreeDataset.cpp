//
// Created by ofenrohr on 19.04.18.
//

#include <alphaDots/MLDataGenerator.h>
#include <alphaDots/MLImageGenerator.h>
#include <alphaDots/cnpy.h>
#include <QtCore/QDateTime>
#include <QtWidgets/QMessageBox>
#include <aiEasyMediumHard.h>
#include <cmath>
#include "StageThreeDataset.h"


using namespace AlphaDots;

StageThreeDataset::StageThreeDataset(bool gui, int w, int h, int thread, int threads) {
    isGUI = gui;
    width = w;
    height = h;

    sampleIdx = 0;
    threadIdx = thread;
    threadCnt = threads;

    qDebug() << "StageThreeDataset: ";
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
    valueDataSize = {
            sampleCnt,
            1
    };
    input = new std::vector<uint8_t>(sampleCnt*heightImg*widthImg);
    policy = new std::vector<uint8_t>(sampleCnt*heightImg*widthImg);
    value = new std::vector<double>(sampleCnt);
}

bool StageThreeDataset::stopConverter() {
    QString timeStr = QDateTime::currentDateTime().toString(QStringLiteral("-hh:mm-dd_MM_yyyy"));
    std::string filename = "/StageThree-" + std::to_string(sampleCnt) + "-" + std::to_string(width) + "x" + std::to_string(height) + timeStr.toStdString() + ".npz";
    //std::string filename = "/StageThree.npz";
    bool success = true;
    if (!cnpy::npz_save(destDir.toStdString()+filename, "input", &(*input)[0], dataSize, "w")) {
        QMessageBox::critical(nullptr, QObject::tr("Error"), QObject::tr("failed to save input data"));
        success = false;
    }
    if (!cnpy::npz_save(destDir.toStdString()+filename, "policy", &(*policy)[0], dataSize, "a")) {
        QMessageBox::critical(nullptr, QObject::tr("Error"), QObject::tr("failed to save policy data"));
        success = false;
    }
    if (!cnpy::npz_save(destDir.toStdString()+filename, "value", &(*value)[0], valueDataSize, "a")) {
        QMessageBox::critical(nullptr, QObject::tr("Error"), QObject::tr("failed to save value data"));
        success = false;
    }
    delete input;
    delete policy;
    delete value;
    return success;
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
    freeLines = ai->getFreeLines(board->lines, board->linesSize);

    // do sth random (stupid moves)
    if (qrand() % 10 < 2) {
        int rndLine = qrand() % freeLines.count();
        board->doMove(freeLines[rndLine]);
        freeLines.removeAt(rndLine);
    }

    int currentPlayer = 1 - board->playerId;

    // generate images
    QImage inputImage = MLImageGenerator::generateInputImage(board);
    QImage outputImage = MLImageGenerator::generateOutputImage(board, ai);

    // calculate value
    double val = 0;
    int freeLinesCnt = freeLines.size();
    QList<int> extraLines;
    for (int i = 0; i < freeLinesCnt; i++) {
        int line = ai->chooseLine(board->linesAsList(), board->squareOwners, QList<Board::Move_t>());
        board->doMove(line);
        extraLines.prepend(line);
    }
    for (const auto &owner: board->squareOwners) {
        val += owner == currentPlayer ? 1 : -1;
    }
    val /= 0.8 * board->squareOwners.size();

    // return gui dataset
    if (isGUI) {
        for (const auto &line : extraLines) {
            board->undoMove(line);
        }
        return Dataset(inputImage, outputImage, val, board);
    }

    // add to data
    int widthImg = MLImageGenerator::boxesToImgSize(width);
    int heightImg = MLImageGenerator::boxesToImgSize(height);
    if (input != nullptr && policy != nullptr && value != nullptr) {
        int sampleStart = (sampleIdx * threadCnt + threadIdx) * heightImg * widthImg;
        int valueSampleStart = sampleIdx * threadCnt + threadIdx;
        //qDebug() << "sampleStart: " << sampleStart;
        for (int y = 0; y < heightImg; y++) {
            for (int x = 0; x < widthImg; x++) {
                input->at(sampleStart + y * widthImg + x) = (uint8_t) inputImage.pixelColor(x,y).red();
                (*policy)[sampleStart + y * widthImg + x] = (uint8_t) outputImage.pixelColor(x,y).red();
                (*value)[valueSampleStart] = val;
            }
        }
    } else {
        qDebug() << "ERROR: input, policy or value is null";
    }

    //qDebug() << ".";
    sampleIdx++;

    return Dataset(inputImage, outputImage, val, board);
}
//
// Created by ofenrohr on 29.10.17.
//

#include <QtCore/QString>
#include <alphaDots/MLDataGenerator.h>
#include <alphaDots/MLImageGenerator.h>
#include <QtCore/QUuid>
#include <aiEasyMediumHard.h>
#include "FirstTryDataset.h"

using namespace AlphaDots;

FirstTryDataset::FirstTryDataset(int w, int h, QString outputDir) {
    width = w;
    height = h;
    output_dir = outputDir;
}

Dataset FirstTryDataset::generateDataset() {
    // generate data
    aiBoard::Ptr board = MLDataGenerator::generateRandomBoard(width, height, 5);

    // make some more moves
    KSquaresAi::Ptr ai = KSquaresAi::Ptr(new aiEasyMediumHard(0, width, height, 2));
    MLDataGenerator::makeAiMoves(board, ai, 20);

    // generate images
    QImage *inputImageP = MLImageGenerator::generateInputImage(board);
    QImage inputImage = inputImageP->copy();
    delete inputImageP;
    QImage *outputImageP = MLImageGenerator::generateOutputImage(board, ai);
    QImage outputImage = outputImageP->copy();
    delete outputImageP;

    // save boards
    QUuid id = QUuid::createUuid();
    QString boardSize = QString::number(width) + QStringLiteral("x") + QString::number(height);
    MLDataGenerator::saveImage(QStringLiteral("firstTry_") + boardSize, id.toString() + QStringLiteral("input"),
                               output_dir, inputImage);
    MLDataGenerator::saveImage(QStringLiteral("firstTry_") + boardSize, id.toString() + QStringLiteral("output_hardai"),
                               output_dir, outputImage);

    Dataset ret(inputImage, outputImage, board);
    return ret;
}

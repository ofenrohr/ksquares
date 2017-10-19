//
// Created by ofenrohr on 10/19/17.
//

#include "MLDataGeneratorWorkerThread.h"
#include "aiBoard.h"
#include "MLDataGenerator.h"
#include "aiEasyMediumHard.h"

#include <QDebug>
#include <QtCore/QUuid>

MLDataGeneratorWorkerThread::MLDataGeneratorWorkerThread(long examples) {
    sampleCnt = examples;
}

MLDataGeneratorWorkerThread::~MLDataGeneratorWorkerThread() {
}

void MLDataGeneratorWorkerThread::process() {
    qDebug() << "thread";

    int width = 5;
    int height = 4;

    int oldProgress = 0;

    for (int i = 0; i < sampleCnt; i++) {
        // generate data
        aiBoard::Ptr board = MLDataGenerator::generateRandomBoard(width, height, 5);

        // make some more moves
        KSquaresAi::Ptr ai = KSquaresAi::Ptr(new aiEasyMediumHard(0, width, height, 2));
        MLDataGenerator::makeAiMoves(board, ai, 20);

        // generate images
        QImage inputImage = MLDataGenerator::generateInputImage(board);
        QImage outputImage = MLDataGenerator::generateOutputImage(board, ai);

        // save boards
        QUuid id = QUuid::createUuid();
        MLDataGenerator::saveImage(QStringLiteral("firstTry_5x4"), id.toString() + QStringLiteral("input"),
                                   QStringLiteral("/home/ofenrohr/arbeit/master/data"), inputImage);
        MLDataGenerator::saveImage(QStringLiteral("firstTry_5x4"), id.toString() + QStringLiteral("output_hardai"),
                                   QStringLiteral("/home/ofenrohr/arbeit/master/data"), outputImage);

        int progr = (int) ((100.0 / sampleCnt) * (i+1.0));
        if (progr != oldProgress) {
            emit progress(progr);
            oldProgress = progr;
        }
    }
}

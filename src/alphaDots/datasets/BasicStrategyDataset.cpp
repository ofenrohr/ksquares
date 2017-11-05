//
// Created by ofenrohr on 05.11.17.
//

#include <alphaDots/MLDataGenerator.h>
#include <zmq.hpp>
#include <aiEasyMediumHard.h>
#include <alphaDots/PBConnector.h>
#include "BasicStrategyDataset.h"

using namespace AlphaDots;

BasicStrategyDataset::BasicStrategyDataset(bool gui, int width, int height) {
    isGUI = gui;
    this->width = width;
    this->height = height;
}

BasicStrategyDataset::~BasicStrategyDataset() {

}

void BasicStrategyDataset::startConverter(int samples) {
    int widthImg = MLDataGenerator::boxesToImgSize(width);
    int heightImg = MLDataGenerator::boxesToImgSize(height);

    QStringList args;
    args << QStringLiteral("/home/ofenrohr/arbeit/master/code/alphaDots/datasetConverter/convert.py")
         << QStringLiteral("--zmq")
         << QString::number(samples)
         << QStringLiteral("--output-file")
         << QStringLiteral("/run/media/ofenrohr/Data/AlphaDots/data/basicStrategy") + QString::number(width) + QStringLiteral("x") + QString::number(height) + QStringLiteral(".npz")
         << QStringLiteral("--x-size")
         << QString::number(widthImg)
         << QStringLiteral("--y-size")
         << QString::number(heightImg);
    if (!isGUI) {
        converter = new ExternalProcess(QStringLiteral("/usr/bin/python2.7"), args);
        converter->startExternalProcess();
    }
}

void BasicStrategyDataset::cleanup() {
    DatasetGenerator::cleanup();
}

Dataset BasicStrategyDataset::generateDataset() {

    zmq::context_t context(1);
	zmq::socket_t socket(context, ZMQ_REQ);
	socket.connect("tcp://127.0.0.1:12355");

    aiBoard::Ptr board = MLDataGenerator::generateRandomBoard(width, height, rand() % (width*height));//aiBoard::Ptr(new aiBoard(width, height));
    KSquaresAi::Ptr ai = KSquaresAi::Ptr(new aiEasyMediumHard(0, width, height, 2));

    bool capture = false; // if true, capturing boxes is possible
    QList<int> freeLines = ai->getFreeLines(board->lines, board->linesSize);
    if (rand() % 2 == 0) {
        // add hard ai moves (smart moves)
        int movesLeft = rand() % freeLines.count() + 1;
        MLDataGenerator::makeAiMoves(board, ai, movesLeft);
    } else {
        // do sth random (stupid moves)
        board->doMove(freeLines[rand() % freeLines.count()]);
        KSquares::BoardAnalysis analysis = ai->analyseBoard(board);
        capture =
            analysis.capturableLongChains.count() > 0 ||
            analysis.capturableLoopChains.count() > 0 ||
            analysis.capturableShortChains.count() > 0;
    }

    // generate images
    QImage inputImage = MLDataGenerator::generateInputImage(board);

    QList<int> safeLines = ai->safeMoves(board->linesSize, board->lines);
    QImage outputImage;
    if (safeLines.count() > 0 && !capture) {
        outputImage = MLDataGenerator::generateOutputImage(board, safeLines);
    } else {
        outputImage = MLDataGenerator::generateOutputImage(board, ai);
    }

    if (isGUI) {
        return Dataset(inputImage, outputImage, board);
    }

    // send it to the python dataset converter
    TrainingExample trainingExample = PBConnector::toProtobuf(inputImage, outputImage);
    if (!PBConnector::sendString(socket, trainingExample.SerializeAsString())) {
        qDebug() << "sending data failed!";
        return Dataset();
    }
    std::string rpl = PBConnector::recvString(socket);
    if (rpl != "ok") {
        qDebug() << "process sent invalid reply: " << rpl.c_str();
        return Dataset();
    }

    //qDebug() << ".";
    sampleIdx++;

    return Dataset(inputImage, outputImage, board);
}


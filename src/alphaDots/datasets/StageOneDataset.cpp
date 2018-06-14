//
// Created by ofenrohr on 30.10.17.
//

#include <alphaDots/ExternalProcess.h>
#include <alphaDots/MLDataGenerator.h>
#include <alphaDots/ProtobufConnector.h>
#include <alphaDots/MLImageGenerator.h>
#include <aiEasyMediumHard.h>
#include <settings.h>
#include "StageOneDataset.h"

using namespace AlphaDots;

StageOneDataset::StageOneDataset(bool gui, int w, int h) {
    isGUI = gui;
    width = w;
    height = h;

    sampleIdx = 0;
}

StageOneDataset::~StageOneDataset() {
    cleanup();
}

void StageOneDataset::cleanup() {
    if (!isGUI) {
        //converter->stopExternalProcess();
    }
}

void StageOneDataset::startConverter(int samples, QString destinationDirectory) {
    int widthImg = MLImageGenerator::boxesToImgSize(width);
    int heightImg = MLImageGenerator::boxesToImgSize(height);

    QStringList args;
    args << Settings::alphaDotsDir() + QStringLiteral("/datasetConverter/convert.py")
         << QStringLiteral("--zmq")
         << QString::number(samples)
         << QStringLiteral("--output-file")
         << destinationDirectory + QStringLiteral("/StageOne-") + QString::number(width) + QStringLiteral("x") + QString::number(height) + QStringLiteral(".npz")
         << QStringLiteral("--x-size")
         << QString::number(widthImg)
         << QStringLiteral("--y-size")
         << QString::number(heightImg);
    if (!isGUI) {
        converter = new ExternalProcess(Settings::pythonExecutable(), args);
        converter->startExternalProcess();
    }
}

Dataset StageOneDataset::generateDataset() {

    zmq::context_t context(1);
	zmq::socket_t socket(context, ZMQ_REQ);
	socket.connect("tcp://127.0.0.1:12355");

    // generate data
    aiBoard::Ptr board = MLDataGenerator::generateRandomBoard(width, height, 15);

    // make some more moves
    KSquaresAi::Ptr ai = KSquaresAi::Ptr(new aiEasyMediumHard(0, width, height, 2));
    QList<int> freeLines = ai->getFreeLines(board->lines, board->linesSize);

    // add hard ai moves (smart moves)
    int movesLeft = rand() % freeLines.count() + 1;
    MLDataGenerator::makeAiMoves(board, ai, movesLeft);

    // do sth random (stupid moves)
    if (rand() % 10 < 2) {
        freeLines = ai->getFreeLines(board->lines, board->linesSize);
        board->doMove(freeLines[rand() % freeLines.count()]);
    }

    // generate images
    QImage inputImage = MLImageGenerator::generateInputImage(board);
    QImage outputImage = MLImageGenerator::generateOutputImage(board, ai);

    if (isGUI) {
        return Dataset(inputImage, outputImage, board);
    }

    // send it to the python dataset converter
    TrainingExample trainingExample = ProtobufConnector::trainingExampleToProtobuf(inputImage, outputImage);
    if (!ProtobufConnector::sendString(socket, trainingExample.SerializeAsString())) {
        qDebug() << "sending data failed!";
        return Dataset();
    }
    bool ok = false;
    std::string rpl = ProtobufConnector::recvString(socket, &ok);
    if (ok || rpl != "ok") {
        qDebug() << "process sent invalid reply: " << rpl.c_str();
        return Dataset();
    }

    //qDebug() << ".";
    sampleIdx++;

    return Dataset(inputImage, outputImage, board);
}

//
// Created by ofenrohr on 01.12.17.
//

#include <alphaDots/MLDataGenerator.h>
#include <alphaDots/ProtobufConnector.h>
#include <alphaDots/MLImageGenerator.h>
#include <aiEasyMediumHard.h>
#include <settings.h>
#include "SequenceDataset.h"

using namespace AlphaDots;


SequenceDataset::SequenceDataset(bool gui, int width, int height) {
    isGUI = gui;
    this->width = width;
    this->height = height;
    connectionReady = false;
}

SequenceDataset::~SequenceDataset() {

}

Dataset SequenceDataset::generateDataset() {
    getSocket();
    QList<QImage> seqData;
    KSquaresAi::Ptr ai = KSquaresAi::Ptr(new aiEasyMediumHard(0, width, height, 2));
    Board board(2, width, height);
    aiBoard::Ptr aiboard(new aiBoard(&board));
    QList<int> linesList;
    bool boardFilled = false;
    while (!boardFilled) {
        int line = ai->chooseLine(board.lines(), board.squares(), board.getLineHistory());
        linesList.append(line);
        bool nextPlayer;
        QList<int> completedSquares;
        board.addLine(line, &nextPlayer, &boardFilled, &completedSquares);
        aiboard->doMove(line);
        seqData.append(MLImageGenerator::generateInputImage(aiboard));
    }

    if (!isGUI) {
        // send it to the python dataset converter
        GameSequence gameSequence = ProtobufConnector::gameSequenceToProtobuf(seqData);
        if (!ProtobufConnector::sendString(*socket, gameSequence.SerializeAsString())) {
            qDebug() << "sending data failed!";
            return Dataset();
        }
        std::string rpl = ProtobufConnector::recvString(*socket);
        if (rpl != "ok") {
            qDebug() << "process sent invalid reply: " << rpl.c_str();
            return Dataset();
        }

        //qDebug() << ".";
        sampleIdx++;
    }

    return Dataset(seqData, aiboard);
    //return ret;
}

void SequenceDataset::startConverter(int samples, QString destinationDirectory) {
    int widthImg = MLImageGenerator::boxesToImgSize(width);
    int heightImg = MLImageGenerator::boxesToImgSize(height);


    QStringList args;
    args << Settings::alphaDotsDir() + QStringLiteral("/datasetConverter/convert.py")
         //<< QStringLiteral("--debug")
         << QStringLiteral("--zmq")
         << QString::number(samples)
         << QStringLiteral("--seq")
         << QStringLiteral("--output-file")
         << destinationDirectory + QStringLiteral("/LSTM-") + QString::number(width) + QStringLiteral("x") + QString::number(height) + QStringLiteral(".npz")
         << QStringLiteral("--x-size")
         << QString::number(widthImg)
         << QStringLiteral("--y-size")
         << QString::number(heightImg);
    if (!isGUI) {
        converter = new ExternalProcess(QStringLiteral("/usr/bin/python2.7"), args);
        converter->startExternalProcess();
    }
}

void SequenceDataset::cleanup() {
    DatasetGenerator::cleanup();
    delete socket;
    delete context;
    connectionReady = false;
}

zmq::socket_t* SequenceDataset::getSocket() {
    if (!connectionReady) {
        qDebug() << "setting up zmq connection";
        context = new zmq::context_t(1);
        socket = new zmq::socket_t(*context, ZMQ_REQ);
        socket->connect("tcp://127.0.0.1:12355");
        connectionReady = true;
        qDebug() << "done setting up zmq connection";
    }

    return socket;
}

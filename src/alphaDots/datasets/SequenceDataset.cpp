//
// Created by ofenrohr on 01.12.17.
//

#include <alphaDots/MLDataGenerator.h>
#include <aiEasyMediumHard.h>
#include <alphaDots/PBConnector.h>
#include "SequenceDataset.h"

SequenceDataset::SequenceDataset(bool gui, int width, int height) {
    isGUI = gui;
    this->width = width;
    this->height = height;
    connectionReady = false;
}

SequenceDataset::~SequenceDataset() {

}

Dataset SequenceDataset::generateDataset() {
    qDebug() << "generateDataset";
    getSocket();
    qDebug() << "socket";
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
        seqData.append(MLDataGenerator::generateOutputImage(aiboard, linesList));
    }

    if (!isGUI) {
        qDebug() << "sending...";
        // send it to the python dataset converter
        GameSequence gameSequence = PBConnector::toProtobuf(seqData);
        if (!PBConnector::sendString(*socket, gameSequence.SerializeAsString())) {
            qDebug() << "sending data failed!";
            return Dataset();
        }
        std::string rpl = PBConnector::recvString(*socket);
        if (rpl != "ok") {
            qDebug() << "process sent invalid reply: " << rpl.c_str();
            return Dataset();
        }
        qDebug() << "done";

        //qDebug() << ".";
        sampleIdx++;
    }

    return Dataset(seqData, aiboard);
    //return ret;
}

void SequenceDataset::startConverter(int samples) {
    int widthImg = MLDataGenerator::boxesToImgSize(width);
    int heightImg = MLDataGenerator::boxesToImgSize(height);

    QStringList args;
    args << QStringLiteral("/home/ofenrohr/arbeit/master/code/alphaDots/datasetConverter/convert.py")
         << QStringLiteral("--debug")
         << QStringLiteral("--zmq")
         << QString::number(samples)
         << QStringLiteral("--seq")
         << QStringLiteral("--output-file")
         << QStringLiteral("/run/media/ofenrohr/Data/AlphaDots/data/sequence") + QString::number(width) + QStringLiteral("x") + QString::number(height) + QStringLiteral(".npz")
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

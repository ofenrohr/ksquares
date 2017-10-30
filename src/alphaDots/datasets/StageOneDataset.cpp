//
// Created by ofenrohr on 30.10.17.
//

#include <alphaDots/DatasetConverter.h>
#include <alphaDots/MLDataGenerator.h>
#include <aiEasyMediumHard.h>
#include <alphaDots/PBConnector.h>
#include "StageOneDataset.h"

StageOneDataset::StageOneDataset(int w, int h) {
    width = w;
    height = h;

}

StageOneDataset::~StageOneDataset() {
}

void StageOneDataset::generateDataset() {

    zmq::context_t context(1);
	zmq::socket_t socket(context, ZMQ_REQ);
	socket.connect("tcp://127.0.0.1:12355");

    // generate data
    aiBoard::Ptr board = MLDataGenerator::generateRandomBoard(width, height, 5);

    // make some more moves
    KSquaresAi::Ptr ai = KSquaresAi::Ptr(new aiEasyMediumHard(0, width, height, 2));
    MLDataGenerator::makeAiMoves(board, ai, 20);

    // generate images
    QImage inputImage = MLDataGenerator::generateInputImage(board);
    QImage outputImage = MLDataGenerator::generateOutputImage(board, ai);

    alphaDots::DotsAndBoxesImage pbImgInput = PBConnector::toProtobuf(inputImage);
    alphaDots::DotsAndBoxesImage pbImgOutput = PBConnector::toProtobuf(outputImage);

    // send input image
    if (!PBConnector::sendString(socket, pbImgInput.SerializeAsString())) {
        qDebug() << "sending data failed!";
        return;
    }
    std::string rpl = PBConnector::recvString(socket);

    if (rpl.compare("ok") != 0) {
        qDebug() << "converter sent invalid reply: " << rpl.c_str();
        return;
    }

    // send output image
    PBConnector::sendString(socket, pbImgInput.SerializeAsString());

	zmq::message_t reply2;
	socket.recv(&reply2);
    std::string rpl2 = std::string(static_cast<char*>(reply2.data()), reply2.size());

    if (rpl2.compare("ok2") != 0) {
        qDebug() << "converter sent invalid reply: " << rpl.c_str();
        return;
    }

}

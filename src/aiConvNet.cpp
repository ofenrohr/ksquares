//
// Created by ofenrohr on 26.10.17.
//

#include <sstream>
#include "aiConvNet.h"
#include "alphaDots/PBConnector.h"
#include "MLDataGenerator.h"


aiConvNet::aiConvNet(int newPlayerId, int newMaxPlayerId, int newWidth, int newHeight, int newLevel, int thinkTime)
		: KSquaresAi(newWidth, newHeight),
		  playerId(newPlayerId),
		  maxPlayerId(newMaxPlayerId),
		  level(newLevel)
{
	width = newWidth;
	height = newHeight;
	linesSize = toLinesSize(width, height);

}

aiConvNet::~aiConvNet() {

}

int aiConvNet::chooseLine(const QList<bool> &newLines, const QList<int> &newSquareOwners,
                          const QList<Board::Move> &lineHistory) {
	zmq::context_t context(1);
	zmq::socket_t socket(context, ZMQ_REQ);
	socket.connect("tcp://127.0.0.1:12354");

    bool *lines = new bool[linesSize];
    for (int i = 0; i < linesSize; ++i)
	{
		lines[i] = newLines[i];
	}

	aiBoard::Ptr board = aiBoard::Ptr(new aiBoard(lines, linesSize, width, height, newSquareOwners, playerId, maxPlayerId));
    alphaDots::DotsAndBoxesImage img = PBConnector::toProtobuf(MLDataGenerator::generateInputImage(board));
	sendString(socket, img.SerializeAsString());

	zmq::message_t reply;
	socket.recv(&reply);
    std::string rpl = std::string(static_cast<char*>(reply.data()), reply.size());

	//qDebug() << "Received: " << (char*)reply.data();
	QImage prediction = PBConnector::fromProtobuf(rpl);

    int bestVal = -1;
	QPoint linePoint;
	std::stringstream pred;
	for (int y = 0; y < prediction.height(); y++) {
		for (int x = 0; x < prediction.width(); x++) {
			int c = prediction.pixelColor(x,y).red();
			pred << c << " ";
			if (c < 10)
				pred << " ";
			if (c < 100)
				pred << " ";
			bool invalidPoint = true;
			if (x % 2 == 1 && y % 2 == 0) {
				invalidPoint = false;
			}
			if (x % 2 == 0 && y % 2 == 1) {
				invalidPoint = false;
			}
			if (x == 0 || y == 0 || x == prediction.width()-1 || y == prediction.height()-1) {
				invalidPoint = true;
			}
			if (c > bestVal && !invalidPoint) {
                bestVal = c;
				linePoint.setX(x);
				linePoint.setY(y);
			}
		}
        pred << "\n";
	}
	qDebug().noquote() << pred.str().c_str();

	qDebug() << "Highest value:" << bestVal << "at" << linePoint;

	int ret;
	if (linePoint.x() % 2 == 0) { // horizontal line
		ret = (linePoint.x() / 2 - 1) + (linePoint.y() / 2) * width + ((linePoint.y() - 1) / 2) * (width + 1);
	} else { // vertical line
		ret = (linePoint.x() / 2) + (linePoint.y() / 2) * width + (linePoint.y()/2 -1) * (width+1);
	}

	delete lines;
    return ret;
}

void aiConvNet::sendString(zmq::socket_t &socket, std::string msg) {
	ulong message_size = msg.size();
	zmq::message_t request(message_size);
	memcpy(request.data(), msg.c_str(), message_size);
	socket.send(request);
}
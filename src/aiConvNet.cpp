//
// Created by ofenrohr on 26.10.17.
//

#include <sstream>
#include "aiConvNet.h"
#include "alphaDots/PBConnector.h"
#include "alphaDots/MLDataGenerator.h"

using namespace AlphaDots;

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
    DotsAndBoxesImage img = PBConnector::toProtobuf(MLDataGenerator::generateInputImage(board));
	PBConnector::sendString(socket, img.SerializeAsString());

	zmq::message_t reply;
	socket.recv(&reply);
    std::string rpl = std::string(static_cast<char*>(reply.data()), reply.size());

	//qDebug() << "Received: " << (char*)reply.data();
	QImage prediction = PBConnector::fromProtobuf(rpl);

	QList<QPoint> bestPoints;
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
            if (lines[PBConnector::pointToLineIndex(QPoint(x,y), width)]) {
				invalidPoint = true;
			}
			if (c > bestVal && !invalidPoint) {
				bestPoints.clear();
                bestVal = c;
				//linePoint.setX(x);
				//linePoint.setY(y);
				linePoint = QPoint(x,y);
			}
            if (c == bestVal && !invalidPoint) {
				bestPoints.append(linePoint);
			}
		}
        pred << "\n";
	}
	qDebug().noquote() << pred.str().c_str();

	qDebug() << "Highest value:" << bestVal << "at" << bestPoints;

	if (bestPoints.count() > 1) {
		linePoint = bestPoints[rand() % bestPoints.count()];
	}
	qDebug() << "selected point: " << linePoint;


	int ret = PBConnector::pointToLineIndex(linePoint, width);

	delete lines;
    return ret;
}

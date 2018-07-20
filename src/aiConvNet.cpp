//
// Created by ofenrohr on 26.10.17.
//

#include <sstream>
#include <QtCore/QElapsedTimer>
#include <settings.h>
#include "aiConvNet.h"
#include "alphaDots/ProtobufConnector.h"
#include "alphaDots/MLDataGenerator.h"
#include "alphaDots/MLImageGenerator.h"
#include "alphaDots/ModelManager.h"
#include <PolicyValueData.pb.h>
#include <cmath>

using namespace AlphaDots;

aiConvNet::aiConvNet(int newPlayerId, int newMaxPlayerId, int newWidth, int newHeight, int newLevel, int thinkTime, ModelInfo model)
		: KSquaresAi(newWidth, newHeight),
		  playerId(newPlayerId),
		  maxPlayerId(newMaxPlayerId),
		  level(newLevel),
		  modelInfo(model),
		  context(zmq::context_t(1)),
		  socket(zmq::socket_t(context, ZMQ_REQ))
{
	width = newWidth;
	height = newHeight;
	linesSize = toLinesSize(width, height);
	turnTime = -5;
	isTainted = false;

	//qDebug() << "aiConvNet";

	port = ModelManager::getInstance().ensureProcessRunning(modelInfo.name(), width, height);
	if (port < 0) {
		qDebug() << "ensureProcessRunning failed!";
		isTainted = true;
	}

	//qDebug() << "zmq connect...";
	//context = zmq::context_t(1);
	//socket = zmq::socket_t(context, ZMQ_REQ);
	try {
		//qDebug().noquote().nospace() << "tcp://127.0.0.1:" << QString::number(port);
		socket.connect("tcp://127.0.0.1:" + std::to_string(port));
		//qDebug() << "zmq connect done";
		//socket.connect("icp:///tmp/alphaDots/model");
	} catch (zmq::error_t err) {
		qDebug() << "Connection failed! " << err.num() << ": " << err.what();
		//return -1;
	}

	// moved to ModelManager and ModelProcess
	/*
    qDebug() << "Starting: modelServer.py --model" << model.name();
	QStringList args;
	args << Settings::alphaDotsDir() + QStringLiteral("/modelServer/modelServer.py")
		 << QStringLiteral("--model")
		 << model.name()
    	 << QStringLiteral("--width")
		 << QString::number(width*2 + 3)
         << QStringLiteral("--height")
         << QString::number(height*2 + 3)
		 //<< QStringLiteral("--debug")
         ;
	modelServer = new ExternalProcess(QStringLiteral("/usr/bin/python2.7"), args);
	modelServer->addEnvironmentVariable(QStringLiteral("CUDA_VISIBLE_DEVICES"), QStringLiteral("-1"));
	if (!modelServer->startExternalProcess()) {
		qDebug() << "ERROR: can't start model server!";
	}
	 */
}

aiConvNet::~aiConvNet() {
	//modelServer->stopExternalProcess();
	//delete modelServer;
	ModelManager::getInstance().freeClaimOnProcess(modelInfo.name(), width, height);
}

int aiConvNet::chooseLine(const QList<bool> &newLines, const QList<int> &newSquareOwners,
                          const QList<Board::Move> &lineHistory) {
	//qDebug() << "aiConvNet chooseLine";
	QCoreApplication::processEvents();
	QElapsedTimer moveTimer;
	moveTimer.start();

	if (port < 0) {
        isTainted = true;
		return -1;
	}

	//qDebug() << "connection status: " << socket.connected();
	if (!socket.connected()) {
		qDebug() << "ERROR: no zmq connection!";
		isTainted = true;
		return -1;
	}

    bool *lines = new bool[linesSize];
    for (int i = 0; i < linesSize; ++i)
	{
		lines[i] = newLines[i];
	}

	if (modelInfo.type() == QStringLiteral("DirectInference") ||
        modelInfo.type() == QStringLiteral("DirectInferenceCategorical")) {
		aiBoard::Ptr board = aiBoard::Ptr(new aiBoard(lines, linesSize, width, height, newSquareOwners, playerId, maxPlayerId));
		DotsAndBoxesImage img = ProtobufConnector::dotsAndBoxesImageToProtobuf(MLImageGenerator::generateInputImage(board));
		ProtobufConnector::sendString(socket, img.SerializeAsString());
	}
    else if (modelInfo.type() == QStringLiteral("Sequence") ||
             modelInfo.type() == QStringLiteral("SequenceCategorical")) {
		//qDebug() << "sequence model";
		aiBoard::Ptr board = aiBoard::Ptr(new aiBoard(width, height));
		QList<QImage> imageSeq;
		imageSeq.append(MLImageGenerator::generateInputImage(board));
		for (Board::Move move : lineHistory) {
			board->doMove(move.line);
			imageSeq.append(MLImageGenerator::generateInputImage(board));
		}
		GameSequence seq = ProtobufConnector::gameSequenceToProtobuf(imageSeq);
        std::vector<std::string> errors;
        seq.FindInitializationErrors(&errors);
        foreach (std::string err, errors) {
            qDebug() << "GameSequence protobuf error: " << QString::fromStdString(err);
        }
		if (errors.size() > 0) {
			qDebug() << "imageSeq: " << imageSeq;
		}
		if (!ProtobufConnector::sendString(socket, seq.SerializeAsString())) {
            qDebug() << "ProtobufConnector::sendString failed!";
            isTainted = true;
			delete[] lines;
			return -1;
        }
	}
    else if (modelInfo.type() == QStringLiteral("PolicyValue")) {
		aiBoard::Ptr board = aiBoard::Ptr(new aiBoard(lines, linesSize, width, height, newSquareOwners, playerId, maxPlayerId));
		QImage inputImg = MLImageGenerator::generateInputImage(board);
		DotsAndBoxesImage img = ProtobufConnector::dotsAndBoxesImageToProtobuf(inputImg);
		ProtobufConnector::sendString(socket, img.SerializeAsString());
	} else {
		qDebug() << "ERROR: unknown model type!";
        qDebug() << modelInfo.type();
		exit(-1);
	}

	//qDebug() << "sending protobuf string done";
	bool ok;
	std::string rpl = ProtobufConnector::recvString(socket, &ok);
	if (!ok) {
		delete[] lines;
		return -1;
	}

	//qDebug() << "Received: " << (char*)reply.data();
    if (modelInfo.type() == QStringLiteral("PolicyValue")) {
		PolicyValueData policyValueData;
		policyValueData.ParseFromString(rpl);

		int ret = -1;
        int lineCnt = policyValueData.policy_size();
		double best = -INFINITY;
		for (int i = 0; i < lineCnt; i++) {
			if (lines[i]) {
				continue;
			}
            if (policyValueData.policy(i) > best) {
				best = policyValueData.policy(i);
				ret = i;
			}
        }

        delete[] lines;
		return ret;
	} else {
		QImage prediction = ProtobufConnector::fromProtobuf(rpl);

		QList<QPoint> bestPoints;
		int bestVal = -1;
		QPoint linePoint;
		std::stringstream pred;
		for (int y = 0; y < prediction.height(); y++) {
			for (int x = 0; x < prediction.width(); x++) {
				int c = prediction.pixelColor(x, y).red();
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
				if (x == 0 || y == 0 || x == prediction.width() - 1 || y == prediction.height() - 1) {
					invalidPoint = true;
				}
				if (lines[ProtobufConnector::pointToLineIndex(QPoint(x, y), width)]) {
					invalidPoint = true;
				}
				if (c > bestVal && !invalidPoint) {
					bestPoints.clear();
					bestVal = c;
					//linePoint.setX(x);
					//linePoint.setY(y);
					linePoint = QPoint(x, y);
				}
				if (c == bestVal && !invalidPoint) {
					bestPoints.append(QPoint(x, y));
				}
			}
			pred << "\n";
		}
		//qDebug().noquote() << pred.str().c_str();

		//qDebug() << "Highest value:" << bestVal << "at" << bestPoints;
		if (bestPoints.count() > 1) {
			linePoint = bestPoints[rand() % bestPoints.count()];
			//qDebug() << "selected point: " << linePoint;
		}


		int ret = ProtobufConnector::pointToLineIndex(linePoint, width);

		turnTime = moveTimer.elapsed();

		//qDebug() << "turn time = " << turnTime;

		delete[] lines;
		return ret;
	}
}

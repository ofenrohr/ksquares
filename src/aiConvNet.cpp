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
#include <ModelServer.pb.h>

using namespace AlphaDots;

aiConvNet::aiConvNet(int newPlayerId, int newMaxPlayerId, int newWidth, int newHeight, int newLevel, int thinkTime, ModelInfo model, bool gpu)
		: KSquaresAi(newWidth, newHeight),
		  playerId(newPlayerId),
		  maxPlayerId(newMaxPlayerId),
		  level(newLevel),
		  modelInfo(model),
		  useGPU(gpu),
		  context(zmq::context_t(1)),
		  socket(zmq::socket_t(context, ZMQ_REQ))
{
	width = newWidth;
	height = newHeight;
	linesSize = toLinesSize(width, height);
	turnTime = -5;
	isTainted = false;

	port = ModelManager::getInstance().ensureProcessRunning(modelInfo.name(), width, height, useGPU);
	if (port < 0) {
		qDebug() << "ensureProcessRunning failed!";
		isTainted = true;
	}

	try {
		socket.connect("tcp://127.0.0.1:" + std::to_string(port));
	} catch (zmq::error_t &err) {
		qDebug() << "Connection failed! " << err.num() << ": " << err.what();
		//return -1;
		isTainted = true;
	}
	rng = gsl_rng_alloc(gsl_rng_taus);
	gsl_rng_set(rng, std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count());
}

aiConvNet::~aiConvNet() {
	gsl_rng_free(rng);
	ModelManager::getInstance().freeClaimOnProcess(modelInfo.name(), width, height, useGPU);
}

int aiConvNet::chooseLine(const QList<bool> &newLines, const QList<int> &newSquareOwners,
                          const QList<Board::Move> &lineHistory) {
	//qDebug() << "aiConvNet chooseLine";
	QCoreApplication::processEvents();
	QElapsedTimer moveTimer;
	moveTimer.start();

	if (isTainted) {
		return -1;
	}

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

	// prepare common part of model server request
    ModelServerRequest srvRequest;
    srvRequest.set_action(ModelServerRequest::PREDICT);
    srvRequest.mutable_predictionrequest()->set_modelhandler(modelInfo.type().toStdString());
    srvRequest.mutable_predictionrequest()->set_modelkey(ModelManager::modelInfoToStr(modelInfo.name(), width, height, useGPU).toStdString());

	if (modelInfo.type() == QStringLiteral("DirectInference") ||
        modelInfo.type() == QStringLiteral("DirectInferenceCategorical")) {
		aiBoard::Ptr board = aiBoard::Ptr(new aiBoard(lines, linesSize, width, height, newSquareOwners, playerId, maxPlayerId));
		DotsAndBoxesImage img = ProtobufConnector::dotsAndBoxesImageToProtobuf(MLImageGenerator::generateInputImage(board));
		srvRequest.mutable_predictionrequest()->set_allocated_image(&img);
	}
    else if (modelInfo.type() == QStringLiteral("Sequence") ||
             modelInfo.type() == QStringLiteral("SequenceCategorical")) {
        // prepare image data
		aiBoard::Ptr board = aiBoard::Ptr(new aiBoard(width, height));
		QList<QImage> imageSeq;
		imageSeq.append(MLImageGenerator::generateInputImage(board));
		for (Board::Move move : lineHistory) {
			board->doMove(move.line);
			imageSeq.append(MLImageGenerator::generateInputImage(board));
		}
		// convert image data to protobuf message
		GameSequence seq = ProtobufConnector::gameSequenceToProtobuf(imageSeq);
		// protobuf error checking
        std::vector<std::string> errors;
        seq.FindInitializationErrors(&errors);
        foreach (std::string err, errors) {
            qDebug() << "GameSequence protobuf error: " << QString::fromStdString(err);
        }
		if (errors.size() > 0) {
			qDebug() << "imageSeq: " << imageSeq;
		}
		// add sequence protobuf message to model server request
		srvRequest.mutable_predictionrequest()->set_allocated_sequence(&seq);
	}
    else if (modelInfo.type() == QStringLiteral("PolicyValue")) {
		aiBoard::Ptr board = aiBoard::Ptr(new aiBoard(lines, linesSize, width, height, newSquareOwners, playerId, maxPlayerId));
		QImage inputImg = MLImageGenerator::generateInputImage(board);
		DotsAndBoxesImage *img = new DotsAndBoxesImage(ProtobufConnector::dotsAndBoxesImageToProtobuf(inputImg));
		srvRequest.mutable_predictionrequest()->set_allocated_image(img);
	} else {
		qDebug() << "ERROR: unknown model type!";
        qDebug() << modelInfo.type();
		exit(-1);
	}

	// send model server request
    if (!ProtobufConnector::sendString(socket, srvRequest.SerializeAsString())) {
        qDebug() << "ProtobufConnector::sendString failed!";
        isTainted = true;
        delete[] lines;
        return -1;
    }

    // receive response...
	//qDebug() << "sending protobuf string done";
	bool ok;
	std::string rpl = ProtobufConnector::recvString(socket, &ok);
	if (!ok) {
	    qDebug() << "Failed to receive reply via zeromq!";
		isTainted = true;
		delete[] lines;
		return -1;
	}

	ModelServerResponse srvResponse;
	srvResponse.ParseFromString(rpl);

	if (srvResponse.status() != ModelServerResponse::RESP_OK) {
		qDebug() << "ModelServer failed: " << QString::fromStdString(srvResponse.errormessage());
		isTainted = true;
		delete[] lines;
		return -1;
	}

	//qDebug() << "Received: " << (char*)reply.data();
    if (modelInfo.type() == QStringLiteral("PolicyValue")) {
		const PolicyValueData policyValueData = srvResponse.predictionresponse().pvdata();

		QList<int> bestLines;
        int lineCnt = policyValueData.policy_size();
		double best = -INFINITY;
		for (int i = 0; i < lineCnt; i++) {
			if (lines[i]) {
				continue;
			}
			if (policyValueData.policy(i) == best) {
				bestLines.append(i);
			}
            if (policyValueData.policy(i) > best) {
				best = policyValueData.policy(i);
				bestLines.clear();
				bestLines.append(i);
			}
        }
		int ret = bestLines[gsl_rng_uniform_int(rng, bestLines.size())];

        delete[] lines;
		return ret;
	} else {
		const QImage prediction = ProtobufConnector::protobufDotsAndBoxesImageToQImage(
				srvResponse.predictionresponse().image());

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
			linePoint = bestPoints[gsl_rng_uniform_int(rng, bestPoints.count())];
			//qDebug() << "selected point: " << linePoint;
		}


		int ret = ProtobufConnector::pointToLineIndex(linePoint, width);

		turnTime = moveTimer.elapsed();

		//qDebug() << "turn time = " << turnTime;

		delete[] lines;
		return ret;
	}
}

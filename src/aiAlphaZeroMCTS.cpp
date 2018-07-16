//
// Created by ofenrohr on 14.04.18.
//

#include "aiAlphaZeroMCTS.h"
#include "aiConvNet.h"

#include <cmath>
#include <algorithm>
#include <chrono>
#include <QFile>
#include <alphaDots/ProtobufConnector.h>
#include <PolicyValueData.pb.h>
#include <alphaDots/ModelManager.h>
#include <alphaDots/MLImageGenerator.h>

using namespace AlphaDots;

bool aiAlphaZeroMCTS::debug(false);
double aiAlphaZeroMCTS::C_puct = 4.0; // overwritten in main.cpp
double aiAlphaZeroMCTS::dirichlet_alpha = 0.03; // overwritten in main.cpp
int aiAlphaZeroMCTS::mcts_iterations = 1500; // overwritten in main.cpp

aiAlphaZeroMCTS::aiAlphaZeroMCTS(int newPlayerId, int newMaxPlayerId, int newWidth, int newHeight,
                                 int thinkTime, ModelInfo model) :
        KSquaresAi(newWidth, newHeight), playerId(newPlayerId), maxPlayerId(newMaxPlayerId),
        mctsTimeout(thinkTime), context(zmq::context_t(1)), socket(zmq::socket_t(context, ZMQ_REQ))
{
    // setup basics
    width = newWidth;
    height = newHeight;
    linesSize = toLinesSize(width, height);
    lines = new bool[linesSize];
    for (int i = 0; i < linesSize; i++) {
        lines[i] = false;
    }
    mctsTimer = QElapsedTimer();
    isTainted = false;
    mctsRootNode = AlphaZeroMCTSNode::Ptr(nullptr);

    // get a model server
	modelServerPort = ModelManager::getInstance().ensureProcessRunning(model.name(), width, height);
	if (modelServerPort < 0) {
		qDebug() << "ensureProcessRunning failed!";
		isTainted = true;
	}

    // connect to model server
    try {
        socket.connect("tcp://127.0.0.1:" + std::to_string(modelServerPort));
    } catch (zmq::error_t &err) {
        qDebug() << "Connection failed! " << err.num() << ": " << err.what();
        isTainted = true;
    }

    // init rng
    rng = gsl_rng_alloc(gsl_rng_taus);
    gsl_rng_set(rng, std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count());
}

aiAlphaZeroMCTS::~aiAlphaZeroMCTS() {
    delete[] lines;
    gsl_rng_free(rng);
    if (!mctsRootNode.isNull()) {
        mctsRootNode->clear();
    }
}

int aiAlphaZeroMCTS::chooseLine(const QList<bool> &newLines, const QList<int> &newSquareOwners,
                                const QList<Board::Move> &lineHistory) {
    QElapsedTimer moveTimer;
    moveTimer.start();

    if (newLines.size() != linesSize) {
        qCritical() << "something went terribly wrong: newLines.size() != linesSize";
        return -1;
    }

    if (!currentMoveSequence.empty()) {
        return currentMoveSequence.takeFirst();
    }

    for (int i = 0; i < linesSize; i++) {
        lines[i] = newLines[i];
    }
    squareOwners = newSquareOwners;
    // do the ai stuff:
    board = aiBoard::Ptr(new aiBoard(lines, linesSize, width, height, squareOwners, playerId, maxPlayerId));
    currentMoveSequence = mcts();
    if (currentMoveSequence.empty()) {
        qCritical() << "ERROR: mcts returned empty move sequence!";
        return -1;
    }
    lastMoveSequence = currentMoveSequence;
    int line = currentMoveSequence.takeFirst();

    if (line < 0 || line >= linesSize)
    {
        qDebug() << "mcts didn't return a correct line: " << line;
        qDebug() << "coosing random valid move";
        QList<int> freeLines = aiFunctions::getFreeLines(lines, linesSize);
        if (freeLines.empty())
        {
            qDebug() << "no valid lines left!";
            turnTime = moveTimer.elapsed();
            return -1;
        }
        turnTime = moveTimer.elapsed();
        return freeLines.at(qrand() % freeLines.size());
    }

    turnTime = moveTimer.elapsed();
    qDebug() << "elapsed time for Alpha Zero MCTS: " << turnTime;

    return line;
}

QList<int> aiAlphaZeroMCTS::mcts() {
    // init mcts
    mctsTimer.start();

    // create new root node or reuse child node from previous run
    if (mctsRootNode.isNull()) {
        mctsRootNode = AlphaZeroMCTSNode::Ptr(new AlphaZeroMCTSNode());
    } else {
        //mctsRootNode->clear();

        int foundChild = 0;
        int childIdx = -1;
        for (int i = 0; i < mctsRootNode->children.size(); i++) {
            const auto &child = mctsRootNode->children[i];
            if (lastMoveSequence == child->moves) {
                bool movesDone = true;
                for (int move : child->moves) {
                    if (!lines[move]) {
                        movesDone = false;
                    }
                }
                if (movesDone) {
                    foundChild++;
                    childIdx = i;
                }
            }
        }
        if (foundChild == 1) {
            qDebug() << "reusing tree";
            AlphaZeroMCTSNode::Ptr tmpNode(nullptr);
            for (int i = 0; i < mctsRootNode->children.size(); i++) {
                if (childIdx == i) {
                    tmpNode = mctsRootNode->children[i];
                } else {
                    mctsRootNode->children[i]->clear();
                }
            }
            if (!tmpNode.isNull()) {
                mctsRootNode = tmpNode;
                mctsRootNode->parent = AlphaZeroMCTSNode::Ptr(nullptr);
            } else {
                assert(false);
                mctsRootNode->clear();
            }
        } else {
            if (foundChild > 1) {
                qDebug() << "WARNING: found more than one child move";
                assert(false);
            }
            mctsRootNode->clear();
        }
    }

    int finishedIterations = 0;
    // fill mcts tree
    //while (!mctsTimer.hasExpired(mctsTimeout))
    while (finishedIterations < mcts_iterations)
    {
        //assert(playerId == board->playerId);

        // reset priors, then apply dirichlet noise
        if (finishedIterations > 0) {
            int i = 0;
            for (const auto &child : mctsRootNode->children) {
                child->prior = original_priors[i++];
            }
            applyDirichletNoiseToChildren(mctsRootNode, dirichlet_alpha);
        }

        AlphaZeroMCTSNode::Ptr node = selection(mctsRootNode);
        if (node.isNull()) { // sth failed
            qDebug() << "WARNING: MCTS selection step failed";
            break;
        }
        predictPolicyValue(node);
        if (node.isNull()) { // sth failed
            qDebug() << "WARNING: MCTS simulation step failed";
            break;
        }
        backpropagation(node);
        if (node.isNull()) { // sth failed
            qDebug() << "WARNING: MCTS backpropagation step failed";
            break;
        }

        // remember priors of root node
        if (finishedIterations == 0) {
            original_priors.clear();
            for (const auto &child : mctsRootNode->children) {
                original_priors.append(child->prior);
            }
        }

        if (isTainted) {
            qDebug() << "aiAlphaZeroMCTS is tainted, returning invalid line on purpose";
            return QList<int>();
        }

        finishedIterations++;
        QCoreApplication::processEvents();
    }

    //qDebug() << "MCTS iterations: " << finishedIterations;

    // select most promising move
    QList<int> returnSequence;
    long childVisitSum = 0;
    for (const auto &child : mctsRootNode->children) {
        childVisitSum += child->visitCnt;
    }
    if (childVisitSum == 0) {
        qDebug() << "ERROR: childVisitSum = 0, children.size() = " << mctsRootNode->children.size();
        return QList<int>();
    }
    double bestProbability = -INFINITY;
    for (const auto &child : mctsRootNode->children) {
        // select by highest visit count
        double pi_a_given_s0 = (double)child->visitCnt / (double)childVisitSum;
        if (pi_a_given_s0 > bestProbability) {
            bestProbability = pi_a_given_s0;
            returnSequence = child->moves;
            //child->value = pi_a_given_s0;
        }
        // select by best value
        /*
        if (child->value > bestProbability) {
            bestProbability = child->value;
            line = child->move;
        }
         */
    }
    lineVal = -mctsRootNode->value;

    // debug stuff
    /*
    #!/usr/bin/bash
    for file in /tmp/AlphaZeroMCTS.*.dot; do; dot -Tpng $file -o $(basename $file .dot).png; done
     */
    //qDebug().noquote() << "mcts node:" << mctsRootNode->toString();
    //QFile graph(i18n("/tmp/AlphaZeroMCTS.") + QString::number(finishedIterations) + i18n(".dot"));

    if (debug) {
        // write monte carlo tree to file
        QString id = QString::number(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count());
        QString path = QObject::tr("/tmp/AlphaZeroMCTS") +
                id +
                QObject::tr(".dot");
        mctsRootNode->saveAsDot(path);

        // write board to file
        QString path2 = QObject::tr("/tmp/AlphaZeroMCTS") +
                       id +
                       QObject::tr(".txt");
        QFile boardTxt(path2);
        if (boardTxt.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text)) {
            QTextStream stream(&boardTxt);
            stream << boardToString(board);
        }
        boardTxt.flush();
        boardTxt.close();
    }

    return returnSequence;
}

AlphaZeroMCTSNode::Ptr aiAlphaZeroMCTS::selection(const AlphaZeroMCTSNode::Ptr &node) {
    if (node->children.isEmpty()) // reached leaf of mcts tree
    {
        // create children
        KSquares::BoardAnalysis analysis = BoardAnalysisFunctions::analyseBoard(board);
        QSharedPointer<QList<QList<int>>> moveSequences = analysis.moveSequences;
        for (int i = 0; i < moveSequences->size(); i++) {
            AlphaZeroMCTSNode::Ptr child = AlphaZeroMCTSNode::Ptr(new AlphaZeroMCTSNode());
            child->moves = moveSequences->at(i);
            child->ownMove = board->playerId == playerId;
            node->children.append(child);
        }
        return node;
    }

    // actual selection
    AlphaZeroMCTSNode::Ptr selectedNode(nullptr);
    double bestVal = -INFINITY;
    double visitSum = 0;
    for (const auto &child : node->children) {
        visitSum += child->visitCnt;
    }
    double visitSumSqrt = sqrt(visitSum);
    for (int i = 0; i < node->children.size(); i++) {
        node->children[i]->puctValue = node->children[i]->value +
                                       C_puct * node->children[i]->prior * (visitSumSqrt / (1.0 + (double)node->visitCnt));
        //assert(node->ownMove != node->children[i]->ownMove);
        if (node->children[i]->puctValue > bestVal) {
            bestVal = node->children[i]->puctValue;
            selectedNode = node->children[i];
        }
    }

    if (selectedNode.isNull()) {
        qDebug() << "selected node is null, no child has been selected?!";
        qDebug().noquote() << boardToString(board);
        assert(false);
        return selectedNode;
    }

    int currentPlayer = board->playerId;
    for (int i = 0; i < selectedNode->moves.size(); i++) {
        board->doMove(selectedNode->moves[i]);
    }
    //assert(currentPlayer != board->playerId || board->drawnLinesCnt == board->linesSize);

    return selection(selectedNode);
}


bool aiAlphaZeroMCTS::predictPolicyValue(const AlphaZeroMCTSNode::Ptr &node) {

    // reached leaf of search tree?
    //if (board->drawnLinesCnt == board->linesSize) {
    if (node->children.empty()) {
        // score the game
        double val = 0.0;
        for (const auto &owner: board->squareOwners) {
            val += owner == playerId ? 1 : -1;
        }
        val /= board->squareOwners.size();
        // set score
        node->value = val;
        node->leaf = true;
        return true;
    }

    // request prediction
    QImage qimg = MLImageGenerator::generateInputImage(board);
    PolicyValueData policyValueData;
    if (ProtobufConnector::getInstance().getBatchPredict()) {
        policyValueData = ProtobufConnector::getInstance().batchPredict(socket, qimg);
    } else {
        // send prediction request
        DotsAndBoxesImage img = ProtobufConnector::dotsAndBoxesImageToProtobuf(qimg);
        if (!ProtobufConnector::sendString(socket, img.SerializeAsString())) {
            qDebug() << "failed to send message to model server";
            isTainted = true;
            return false;
        }

        // receive prediction from model server
        bool ok = false;
        std::string rpl = ProtobufConnector::recvString(socket, &ok);
        if (!ok) {
            qDebug() << "failed to receive message from model server";
            isTainted = true;
            return false;
        }
        //qDebug() << QString::fromStdString(rpl);
        policyValueData.ParseFromString(rpl);
    }

    // put data into mcts nodes
    //int lineCnt = policyValueData.policy_size();
    double priorSum = 0; // sum up prior to normalize
    node->value = policyValueData.value() * (node->ownMove ? 1 : -1);
    for (const auto &child : node->children) {
        child->parent = node;
        // average over all moves in sequence to get prior
        // TODO: or use the maximum? long chains might have low average?
        // TODO: what about sets of equal lines? move sequences only return one representative line for each set!
        child->prior = 0;
        for (int i = 0; i < child->moves.size(); i++) {
            child->prior += policyValueData.policy(child->moves[i]);
            //child->prior = std::max(child->prior, policyValueData.policy(child->moves[i]));
        }
        child->prior /= (double) child->moves.size();
        if (child->prior == NAN) {
            assert(false);
        }
        priorSum += child->prior;
    }
    ProtobufConnector::getInstance().releaseBatchSample();
    // normalize
    for (const auto &child : node->children) {
        child->prior /= priorSum;
    }

    return true;
}

void aiAlphaZeroMCTS::backpropagation(const AlphaZeroMCTSNode::Ptr &node) {
    double backupValue = node->value;
    AlphaZeroMCTSNode::Ptr tmpNode = node;//AlphaZeroMCTSNode::Ptr(new AlphaZeroMCTSNode(*node.data()));
    while (!tmpNode->parent.isNull()) {
        tmpNode->visitCnt++;
        double invert = tmpNode->ownMove ? 1 : -1;
        tmpNode->fullValue += backupValue * invert;
        tmpNode->value = (double)tmpNode->fullValue / (double)tmpNode->visitCnt;
        //int currentPlayer = board->playerId;
        for (int i = tmpNode->moves.size()-1; i >= 0; i--) {
            board->undoMove(tmpNode->moves[i]);
        }
        //assert(board->playerId != currentPlayer);
        //qDebug() << "backup " << tmpNode->move;
        tmpNode = tmpNode->parent;
    }
    // update the root node
    tmpNode->visitCnt++;
    tmpNode->fullValue += node->value * (tmpNode->ownMove ? 1 : -1);
    tmpNode->value = (double)tmpNode->fullValue / (double)tmpNode->visitCnt;
}

void aiAlphaZeroMCTS::applyDirichletNoiseToChildren(const AlphaZeroMCTSNode::Ptr &parentNode, double alphaValue) {
    size_t K = parentNode->children.size();
    auto alpha = new double[K]();
    auto theta = new double[K]();
    for (int i = 0; i < K; i++) {
        alpha[i] = alphaValue;
        //theta[i] = 0.0;
    }
    double eps = 0.5;
    gsl_ran_dirichlet(rng, K, alpha, theta);
    for (int i = 0; i < K; i++) {
        const auto &child = parentNode->children[i];
        child->prior = (1-eps) * child->prior + eps * theta[i];
    }
}

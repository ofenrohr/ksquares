//
// Created by ofenrohr on 14.04.18.
//

#include "aiAlphaZeroMCTS.h"
#include "aiConvNet.h"

#include <cmath>
#include <chrono>
#include <alphaDots/ProtobufConnector.h>
#include <PolicyValueData.pb.h>
#include <alphaDots/ModelManager.h>
#include <alphaDots/MLImageGenerator.h>

using namespace AlphaDots;

aiAlphaZeroMCTS::aiAlphaZeroMCTS(int newPlayerId, int newMaxPlayerId, int newWidth, int newHeight, int newLevel,
                                 int thinkTime, ModelInfo model) :
        KSquaresAi(newWidth, newHeight), playerId(newPlayerId), maxPlayerId(newMaxPlayerId), level(newLevel),
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

    // init gsl
    rng = gsl_rng_alloc(gsl_rng_taus);
    gsl_rng_set(rng, std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count());
}

aiAlphaZeroMCTS::~aiAlphaZeroMCTS() {
    delete[] lines;
    gsl_rng_free(rng);
}

int aiAlphaZeroMCTS::chooseLine(const QList<bool> &newLines, const QList<int> &newSquareOwners,
                                const QList<Board::Move> &lineHistory) {
    QElapsedTimer moveTimer;
    moveTimer.start();

    if (newLines.size() != linesSize) {
        qCritical() << "something went terribly wrong: newLines.size() != linesSize";
    }
    for (int i = 0; i < linesSize; i++) {
        lines[i] = newLines[i];
    }
    squareOwners = newSquareOwners;
    // do the ai stuff:
    board = aiBoard::Ptr(new aiBoard(lines, linesSize, width, height, squareOwners, playerId, maxPlayerId));
    int line = mcts();

    if (line < 0 || line >= linesSize)
    {
        qDebug() << "mcts didn't return a correct line: " << line;
        qDebug() << "coosing random valid move";
        QList<int> freeLines = aiFunctions::getFreeLines(lines, linesSize);
        if (freeLines.empty())
        {
            qDebug() << "no valid lines left!";
            turnTime = moveTimer.elapsed();
            return 0;
        }
        turnTime = moveTimer.elapsed();
        return freeLines.at(qrand() % freeLines.size());
    }

    turnTime = moveTimer.elapsed();
    return line;
}

int aiAlphaZeroMCTS::mcts() {
    // init mcts
    mctsTimer.start();
    mctsRootNode = AlphaZeroMCTSNode::Ptr(new AlphaZeroMCTSNode());

    int mctsIterations = 0;
    // fill mcts tree
    //while (!mctsTimer.hasExpired(mctsTimeout))
    while (mctsIterations < mcts_iterations)
    {
        //applyDirichletNoiseToChildren(mctsRootNode, dirichlet_alpha / (linesSize - mctsRootNode->children.size()));

        AlphaZeroMCTSNode::Ptr node = selection(mctsRootNode);
        if (node.isNull()) { // sth failed
            qDebug() << "WARNING: MCTS selection step failed";
            break;
        }
        simulation(node);
        if (node.isNull()) { // sth failed
            qDebug() << "WARNING: MCTS simulation step failed";
            break;
        }
        backpropagation(node);
        if (node.isNull()) { // sth failed
            qDebug() << "WARNING: MCTS backpropagation step failed";
            break;
        }

        mctsIterations++;
        QCoreApplication::processEvents();

        // debug stuff
        /*
        #!/usr/bin/bash
        for file in /tmp/AlphaZeroMCTS.*.dot; do; dot -Tpng $file -o $(basename $file .dot).png; done
         */
        //qDebug().noquote() << "mcts node:" << mctsRootNode->toString();
        /*
        QFile graph(i18n("/tmp/AlphaZeroMCTS.") + QString::number(mctsIterations) + i18n(".dot"));
        if (graph.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text)) {
            QTextStream stream(&graph);
            stream << "digraph {";
            stream << mctsRootNode->toDotString();
            stream << "}";
        }
         */
    }

    //qDebug() << "MCTS iterations: " << mctsIterations;

    // select most promising move
    // TODO: update
    int line = -1;
    long childVisitSum = 0;
    for (const auto &child : mctsRootNode->children) {
        childVisitSum += child->visitCnt;
    }
    if (childVisitSum == 0) {
        qDebug() << "ERROR: childVisitSum = 0, children.size() = " << mctsRootNode->children.size();
        return -1;
    }
    long mostVisited = -1;
    for (const auto &child : mctsRootNode->children) {
        double pi_a_given_s0 = (double)child->visitCnt / (double)childVisitSum;
        if (pi_a_given_s0 > mostVisited) {
            mostVisited = pi_a_given_s0;
            line = child->move;
            child->value = pi_a_given_s0;
        }
    }


    return line;
}

AlphaZeroMCTSNode::Ptr aiAlphaZeroMCTS::selection(const AlphaZeroMCTSNode::Ptr &node) {
    if (node->children.isEmpty()) // reached leaf of mcts tree
    {
        // create children
        for (int i = 0; i < board->linesSize; i++) {
            if (!board->lines[i]) {
                AlphaZeroMCTSNode::Ptr child = AlphaZeroMCTSNode::Ptr(new AlphaZeroMCTSNode());
                child->move = i;
                child->visitCnt = 0;
                child->fullValue = child->value = child->prior = 0.0;
                node->children.append(child);
            }
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
    for (int i = 0; i < node->children.size(); i++) {
        node->children[i]->puctValue = node->children[i]->value + C_puct * node->children[i]->prior * (sqrt(visitSum) / (1.0 + (double)node->visitCnt));
        if (node->children[i]->puctValue > bestVal) {
            bestVal = node->children[i]->puctValue;
            selectedNode = node->children[i];
        }
    }

    if (selectedNode.isNull()) {
        qDebug() << "selected node is null, no child has been selected?!";
        return selectedNode;
    }

    board->doMove(selectedNode->move);

    return selection(selectedNode);
}

void aiAlphaZeroMCTS::simulation(const AlphaZeroMCTSNode::Ptr &node) {
    // fill values
    predictPolicyValue(node, board);
}

bool aiAlphaZeroMCTS::predictPolicyValue(const AlphaZeroMCTSNode::Ptr &parentNode, const aiBoard::Ptr &board) {
    // send prediction request
    DotsAndBoxesImage img = ProtobufConnector::dotsAndBoxesImageToProtobuf(MLImageGenerator::generateInputImage(board));
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
    PolicyValueData policyValueData;
    policyValueData.ParseFromString(rpl);

    // put data into mcts nodes
    //int lineCnt = policyValueData.policy_size();
    parentNode->value = policyValueData.value();
    for (const auto &child : parentNode->children) {
        child->parent = parentNode;
        child->prior = policyValueData.policy(child->move);
    }

    return true;
}

void aiAlphaZeroMCTS::backpropagation(const AlphaZeroMCTSNode::Ptr &node) {
    AlphaZeroMCTSNode::Ptr tmpNode = node;//AlphaZeroMCTSNode::Ptr(new AlphaZeroMCTSNode(*node.data()));
    while (!tmpNode->parent.isNull()) {
        tmpNode->visitCnt++;
        tmpNode->fullValue += node->value;
        tmpNode->value = (double)tmpNode->fullValue / (double)tmpNode->visitCnt;
        board->undoMove(tmpNode->move);
        //qDebug() << "backup " << tmpNode->move;
        tmpNode = tmpNode->parent;
    }
    tmpNode->visitCnt++;
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

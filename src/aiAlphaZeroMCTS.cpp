//
// Created by ofenrohr on 14.04.18.
//

#include "aiAlphaZeroMCTS.h"
#include "aiConvNet.h"

#include <cmath>
#include <alphaDots/ProtobufConnector.h>

using namespace AlphaDots;

aiAlphaZeroMCTS::aiAlphaZeroMCTS(int newPlayerId, int newMaxPlayerId, int newWidth, int newHeight, int newLevel,
                                 int thinkTime) :
        KSquaresAi(newWidth, newHeight), playerId(newPlayerId), maxPlayerId(newMaxPlayerId), level(newLevel),
        mctsTimeout(thinkTime)
{
    width = newWidth;
    height = newHeight;
    linesSize = toLinesSize(width, height);
    lines = new bool[linesSize];
    for (int i = 0; i < linesSize; i++)
        lines[i] = false;
    mctsTimer = QElapsedTimer();
    simAi = KSquaresAi::Ptr(new aiConvNet(0, 1, width, height, 0, thinkTime, ProtobufConnector::getModelByName(QStringLiteral("alphaZeroV5"))));
}

aiAlphaZeroMCTS::~aiAlphaZeroMCTS() {
    delete[] lines;
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
    while (!mctsTimer.hasExpired(mctsTimeout))
    {
        AlphaZeroMCTSNode::Ptr node = selection(mctsRootNode);
        if (node.isNull()) { // sth failed
            qDebug() << "WARNING: MCTS selection step failed";
            break;
        }
        simulation(node);
        backpropagation(node);

        mctsIterations++;
    }

    qDebug() << "MCTS iterations: " << mctsIterations;

    // select most promising move
    // TODO: update
    int line = -1;
    long mostVisited = -1;
    AlphaZeroMCTSNode::Ptr retNode;
    for (int i = 0; i < mctsRootNode->children.size(); i++)
    {
        if (mctsRootNode->children[i]->visitCnt > mostVisited)
        {
            line = mctsRootNode->children[i]->moveSequence[0];
            mostVisited = mctsRootNode->children[i]->visitCnt;
            retNode = mctsRootNode->children[i];
        }
    }
    qDebug().noquote() << "mcts node:" << mctsRootNode->toString();
    qDebug().noquote() << mctsRootNode->toDotString();
    return line;
}

AlphaZeroMCTSNode::Ptr aiAlphaZeroMCTS::selection(AlphaZeroMCTSNode::Ptr node) {
    if (node->children.isEmpty()) // reached leaf of mcts tree
    {
        // compute (game) children of (mcts) leaf node
        QList<int> moveSeqToNode;
        AlphaZeroMCTSNode::Ptr tmpNode = node;
        while (!tmpNode->parent.isNull())
        {
            for (int i = tmpNode->moveSequence.size() - 1; i >= 0; i--)
                moveSeqToNode.prepend(tmpNode->moveSequence[i]);
            tmpNode = tmpNode->parent;
        }
        for (int i : moveSeqToNode) {
            board->doMove(i);
        }
        KSquares::BoardAnalysis analysis = BoardAnalysisFunctions::analyseBoard(board);

        for (int i = moveSeqToNode.size() - 1; i >= 0; i--)
        {
            board->undoMove(moveSeqToNode[i]);
        }
        for (const auto &moveSeq : *analysis.moveSequences) {
            AlphaZeroMCTSNode::Ptr newNode(new AlphaZeroMCTSNode());
            newNode->moveSequence.append(moveSeq);
            newNode->parent = node;
            if (qrand() % 2)
                node->children.append(newNode);
            else
                node->children.prepend(newNode);
        }
        return node;
    }

    // actual selection
    AlphaZeroMCTSNode::Ptr selectedNode;
    double bestVal = -INFINITY;
    double C = 4;
    double visitSum = 0;
    for (const auto &child : node->children) {
        visitSum += child->visitCnt;
    }
    for (int i = 0; i < node->children.size(); i++)
    {
        double pucbVal = node->children[i]->value + C * node->children[i]->prior * (sqrt(visitSum) / (1 + node->visitCnt));
        if (pucbVal > bestVal)
        {
            bestVal = pucbVal;
            selectedNode = node->children[i];
        }
    }

    if (selectedNode.isNull())
    {
        qDebug() << "selected node is null, no child has been selected?!";
        return selectedNode;
    }

    return selection(selectedNode);
}

void aiAlphaZeroMCTS::simulation(AlphaZeroMCTSNode::Ptr node) {
    QVector<int> simulationHistory;
    int missingLines = 0;
    QList<bool> linesList;
    for (int i = 0; i < board->linesSize; i++) {
        if (!board->lines[i]) {
            missingLines++;
            linesList.append(false);
        } else {
            linesList.append(true);
        }
    }
    simulationHistory.reserve(missingLines);
    // do simulation
    while (missingLines > 0)
    {
        int line = simAi->chooseLine(linesList, QList<int>(), QList<Board::Move_t>());
        board->doMove(line);
        linesList[line] = true;
        simulationHistory.append(line);
        missingLines--;
    }
    // get simulation result
    int tmp = 0;
    for (int squareOwner : board->squareOwners) {
        tmp += squareOwner == playerId ? 1 : -1;
    }
    node->fullValue = tmp > 0 ? 1 : (tmp < 0 ? -1 : 0);
    node->value = node->fullValue;
    // undo simulation
    for (int i = simulationHistory.size() -1; i >= 0; i--)
    {
        board->undoMove(simulationHistory[i]);
    }
}

void aiAlphaZeroMCTS::backpropagation(AlphaZeroMCTSNode::Ptr node) {
    AlphaZeroMCTSNode::Ptr tmpNode = node;
    //int addValue = node->value;
    while (!tmpNode->parent.isNull())
    {
        tmpNode->visitCnt++;
        tmpNode->fullValue += node->value;
        tmpNode->value = (double)tmpNode->fullValue / (double)tmpNode->visitCnt;
        tmpNode = tmpNode->parent;
    }
}

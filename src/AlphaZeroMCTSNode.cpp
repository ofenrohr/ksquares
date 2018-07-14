//
// Created by ofenrohr on 14.04.18.
//

#include <QtCore/QTextStream>
#include <QtCore/QFile>
#include "AlphaZeroMCTSNode.h"

using namespace AlphaDots;

AlphaZeroMCTSNode::AlphaZeroMCTSNode() {
    visitCnt = 0;
    fullValue = 0;
    value = 0;
    prior = 0;
    moves.clear();
    puctValue = 0;
    leaf = false;
    ownMove = false;
    children.clear();
}

AlphaZeroMCTSNode::AlphaZeroMCTSNode(const AlphaZeroMCTSNode &node) {
    //uuid = QUuid::createUuid();
    visitCnt = node.visitCnt;
    fullValue = node.fullValue;
    value = node.value;
    prior = node.prior;
    parent = node.parent;
    moves = node.moves;
    puctValue = node.puctValue;
    leaf = node.leaf;
    ownMove = node.ownMove;
    children = node.children;
}

AlphaZeroMCTSNode::~AlphaZeroMCTSNode() {
    clear();
}

void AlphaZeroMCTSNode::createUUID() {
    if (uuid.isNull()) {
        uuid = QUuid::createUuid();
    }
}

QString AlphaZeroMCTSNode::toDotString() {
    createUUID();
    QString ret;
    QString moveList = QObject::tr("[");
    for (int i = 0; i < moves.size(); i++) {
        if (i > 0) {
            moveList.append(QObject::tr(","));
        }
        moveList.append(QString::number(moves[i]));
    }
    moveList.append(QObject::tr("]"));
    ret.append(getNodeName() + QStringLiteral(" [label=\"")+moveList+QStringLiteral(", V: ") +
               QString::number(value, 'f', 3) + QStringLiteral(", N: ") + QString::number(visitCnt) +
               QStringLiteral(", P: ") + QString::number(prior, 'f', 2) + QStringLiteral(", U: ") + QString::number(puctValue, 'f', 3) +
               QStringLiteral("\", shape=\"") + ( leaf ? QStringLiteral("ellipse") : QStringLiteral("box") ) + QStringLiteral("\"") +
               (!ownMove ? QStringLiteral(", fillcolor=yellow, style=filled") : QStringLiteral("") ) + QStringLiteral("];\n")
    );
    for (auto &child : children) {
        if (child->visitCnt > 0) {
            child->createUUID();
            ret.append(child->toDotString());
            ret.append(getNodeName() + QStringLiteral(" -> ") + child->getNodeName() + QStringLiteral(";\n"));
        }
    }
    return ret;
}

QString AlphaZeroMCTSNode::getNodeName() {
    QString id = uuid.toString()
            .replace(QLatin1String("-"),QLatin1String(""))
            .replace(QLatin1String("{"),QLatin1String(""))
            .replace(QLatin1String("}"),QLatin1String(""))
    ;
    return QStringLiteral("node") + id;
}

QString AlphaZeroMCTSNode::toString() {
    QString ret = QStringLiteral("MCTSNode: value=")+QString::number(value)+QStringLiteral(", visitedCnt=")+QString::number(visitCnt);
    for (auto &child : children) {
        ret.append(QStringLiteral("\nchild value=")+QString::number(child->value)+QStringLiteral(", visitedCnt=") + QString::number(child->visitCnt));
    }
    return ret;
}

void AlphaZeroMCTSNode::clear() {
    visitCnt = 0;
    fullValue = 0.0;
    value = 0.0;
    prior = 0.0;
    moves.clear();
    puctValue = 0;
    parent = AlphaZeroMCTSNode::Ptr(nullptr);
    for (const auto &child : children) {
        child->clear();
    }
    children.clear();
    ownMove = false;
    leaf = false;
}

void AlphaZeroMCTSNode::saveAsDot(QString &path) {
    QFile graph(path);
    if (graph.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text)) {
        QTextStream stream(&graph);
        stream << "digraph {";
        stream << toDotString();
        stream << "}";
    }
    graph.flush();
    graph.close();
}

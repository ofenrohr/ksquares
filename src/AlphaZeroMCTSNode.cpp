//
// Created by ofenrohr on 14.04.18.
//

#include "AlphaZeroMCTSNode.h"

using namespace AlphaDots;

AlphaZeroMCTSNode::AlphaZeroMCTSNode() {
    visitCnt = 0;
    fullValue = 0;
    value = 0;
    prior = 0;
    move = -1;
    children.clear();
}

AlphaZeroMCTSNode::AlphaZeroMCTSNode(const AlphaZeroMCTSNode &node) {
    //uuid = QUuid::createUuid();
    visitCnt = node.visitCnt;
    fullValue = node.fullValue;
    value = node.value;
    prior = node.prior;
    parent = node.parent;
    move = node.move;
    children = node.children;
}

void AlphaZeroMCTSNode::createUUID() {
    if (uuid.isNull()) {
        uuid = QUuid::createUuid();
    }
}

QString AlphaZeroMCTSNode::toDotString() {
    createUUID();
    QString ret;
    ret.append(getNodeName() + QStringLiteral(" [label=\"")+QString::number(move)+QStringLiteral(", V: ") +
               QString::number(value, 'f', 3) + QStringLiteral(", N: ") + QString::number(visitCnt) +
               QStringLiteral(", P: ") + QString::number(prior, 'f', 2) + QStringLiteral(", U: ") + QString::number(puctValue, 'f', 3) +
               QStringLiteral("\", shape=\"box\"];\n")
    );
    for (auto &child : children) {
        if (child->visitCnt >= 0) {
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

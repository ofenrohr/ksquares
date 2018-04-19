//
// Created by ofenrohr on 14.04.18.
//

#ifndef KSQUARES_ALPHAZEROMCTSNODE_H
#define KSQUARES_ALPHAZEROMCTSNODE_H


#include <QtCore/QSharedPointer>
#include <QtCore/QUuid>

namespace AlphaDots {
    class AlphaZeroMCTSNode {
    public:
        typedef QSharedPointer<AlphaZeroMCTSNode> Ptr;

        AlphaZeroMCTSNode();
        AlphaZeroMCTSNode(const AlphaZeroMCTSNode &node);

        /// times this node was visited (N)
        int visitCnt;
        /// accumulated value over all visits (W)
        double fullValue;
        /// mean action value (Q)
        double value;
        /// prior probability (P) of this node according to the policy network
        double prior;
        /// lines drawn to get from parent node to this one
        QList<int> moveSequence;

        /// Parent node
        AlphaZeroMCTSNode::Ptr parent;
        /// Child nodes
        QList<AlphaZeroMCTSNode::Ptr> children;

        /// debug flag
        bool debug;
        /// node id used for printing
        QUuid uuid;
        /// call to create uuid
        void createUUID();
        /// export data as dot graph
        QString toDotString();
        /// printable node id
        QString getNodeName();
        /// printable info about node and its direct children
        QString toString();
    };
}

#endif //KSQUARES_ALPHAZEROMCTSNODE_H

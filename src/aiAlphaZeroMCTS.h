//
// Created by ofenrohr on 14.04.18.
//

#ifndef KSQUARES_AIALPHAZEROMCTS_H
#define KSQUARES_AIALPHAZEROMCTS_H


#include <QtCore/QElapsedTimer>
#include "aicontroller.h"
#include "AlphaZeroMCTSNode.h"

namespace AlphaDots {
    class aiAlphaZeroMCTS : public KSquaresAi {
    public:
        aiAlphaZeroMCTS(int newPlayerId, int newMaxPlayerId, int newWidth, int newHeight, int newLevel,
                        int thinkTime = 5000);

        ~aiAlphaZeroMCTS();

        int chooseLine(const QList<bool> &newLines, const QList<int> &newSquareOwners,
                       const QList<Board::Move> &lineHistory);

        QString getName() { return QStringLiteral("alphazeromcts"); }

        virtual bool enabled() { return true; }

        virtual bool tainted() { return false; }

        virtual long lastMoveTime() { return turnTime; }

    protected:
        int mcts();

        AlphaZeroMCTSNode::Ptr selection(AlphaZeroMCTSNode::Ptr node);

        void simulation(AlphaZeroMCTSNode::Ptr node);

        void backpropagation(AlphaZeroMCTSNode::Ptr node);

        /// The ID of the player this AI belongs to
        int playerId;
        /// number of players - 1
        int maxPlayerId;
        /// board width in squares
        int width;
        /// board height in squares
        int height;
        /// The strength of the ai
        int level;
        /// List of the owners of each square
        QList<int> squareOwners;
        /// Array of the lines on the board
        bool *lines;
        /// mcts tree root node
        AlphaZeroMCTSNode::Ptr mctsRootNode;
        /// initial board
        aiBoard::Ptr board;

        KSquaresAi::Ptr simAi;

        /// time logging
        long turnTime;

        QElapsedTimer mctsTimer;
        long mctsTimeout;

    };
}


#endif //KSQUARES_AIALPHAZEROMCTS_H

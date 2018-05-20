//
// Created by ofenrohr on 14.04.18.
//

#ifndef KSQUARES_AIALPHAZEROMCTS_H
#define KSQUARES_AIALPHAZEROMCTS_H


#include <QtCore/QElapsedTimer>
#include <zmq.hpp>
#include <alphaDots/ProtobufConnector.h>
#include "aicontroller.h"
#include "AlphaZeroMCTSNode.h"
#include <gsl/gsl_randist.h>
#include <klocalizedstring.h>

namespace AlphaDots {
    class aiAlphaZeroMCTS : public KSquaresAi {
    public:
        //==========================
        // KSquaresAi interface stuff
        //==========================

        aiAlphaZeroMCTS(int newPlayerId, int newMaxPlayerId, int newWidth, int newHeight, int newLevel,
                        int thinkTime = 5000, ModelInfo model = ProtobufConnector::getModelByName(i18n("AlphaZeroV7")));

        ~aiAlphaZeroMCTS();

        int chooseLine(const QList<bool> &newLines, const QList<int> &newSquareOwners,
                       const QList<Board::Move> &lineHistory);

        QString getName() { return i18n("alphazeromcts"); }

        virtual bool enabled() { return true; }

        virtual bool tainted() { return isTainted; }

        virtual long lastMoveTime() { return turnTime; }


        //==========================
        // AlphaZero stuff
        //==========================

        /**
         * Send a board to the python model server and write the result to the parentNode and its children.
         * The parentNode's prior will be set to the predicted value and the child nodes' value is set to the
         * predicted policy probabilities.
         * @param parentNode parent AlphaZero MCTS Node
         * @param board current board state that's associated with the parentNode
         * @return true if there was no error, false otherwise
         */
        bool predictPolicyValue(const AlphaZeroMCTSNode::Ptr &parentNode, const aiBoard::Ptr &board);

        void applyDirichletNoiseToChildren(const AlphaZeroMCTSNode::Ptr &parentNode, double alpha);

    protected:
        int mcts();

        AlphaZeroMCTSNode::Ptr selection(const AlphaZeroMCTSNode::Ptr &node);

        void simulation(const AlphaZeroMCTSNode::Ptr &node);

        void backpropagation(const AlphaZeroMCTSNode::Ptr &node);

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
        /// tainted flag
        bool isTainted;
        /// mcts tree root node
        AlphaZeroMCTSNode::Ptr mctsRootNode;
        /// initial board
        aiBoard::Ptr board;

        // model server
        int modelServerPort;
        zmq::context_t context;
        zmq::socket_t socket;

        // gsl random number generator
        gsl_rng *rng;

        // Hyperparameters
        double C_puct = 10.0;
        double dirichlet_alpha = 0.03;
        double mcts_iterations = 1000;

        /// time logging
        long turnTime;

        QElapsedTimer mctsTimer;
        long mctsTimeout;

    };
}


#endif //KSQUARES_AIALPHAZEROMCTS_H

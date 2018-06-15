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

        aiAlphaZeroMCTS(int newPlayerId, int newMaxPlayerId, int newWidth, int newHeight,
                        int thinkTime = 5000, ModelInfo model = ProtobufConnector::getInstance().getModelByName(i18n("AlphaZeroV11")));

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
        bool predictPolicyValue(const AlphaZeroMCTSNode::Ptr &parentNode, const aiBoard::Ptr &board,
                                const QImage &inputImage);

        void applyDirichletNoiseToChildren(const AlphaZeroMCTSNode::Ptr &parentNode, double alpha);

        static void setDebug(bool mode) {debug=mode;}

        // Hyperparameters
        static double C_puct;// = 10.0;
        static double dirichlet_alpha;// = 0.03;
        static int mcts_iterations;// = 1500;
        static double prior_eps;// = 0;//0.001;

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
        //int level;
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
        /// last returned line
        int lastLine;

        // model server
        int modelServerPort;
        zmq::context_t context;
        zmq::socket_t socket;

        // GSLTest random number generator
        gsl_rng *rng;

        // list of original prior values
        QList<double> original_priors;

        /// time logging
        long turnTime;

        QElapsedTimer mctsTimer;
        long mctsTimeout;

        static bool debug;
    };
}


#endif //KSQUARES_AIALPHAZEROMCTS_H

//
// Created by ofenrohr on 22.07.18.
//

#ifndef KSQUARES_AILEVELLIST_H
#define KSQUARES_AILEVELLIST_H

namespace KSquares {
    enum AILevel {
        AI_EASY,
        AI_MEDIUM,
        AI_HARD,
        AI_VERYHARD,
        AI_DABBLE,
        AI_DABBLENOHASH,
        AI_QDAB,
        AI_KNOX,
        AI_MCTS_A,
        AI_MCTS_B,
        AI_MCTS_C,
        AI_DABBLENATIVE,
        AI_CONVNET,
        AI_MCTS_CONVNET,
        AI_MCTS_ALPHAZERO
        // if you add an ai here, also add it to alphaDots/modelEvaluation/ModelEvaluation.cpp -> parseAiLevel
    };
}

#endif //KSQUARES_AILEVELLIST_H

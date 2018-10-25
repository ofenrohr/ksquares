//
// Created by ofenrohr on 22.10.18.
//

#include "DoubleDealingAnalysis.h"

DoubleDealingAnalysis::DoubleDealingAnalysis() {
    _headers = QList<QString>() << "Double Dealing";
}

DoubleDealingAnalysis::~DoubleDealingAnalysis() = default;

QList<QVariant> DoubleDealingAnalysis::analyseResult(AITestResult &result, bool *ok) {
    *ok = true;
    auto doubleDealingCounter = QList<int>() << 0;
    aiBoard::Ptr board = aiBoard::Ptr(new aiBoard(result.setup.boardSize.x(), result.setup.boardSize.y()));
    int analysePlayerId = result.setup.aiLevelP1 > 0 ? 0 : 1;
    /*
    for (int move : result.autoFillMoves) {
        board->doMove(move);
    }
     */
    for (int move : result.moves) {
        int tmpPlayer = board->playerId;
        board->doMove(move);
        // check if player changed
        if (board->playerId != tmpPlayer) {
            // analyse board on change
            auto analysis = BoardAnalysisFunctions::analyseBoard(board);
            // if Double Dealing happened, there are two options:
            // 1. Double Dealing in a long or short chain -> 1 capturable short chain with 2 boxes, nothing else
            //    (there could be other open chains, but that would not make much sense, except if it's situation 2)
            // 2. Double Dealing in a loop chain -> 2 capturable short chains with 2 boxes each
            //
            // technically it is possible to do Double Dealing when there are other capturable chains, but
            // that most probably is not an optimal strategy.
            if (analysis.capturableLongChains.empty() &&
                analysis.capturableLoopChains.empty())
            {
                // option 1: Double Dealing in long or short chain
                if (analysis.capturableShortChains.size() == 1) {
                    if (analysis.chains[analysis.capturableShortChains[0]].squares.size() == 2 &&
                        analysis.chains[analysis.capturableShortChains[0]].lines.size() == 1)
                    {
                        //doubleDealingCounter[board->playerId]++;
                        if (board->playerId != analysePlayerId) {
                            doubleDealingCounter[0]++;
                        }
                        //qDebug().noquote() << aiFunctions::boardToString(board);
                    }
                }
                // option 2: Double Dealing in cyclic chains
                if (analysis.capturableShortChains.size() == 2) {
                    if (analysis.chains[analysis.capturableShortChains[0]].squares.size() == 2 &&
                        analysis.chains[analysis.capturableShortChains[0]].lines.size() == 1 &&
                        analysis.chains[analysis.capturableShortChains[1]].squares.size() == 2 &&
                        analysis.chains[analysis.capturableShortChains[1]].lines.size() == 1)
                    {
                        //doubleDealingCounter[board->playerId]++;
                        if (board->playerId != analysePlayerId) {
                            doubleDealingCounter[0]++;
                        }
                    }
                }
            }
        }
    }
    return QList<QVariant>() << QVariant(doubleDealingCounter[0]);
}

int DoubleDealingAnalysis::headerCount() {
    return _headers.size();
}

QList<QString> DoubleDealingAnalysis::headers() {
    return _headers;
}

QString DoubleDealingAnalysis::moduleName() {
    return "DoubleDealing";
}

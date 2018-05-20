//
// Created by ofenrohr on 20.05.18.
//

#ifndef KSQUARES_TESTUTILS_H
#define KSQUARES_TESTUTILS_H


#include <board.h>

class testutils {
public:
    static void executeAi(QList<int> testAIs, Board *board, int player, std::string name, QList<int> expectedLines);
};


#endif //KSQUARES_TESTUTILS_H

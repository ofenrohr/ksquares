//
// Created by ofenrohr on 8/10/17.
//

#ifndef KSQUARES_AIDABBLENATIVE_H
#define KSQUARES_AIDABBLENATIVE_H

#include "aicontroller.h"
#include "aifunctions.h"
#include "board.h"
#include "aiBoard.h"
#include "dbgame-nohash.h"

#include <QString>

class aiDabbleNative : public KSquaresAi {
public:
    aiDabbleNative(int newPlayerId, int newMaxPlayerId, int newWidth, int newHeight, int newLevel, int thinkTime = 5000);

    ~aiDabbleNative();

    int chooseLine(const QList<bool> &newLines, const QList<int> &newSquareOwners, const QList<Board::Move> &lineHistory);
    QString getName() { return QStringLiteral("DabbleNative"); }
    virtual bool enabled() { return true; }
    virtual bool tainted() { return false; }
    virtual long lastMoveTime() { return turnTime; }

protected:
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
    /// number of lines on board
    //int linesSize;
    /// List of the owners of each square
    QList<int> squareOwners;
    /// Array of the lines on the board
    bool *lines;
    /// time logging
    long turnTime;
    /// initial board
    aiBoard::Ptr board;

    long moveTimeout;

    dabble_nohash::DBGame *m_game;
};


#endif //KSQUARES_AIDABBLENATIVE_H

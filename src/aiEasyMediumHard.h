/***************************************************************************
 *   Copyright (C) 2014 by Tom Vincent Peters   <kde@vincent-peters.de>    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "aicontroller.h"
#include "aifunctions.h"
#include "board.h"

/**
 * @short AI Easy Medium and Hard for KSquares
 *
 * When playing a game of squares there are a number of stages the game goes through:
 * @li The random line placement stage. Where players are just placing lines while trying to not complete the third side of any squares
 * @li Next players will try to only draw the third side of a square if it will only give the opponent the minimum amount of points
 * @li The more advanced player will, at the end of a large run of squares leave a small area at the end, forcing the opponent to take only that small section, leaving another large area open to him.
 * @li The even more advanced player will fight for control over the game. This means that he will count the chains forming in the last phase of the "random line" game phase and thus make sure that he will be the one who gets the first long chain. This works like a Nim game somehow.
 * Currently, the first three points are implemented.
 *
 * @author Matt Williams <matt@milliams.com>
 * @author Tom Vincent Peters <kde@vincent-peters.de>
 */

class aiEasyMediumHard : public KSquaresAi {
public:
    aiEasyMediumHard(int newPlayerId, int newWidth, int newHeight, int newLevel);

    //~aiEasyMediumHard();
    int
    chooseLine(const QList<bool> &newLines, const QList<int> &newSquareOwners, const QList<Board::Move> &lineHistory);

    QString getName();

    virtual bool enabled() { return true; }

    virtual bool tainted() { return false; }

    virtual long lastMoveTime() { return lastTurnTime; }

    int chooseLine(bool *linesB);

    /**
     * Finds chains on board that can be taken by the player to move
     * this is public to make testing easier
     * @param lines lines of the board
     * @param linesSize number of lines on the board
     * @param width width of board in boxes
     * @param height height of board in boxes
     * @param ownChains returns the chains the current player can score
     * @return number of squares taken
     */
    static int findOwnChains(bool *lines, int linesSize, int width, int height, QList<QList<int> > *ownChains);

protected:
    /**
     * @param choiceList list of indices (of lines) which have squares next to them with two lines drawn (relates to @ref lines )
     *
     * @return list of indices (of lines) which would be the least damaging in the short term
     */
    QList<int> chooseLeastDamaging(const QList<int> &choiceList) const;

    /// The ID of the player this AI belongs to
    int playerId;
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
    long lastTurnTime;
};

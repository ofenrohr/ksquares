/***************************************************************************
 *   Copyright (C) 2015 by Tom Vincent Peters   <kde@vincent-peters.de>    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef AIMCTS_H
#define AIMCTS_H

#include "aicontroller.h"
#include "aifunctions.h"
#include "board.h"
#include "aiBoard.h"

#include <QString>
#include <QElapsedTimer>
#include <QPair>
#include <QSharedPointer>

class MCTSNode
{
public:
    typedef QSharedPointer<MCTSNode> Ptr;

    MCTSNode();

    long visitedCnt;
    double value;
    long fullValue;
    QList<int> moveSequence; // lines drawn to get from parent node to this one
    MCTSNode::Ptr parent;
    QList<MCTSNode::Ptr> children;
    bool gameLeaf;
    bool inTree;

    QString toString();

};

class aiMCTS : public KSquaresAi
{
	public:
		aiMCTS(int newPlayerId, int newMaxPlayerId, int newWidth, int newHeight, int newLevel, int thinkTime = 5000);
		~aiMCTS();
		
		int chooseLine(const QList<bool> &newLines, const QList<int> &newSquareOwners, const QList<Board::Move> &lineHistory);
		QString getName() { return QStringLiteral("mcts"); }
		virtual bool enabled() { return true; }
		virtual bool tainted() { return false; }
		virtual long lastMoveTime() { return turnTime; }

	protected:
		int mcts();
		
		MCTSNode::Ptr selection(MCTSNode::Ptr node);
		void expansion(MCTSNode::Ptr node);
		void simulation(MCTSNode::Ptr node);
		void backpropagation(MCTSNode::Ptr node);
		
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
		/// mcts tree root node
		MCTSNode::Ptr mctsRootNode;
		/// initial board
		aiBoard::Ptr board;

		KSquaresAi::Ptr simAi;

		/// time logging
		long turnTime;
		
		QElapsedTimer mctsTimer;
		long mctsTimeout;
};

#endif

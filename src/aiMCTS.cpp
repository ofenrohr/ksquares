/***************************************************************************
 *   Copyright (C) 2015 by Tom Vincent Peters   <kde@vincent-peters.de>    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "aiMCTS.h"

#include <cmath>
#include <limits>
#include <QDebug>
#include <algorithm>
#include <QElapsedTimer>
#include <QMap>
#include <QPair>

#include "boardAnalysis.h"
#include "aiEasyMediumHard.h"
#include "aiConvNet.h"

aiMCTS::aiMCTS(int newPlayerId, int newMaxPlayerId, int newWidth, int newHeight, int newLevel, int thinkTime) : KSquaresAi(newWidth, newHeight), playerId(newPlayerId), maxPlayerId(newMaxPlayerId), level(newLevel), mctsTimeout(thinkTime)
{
	width = newWidth;
	height = newHeight;
	linesSize = toLinesSize(width, height);
	lines = new bool[linesSize];
	for (int i = 0; i < linesSize; i++)
		lines[i] = false;
	mctsTimer = QElapsedTimer();
	switch (level)
	{
		default:
			level = KSquares::AI_EASY;
		case KSquares::AI_EASY:
		case KSquares::AI_MEDIUM:
		case KSquares::AI_HARD:
			simAi = KSquaresAi::Ptr(new aiConvNet(0, 1, width, height, 0));
            break;
		case KSquares::AI_CONVNET:
			simAi = KSquaresAi::Ptr(new aiEasyMediumHard(0, width, height, level));
		break;
	}
}

aiMCTS::~aiMCTS()
{
	delete[] lines;
}

MCTSNode::MCTSNode()
{
	visitedCnt = 0;
	value = 0.0;
	moveSequence = QList<int>();
	parent = MCTSNode::Ptr(nullptr);
	children = QList<MCTSNode::Ptr>();
	gameLeaf = false;
	inTree = false;
}

int aiMCTS::chooseLine(const QList<bool> &newLines, const QList<int> &newSquareOwners, const QList<Board::Move> &/*lineHistory*/)
{
	QElapsedTimer moveTimer;
	moveTimer.start();
	
	if (newLines.size() != linesSize)
	{
		qCritical() << "something went terribly wrong: newLines.size() != linesSize";
	}
	for (int i = 0; i < linesSize; i++)
		lines[i] = newLines[i];
	squareOwners = newSquareOwners;
	// do the ai stuff:
	board = aiBoard::Ptr(new aiBoard(lines, linesSize, width, height, squareOwners, playerId, maxPlayerId));
	int line = mcts();
	
	if (line < 0 || line >= linesSize)
	{
		qDebug() << "mcts didn't return a correct line: " << line;
		qDebug() << "coosing random valid move";
		QList<int> freeLines = aiFunctions::getFreeLines(lines, linesSize);
		if (freeLines.empty())
		{
			qDebug() << "no valid lines left!";
			turnTime = moveTimer.elapsed();
			return 0;
		}
		turnTime = moveTimer.elapsed();
		return freeLines.at(qrand() % freeLines.size());
	}
	
	turnTime = moveTimer.elapsed();
	return line;
}

int aiMCTS::mcts()
{
	// init mcts
	mctsTimer.start();
	mctsRootNode = MCTSNode::Ptr(new MCTSNode());

	int mctsIterations = 0;
	// fill mcts tree
	while (!mctsTimer.hasExpired(mctsTimeout))
	{
		MCTSNode::Ptr node = selection(mctsRootNode);
		if (node.isNull()) // whole game tree was expored. TODO: really done with search?
			break;
		expansion(node);
		simulation(node);
		backpropagation(node);

		mctsIterations++;
	}

	qDebug() << "MCTS iterations: " << mctsIterations;
	
	// select most promising move
	int line = -1;
	long mostVisited = -1;
	for (int i = 0; i < mctsRootNode->children.size(); i++)
	{
		if (mctsRootNode->children[i]->visitedCnt > mostVisited)
		{
			line = mctsRootNode->children[i]->moveSequence[0];
			mostVisited = mctsRootNode->children[i]->visitedCnt;
		}
	}
	return line;
}

MCTSNode::Ptr aiMCTS::selection(MCTSNode::Ptr node)
{
	if (node->children.isEmpty()) // reached leaf of mcts tree
	{
		// compute (game) children of (mcts) leaf node
		QList<int> moveSeqToNode;
		MCTSNode::Ptr tmpNode = node;
		while (!tmpNode->parent.isNull())
		{
			for (int i = tmpNode->moveSequence.size() - 1; i >= 0; i--)
				moveSeqToNode.prepend(tmpNode->moveSequence[i]);
			tmpNode = tmpNode->parent;
		}
		for (int i = 0; i < moveSeqToNode.size(); i++)
		{
			board->doMove(moveSeqToNode[i]);
		}
		KSquares::BoardAnalysis analysis = BoardAnalysisFunctions::analyseBoard(board);
		for (int i = moveSeqToNode.size() - 1; i >= 0; i--)
		{
			board->undoMove(moveSeqToNode[i]);
		}
		for (int i = 0; i < analysis.moveSequences->size(); i++)
		{
			MCTSNode::Ptr newNode(new MCTSNode());
			newNode->moveSequence.append(analysis.moveSequences->at(i));
			newNode->parent = node;
			if (qrand() % 2)
				node->children.append(newNode);
			else
				node->children.prepend(newNode);
		}
	}
	
	if (node->children.isEmpty()) // reached leaf of game tree
	{
		return node;
	}
	
	// actual selection
	MCTSNode::Ptr selectedNode;
	double bestVal = -INFINITY;
	double C = 4;
	double lnnp = log(node->visitedCnt+1);
	for (int i = 0; i < node->children.size(); i++)
	{
		double uctVal = node->children[i]->value + C * sqrt(lnnp / (double)(node->children[i]->visitedCnt+1));
		if (uctVal > bestVal)
		{
			bestVal = uctVal;
			selectedNode = node->children[i];
		}
	}
	
	if (selectedNode.isNull())
	{
		qDebug() << "selected node is null, no child has been selected?!";
		return selectedNode;
	}
	
	if (selectedNode->inTree)
	{
		return selection(selectedNode);
	}
	
	return selectedNode;
}

void aiMCTS::expansion(MCTSNode::Ptr node)
{
	node->inTree = true;
}

void aiMCTS::simulation(MCTSNode::Ptr node)
{

	QVector<int> simulationHistory;
	int missingLines = 0;
	QList<bool> linesList;
	for (int i = 0; i < board->linesSize; i++) {
		if (!board->lines[i]) {
			missingLines++;
			linesList.append(false);
		} else {
			linesList.append(true);
		}
	}
	simulationHistory.reserve(missingLines);
	// do simulation
	while (missingLines > 0)
	{
		int line = simAi->chooseLine(linesList, QList<int>(), QList<Board::Move_t>());
		board->doMove(line);
		linesList[line] = true;
		simulationHistory.append(line);
		missingLines--;
	}
	// get simulation result
	for (int i = 0; i < board->squareOwners.size(); i++)
	{
		node->fullValue += board->squareOwners[i] == playerId ? 1 : -1;
	}
	node->value = node->fullValue;
	// undo simulation
	for (int i = simulationHistory.size() -1; i >= 0; i--)
	{
		board->undoMove(simulationHistory[i]);
	}
}

void aiMCTS::backpropagation(MCTSNode::Ptr node)
{
	MCTSNode::Ptr tmpNode = node;
	int addValue = node->value;
	while (!tmpNode->parent.isNull())
	{
		tmpNode->visitedCnt++;
		tmpNode->fullValue += addValue;
		tmpNode->value = (double)tmpNode->fullValue / (double)tmpNode->visitedCnt;
		tmpNode = tmpNode->parent;
	}
}

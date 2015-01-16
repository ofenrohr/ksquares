/***************************************************************************
 *   Copyright (C) 2014 by Tom Vincent Peters   <kde@vincent-peters.de>    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "aiAlphaBeta.h"

#include <limits>
#include <KDebug>
#include <cmath>
#include <algorithm>
#include <QElapsedTimer>
#include <QMap>
#include <QPair>

aiAlphaBeta::aiAlphaBeta(int newPlayerId, int newMaxPlayerId, int newWidth, int newHeight, int newLevel) : KSquaresAi(newWidth, newHeight), playerId(newPlayerId), maxPlayerId(newMaxPlayerId), level(newLevel)
{
	width = newWidth;
	height = newHeight;
	linesSize = toLinesSize(width, height);
	lines = new bool[linesSize];
	debug = false;
	maxEvalTime = 0;
	alphabetaTimeout = 5000; // 5 sec timeout
	heuristic = new aiHeuristic(true, true, true);
}

aiAlphaBeta::~aiAlphaBeta()
{
	delete[] lines;
	delete heuristic;
}

int aiAlphaBeta::chooseLine(const QList<bool> &newLines, const QList<int> &newSquareOwners)
{
	if (newLines.size() != linesSize)
	{
		kFatal() << "something went terribly wrong: newLines.size() != linesSize";
	}
	// put lines into local board representation
	for (int i = 0; i < linesSize; ++i) lines[i] = newLines[i];
	// remember square owner table
	squareOwners = newSquareOwners;
	// do the ai stuff:
	kDebug() << "incoming board:" << boardToString(lines, linesSize, width, height);
	
	// do sth smart
	aiBoard::Ptr board = aiBoard::Ptr(new aiBoard(lines, linesSize, width, height, squareOwners, playerId, maxPlayerId));
	
	int line;
	alphabeta(board, 4, &line);
	
	if (line < 0 || line >= linesSize)
	{
		kDebug() << "alphabeta didn't return a correct line: " << line;
		kDebug() << "coosing random valid move";
		QList<int> freeLines = aiFunctions::getFreeLines(lines, linesSize);
		if (freeLines.size() <= 0)
		{
			kDebug() << "no valid lines left!";
			return 0;
		}
		return freeLines.at(qrand() % freeLines.size());
	}
	
	return line;
}

/*
 * Sources:
 * http://www.fierz.ch/strategy1.htm
*/

float aiAlphaBeta::alphabeta(aiBoard::Ptr board, int depth, int *line, int parentNode)
{
	if (line != NULL)
	{
		if (!alphabetaTimer.isValid())
		{
			kDebug() << "starting alphabeta timer";
			alphabetaTimer.start();
		}
		else
		{
			kDebug() << "restarting alphabeta timer";
			alphabetaTimer.restart();
		}
	}
	QList<QList<int> > moveSequences = getMoveSequences(board);
	
	int thisNode = debugNodeCnt;
	debugNodeCnt++;
	if (debug)
	{
		QString boardStr = aiFunctions::boardToString(board->lines, board->linesSize, board->width, board->height).trimmed();
		boardStr.replace(QString("\n"), QString("\\l"));
		//QString debugDot = "";
		debugDot.append("  n");
		debugDot.append(QString::number(thisNode));
		debugDot.append("[label=\"p:");
		debugDot.append(QString::number(board->playerId));
		debugDot.append("\\l");
		debugDot.append(boardStr);
		debugDot.append("\"];\n");
		if (parentNode != -1)
		{
			//kDebug() << debugDot;
			debugDot.append("  n");
			debugDot.append(QString::number(thisNode));
			debugDot.append(" -- n");
			debugDot.append(QString::number(parentNode));
			debugDot.append(";\n");
		}
	}
	
	if (moveSequences.size() == 0) // game is over
	{
		//kDebug() << "terminal node";
		int winner = aiFunctions::getLeader(board->squareOwners);
		if (winner == -2) // draw
			return 0;
		if (winner == playerId)
			return -INFINITY;
		else
			return INFINITY;
	}
	
	bool terminalNode = false;
	if (depth == 0) terminalNode = true;
	if (line == NULL && alphabetaTimer.hasExpired(alphabetaTimeout)) 
	{
		terminalNode = true;
		kDebug() << "alphabeta timeout reached, not going deeper";
	}
	if (terminalNode)
	{
		//kDebug() << "evaluating board:" << boardToString(board->lines, board->linesSize, board->width, board->height);
		int eval = evaluate(board);
		if (debug)
		{
			//kDebug() << "result: " << eval;
			debugDot.append("  e");
			debugDot.append(QString::number(thisNode));
			debugDot.append("[label=\"");
			debugDot.append(QString::number(eval));
			debugDot.append("\\n");
			debugDot.append(QString::number(lastEvalTime));
			debugDot.append(" ms");
			debugDot.append("\"];\n  e");
			debugDot.append(QString::number(thisNode));
			debugDot.append(" -- n");
			debugDot.append(QString::number(thisNode));
			debugDot.append(";\n");
		}
		return eval;
	}
	
	float bestValue = -INFINITY;
	for (int i = 0; i < moveSequences.size(); i++)
	{
		for (int j = 0; j < moveSequences[i].size(); j++)
		{
			board->doMove(moveSequences[i][j]);
		}
		float val = -alphabeta(board, depth - 1, NULL, thisNode);
		for (int j = 0; j < moveSequences[i].size(); j++)
		{
			board->undoMove(moveSequences[i][j]);
		}
		if (val > bestValue)
		{
			bestValue = val;
			if (line != NULL)
				*line = moveSequences[i][0];
		}
	}
	return bestValue;
}

float aiAlphaBeta::evaluate(aiBoard::Ptr board)
{
	QElapsedTimer evalTimer;
	evalTimer.start();
	float ret = heuristic->evaluate(board, playerId);
	long evalTime = evalTimer.elapsed();
	if (evalTime > maxEvalTime)
		evalTime = maxEvalTime;
	return ret;
}

// TODO: move to ai functions
QMap<int, int> getSquareValences(aiBoard::Ptr board)
{
	QMap<int, int> squareValences; // square, valence
	
	// find untaken squares and calculate valence
	QList<int> freeSquares;
	for (int i = 0; i < board->squareOwners.size(); i++)
	{
		squareValences[i] = aiFunctions::countBorderLines(board->width, board->height, i, board->lines);
	}
	
	return squareValences;
}

// TODO: move to ai functions
// Checks if given square is at at least one border of the board
bool isBorderSquare(aiBoard::Ptr board, int square)
{
	int squareLines[4];
	aiFunctions::linesFromSquare(board->width, board->height, squareLines, square);
	for (int i = 0; i < 4; i++)
	{
		if (aiFunctions::squaresFromLine(board->width, board->height, squareLines[i]).size() == 1)
			return true;
	}
	return false;
}

QList<int> aiAlphaBeta::getDoubleDealingSequence(KSquares::Chain &chain)
{
	QList<int> ret;
	
	if (chain.type != KSquares::CHAIN_LONG && chain.type != KSquares::CHAIN_LOOP)
	{
		return ret;
	}
	
	ret.append(chain.lines);
	
	if (chain.type == KSquares::CHAIN_LONG)
	{
		ret.removeAt(ret.size() - 2);
	}
	
	if (chain.type == KSquares::CHAIN_LOOP)
	{
		if (ret.size() < 3)
		{
			ret.clear();
			return ret;
		}
		ret.removeAt(ret.size() - 3);
		ret.removeAt(ret.size() - 1);
	}
	
	return ret;
}

QList<QList<int> > aiAlphaBeta::getMoveSequences(aiBoard::Ptr board)
{
	QList<QList<int> > moveSequences;
	
	QList<KSquares::Chain> chains;
	aiFunctions::findChains(board, &chains);
	
	QList<int> capturableLongChains;
	QList<int> capturableLoopChains;
	QList<int> capturableShortChains;
	
	for (int i = 0; i < chains.size(); i++)
	{
		switch (chains[i].type)
		{
			case KSquares::CHAIN_LONG:
				if (chains[i].ownChain)
					capturableLongChains.append(i);
			break;
			case KSquares::CHAIN_LOOP:
				if (chains[i].ownChain)
					capturableLoopChains.append(i);
			break;
			case KSquares::CHAIN_SHORT:
				if (chains[i].ownChain)
					capturableShortChains.append(i);
			break;
			case KSquares::CHAIN_UNKNOWN:
			default:
				kDebug() << "WARNING: unknown chain! " << chains[i].lines;
			break;
		}
	}
	
	// find out if double dealing is possible and remember the id of the chain in which double dealing shall happen
	int doubleDealingChainIndex = -1;
	// double dealing in long chain
	if (capturableLongChains.size() > 0)
	{
		doubleDealingChainIndex = capturableLongChains[capturableLongChains.size()-1];
	}
	else 
	{ // try double dealing in short and loop chains
		QList<int> capturableShortAndLoopChains;
		capturableShortAndLoopChains.append(capturableShortChains);
		// TODO: sort loop chains by number of squares in descending order!
		QList<int> sortedCapturableLoopChains;
		sortedCapturableLoopChains.append(capturableLoopChains);
		capturableShortAndLoopChains.append(sortedCapturableLoopChains);
		
		for (int i = 0; i < capturableShortAndLoopChains.size(); i++)
		{
			QList<int> doubleDealingVariant = getDoubleDealingSequence(chains[capturableShortAndLoopChains[i]]);
			if (doubleDealingVariant.size() > 0)
			{
				doubleDealingChainIndex = capturableShortAndLoopChains[i];
				break;
			}
		}
	}
	
	// generate the basic move sequence - excluding the chain where double dealing will happen
	QList<int> baseMoveSequence;
	for (int i = 0; i < capturableShortChains.size(); i++)
	{
		if (doubleDealingChainIndex == capturableShortChains[i])
			continue;
		baseMoveSequence.append(chains[capturableShortChains[i]].lines);
	}
	for (int i = 0; i < capturableLoopChains.size(); i++)
	{
		if (doubleDealingChainIndex == capturableLoopChains[i])
			continue;
		baseMoveSequence.append(chains[capturableLoopChains[i]].lines);
	}
	for (int i = 0; i < capturableLongChains.size(); i++)
	{
		if (doubleDealingChainIndex == capturableLongChains[i])
			continue;
		baseMoveSequence.append(chains[capturableLongChains[i]].lines);
	}
	
	// add double dealing version
	if (doubleDealingChainIndex != -1)
	{
		QList<int> doubleDealingSequence;
		doubleDealingSequence.append(baseMoveSequence);
		doubleDealingSequence.append(getDoubleDealingSequence(chains[doubleDealingChainIndex]));
		moveSequences.append(doubleDealingSequence);
		
		// add normal variant to basic move sequence
		baseMoveSequence.append(chains[doubleDealingChainIndex].lines);
	}
	
	/* ============================================== */
	/* == board changes from here on               == */
	/* ============================================== */
	
	// now we need to create a board with all capturable chains filled in to
	// get the newly available chains (filling out chains can connect separate chains)
	for (int i = 0; i < baseMoveSequence.size(); i++)
	{
		board->doMove(baseMoveSequence[i]);
	}
	
	// look for chains a second time
	QList<KSquares::Chain> otherChains;
	aiFunctions::findChains(board, &otherChains);
	
	QList<int> openLongChains;
	QList<int> openLoopChains;
	QList<int> openShortChains;
	
	for (int i = 0; i < otherChains.size(); i++)
	{
		switch (otherChains[i].type)
		{
			case KSquares::CHAIN_LONG:
				if (!otherChains[i].ownChain)
					openLongChains.append(i);
				else
					kDebug() << "ERROR: capturable chain found when there should be none!";
			break;
			case KSquares::CHAIN_LOOP:
				if (!otherChains[i].ownChain)
					openLoopChains.append(i);
				else
					kDebug() << "ERROR: capturable chain found when there should be none!";
			break;
			case KSquares::CHAIN_SHORT:
				if (!otherChains[i].ownChain)
					openShortChains.append(i);
				else
					kDebug() << "ERROR: capturable chain found when there should be none!";
			break;
			case KSquares::CHAIN_UNKNOWN:
			default:
				kDebug() << "WARNING: unknown chain! " << otherChains[i].lines;
			break;
		}
	}
	
	// add full capture version
	// move sequences for open chains + free lines
	// get free lines and filter them
	QList<int> freeLines; // = aiFunctions::getFreeLines(board->lines, board->linesSize);
	for (int i = 0; i < board->linesSize; i++)
		if (!board->lines[i] && !baseMoveSequence.contains(i))
			freeLines.append(i);
	//kDebug() << "free lines: " << freeLines;
	if (freeLines.size() == 0)
		moveSequences.append(baseMoveSequence);
	// add one sequence for each long and loop chain
	QList<int> openLongAndLoopChains;
	openLongAndLoopChains.append(openLongChains);
	openLongAndLoopChains.append(openLoopChains);
	for (int i = 0; i < openLongAndLoopChains.size(); i++)
	{
		QList<int> moveSequence;
		moveSequence.append(baseMoveSequence);
		moveSequence.append(otherChains[openLongAndLoopChains[i]].lines[0]);
		for (int j = 0; j < otherChains[openLongAndLoopChains[i]].lines.size(); j++)
		{
			freeLines.removeAll(otherChains[openLongAndLoopChains[i]].lines[j]);
		}
		moveSequences.append(moveSequence);
	}
	// add half and hard hearted handouts for short chains
	for (int i = 0; i < openShortChains.size(); i++)
	{
		if (otherChains[openShortChains[i]].squares.size() != 2)
			continue;
		QList<int> halfHeartedSequence;
		halfHeartedSequence.append(baseMoveSequence);
		halfHeartedSequence.append(otherChains[openShortChains[i]].lines[0]);
		moveSequences.append(halfHeartedSequence);
		QList<int> hardHeartedSequence;
		hardHeartedSequence.append(baseMoveSequence);
		hardHeartedSequence.append(otherChains[openShortChains[i]].lines[1]);
		moveSequences.append(hardHeartedSequence);
		for (int j = 0; j < otherChains[openShortChains[i]].lines.size(); j++)
		{
			freeLines.removeAll(otherChains[openShortChains[i]].lines[j]);
		}
	}
	// add all that's left
	for (int i = 0; i < freeLines.size(); i++)
	{
		QList<int> moveSequence;
		moveSequence.append(baseMoveSequence);
		moveSequence.append(freeLines[i]);
		moveSequences.append(moveSequence);
	}
	
	// undo the filled out chains
	for (int i = baseMoveSequence.size() - 1; i >= 0; i--)
	{
		board->undoMove(baseMoveSequence[i]);
	}
	
	return moveSequences;
}

QList<QList<int> > getMoveSequencesOld(aiBoard::Ptr board)
{
	QList<int> freeLines = aiFunctions::getFreeLines(board->lines, board->linesSize);
	QMap<int, int> squareValence;
	QList<QPair<int, int> > chainEnds; // line, square
	while (freeLines.size() > 0)
	{
		int line = freeLines.takeLast();
		QList<int> lineSquares = aiFunctions::squaresFromLine(board->width, board->height, line);
		for (int i = 0; i < lineSquares.size(); i++)
		{
			if (squareValence.contains(lineSquares[i]))
				continue;
			squareValence[lineSquares[i]] = aiFunctions::countBorderLines(board->width, board->height, lineSquares[i], board->lines);
			
			if (squareValence[lineSquares[i]] == 3) // square can be captured
			{
				QPair<int, int> chainEnd(line, lineSquares[i]);
				chainEnds.append(chainEnd);
			}
		}
	}
	
	kDebug() << "chainEnds: " << chainEnds;
	
	QList<QList<int> > ret;
	for (int i = 0; i < chainEnds.size(); i++)
	{
		QList<int> lineSequence;
		//lineSequence.append(chainEnds[i].first);
		QPair<int, int> elem = chainEnds[i];
		bool seqDone = false;
		while (!seqDone)
		{
			seqDone = true;
			int line = elem.first;
			int square = elem.second;
			QList<int> lineSquares = aiFunctions::squaresFromLine(board->width, board->height, line);
			for (int j = 0; j < lineSquares.size(); j++)
			{
				if (lineSquares[j] == square)
					continue;
				int squareLines[4];
				aiFunctions::linesFromSquare(board->width, board->height, squareLines, lineSquares[j]);
				for (int k = 0; k < 4; k++)
				{
					if (board->lines[squareLines[k]] || squareLines[k] == line)
						continue;
					if (squareValence[lineSquares[j]] == 2) // chain continues
					{
						seqDone = false;
					}
					elem.first = squareLines[k];
					elem.second = lineSquares[j];
					lineSequence.append(line);
				}
			}
		}
		ret.append(lineSequence);
	}
	return ret;
}

void aiAlphaBeta::setTimeout(long timeout)
{
	alphabetaTimeout = timeout;
}

long aiAlphaBeta::getTimeout()
{
	return alphabetaTimeout;
}

void aiAlphaBeta::setDebug(bool val)
{
	debug = val;
	debugDot = "";
	debugNodeCnt = 0;
}

QString aiAlphaBeta::getDebugDot()
{
	return debugDot;
}

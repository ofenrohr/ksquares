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
	heuristic = new aiHeuristic(false, true, true);
	searchDepth = 30;
	debugDepth = searchDepth;
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
	
	kDebug() << "alphabeta START";
	int line = -1;
	float evalResult = alphabeta(board, searchDepth, &line);
	kDebug() << "alphabeta END " << line;
	
	kDebug() << "alphabeta eval result = " << evalResult;
	
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

float aiAlphaBeta::alphabeta(aiBoard::Ptr board, int depth, int *line, float alpha, float beta, int parentNode)
{
	if (line != NULL)
	{
		if (!alphabetaTimer.isValid())
		{
			//kDebug() << "starting alphabeta timer";
			alphabetaTimer.start();
		}
		else
		{
			//kDebug() << "restarting alphabeta timer";
			alphabetaTimer.restart();
		}
	}
	KSquares::BoardAnalysis analysis = aiFunctions::analyseBoard(board);
	QList<QList<int> > moveSequences = getMoveSequences(board, analysis);
	
	int thisNode = debugNodeCnt;
	debugNodeCnt++;
	QString debugLabel = "";
	if (debug && searchDepth - depth <= debugDepth)
	{
		QString boardStr = aiFunctions::boardToString(board->lines, board->linesSize, board->width, board->height).trimmed();
		boardStr.replace(QString("\n"), QString("\\l"));
		//boardStr.replace(QString(" "), QString(" ")); // space with non-breaking space
		/*
		debugDot.append("  n");
		debugDot.append(QString::number(thisNode));
		debugDot.append("[shape=box, label=\"p:");
		*/
		debugLabel.append("p:"+QString::number(board->playerId));
		debugLabel.append("\\l");
		debugLabel.append(boardStr);
		/*
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
		*/
	}
	
	if (moveSequences.size() == 0) // game is over
	{
		//kDebug() << "terminal node - board filled";
		// TODO: remove check
		if (board->squareOwners.contains(-1))
			kDebug() << "full board contains square without owner!";
		int result = 0;
		for (int i = 0; i < board->squareOwners.size(); i++)
		{
			if (board->squareOwners[i] < 0)
				kDebug() << "ERROR: no move sequences available, but untaken squares on board!!";
			
			result += board->squareOwners[i] == board->playerId ? 1 : -1;
		}
		if (debug && searchDepth - depth <= debugDepth)
		{
			QMap<int, int> scoreMap = aiFunctions::getScoreMap(board->squareOwners);
			QString resultStr = "\\n";
			for (int i = 0; i < scoreMap.keys().size(); i++)
			{
				resultStr += "p"; 
				resultStr += QString::number(scoreMap.keys()[i]);
				resultStr += ":";
				resultStr += QString::number(scoreMap[scoreMap.keys()[i]]);
				resultStr += "\\l";
			}
			debugDot.append("  n");
			debugDot.append(QString::number(thisNode));
			debugDot.append("[shape=box, label=\"end: ");
			debugDot.append(QString::number(result));
			debugDot.append(resultStr);
			debugDot.append(debugLabel);
			debugDot.append("\"];\n ");
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
		// the player didn't change: return negative result!
		return -result;
		
		/*
		int winner = aiFunctions::getLeader(board->squareOwners);
		if (winner < 0) // draw
			return 0;
		if (winner == board->playerId)
			return INFINITY;
		else
			return -INFINITY;
		*/
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
		heuristic->setAnalysis(analysis);
		float eval = evaluate(board);
		if (debug && searchDepth - depth <= debugDepth)
		{
			//kDebug() << "result: " << eval;
			debugDot.append("  n");
			debugDot.append(QString::number(thisNode));
			debugDot.append("[shape=circle, label=\"");
			debugDot.append(QString::number(eval));
			debugDot.append("\\nsc:");
			debugDot.append(QString::number(analysis.openShortChains.size()));
			debugDot.append("\\nlc:");
			debugDot.append(QString::number(analysis.openLongChains.size()));
			debugDot.append("\\ncc:");
			debugDot.append(QString::number(analysis.openLoopChains.size()));
			debugDot.append("\\ncsc:");
			debugDot.append(QString::number(analysis.capturableShortChains.size()));
			debugDot.append("\\nclc:");
			debugDot.append(QString::number(analysis.capturableLongChains.size()));
			debugDot.append("\\nccc:");
			debugDot.append(QString::number(analysis.capturableLoopChains.size()));
			debugDot.append("\\n");
			debugDot.append(debugLabel);
			//debugDot.append("\\n");
			//debugDot.append(QString::number(lastEvalTime));
			//debugDot.append(" ms");
			debugDot.append("\"];\n ");
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
		return eval;
	}
	
	//int localLine = -1;
	float bestValue = -INFINITY;
	for (int i = 0; i < moveSequences.size(); i++)
	{
		//if (moveSequences[i].size() == 0)
		//	kDebug() << "empty move sequence!";
		for (int j = 0; j < moveSequences[i].size(); j++)
		{
			board->doMove(moveSequences[i][j]);
		}
		float val = -alphabeta(board, depth - 1, NULL, -beta, -alpha, thisNode);
		for (int j = moveSequences[i].size() -1; j >= 0; j--)
		{
			board->undoMove(moveSequences[i][j]);
		}
		if (val > bestValue)
		{
			bestValue = val;
			if (line != NULL)
				*line = moveSequences[i][0];
		}
		/*
		if (val > alpha)
			alpha = val;
		if (alpha >= beta)
		{
			//kDebug() << "pruned";
			break;
		}
		*/
	}
	//kDebug() << localLine << " ";
	//if (bestValue == -INFINITY && moveSequences.size() > 0 && line != NULL)
	//	*line = moveSequences[0][0];
	
	if (debug && searchDepth - depth <= debugDepth)
	{
		debugDot.append("  n");
		debugDot.append(QString::number(thisNode));
		debugDot.append("[shape=box, label=\"bv:");
		debugDot.append(QString::number(bestValue));
		debugDot.append(debugLabel);
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
	
	return bestValue;
}

float aiAlphaBeta::evaluate(aiBoard::Ptr board)
{
	QElapsedTimer evalTimer;
	evalTimer.start();
	float ret = heuristic->evaluate(board);
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

QList<int> aiAlphaBeta::ignoreCornerLines(aiBoard::Ptr board)
{
	QList<int> ignoreLines;
	int squareLines[4]; // top, left, right, bottom
	
	aiFunctions::linesFromSquare(board->width, board->height, squareLines, 0);
	if (!board->lines[squareLines[0]] && !board->lines[squareLines[1]])
		ignoreLines.append(squareLines[0]);
	
	aiFunctions::linesFromSquare(board->width, board->height, squareLines, board->width-1);
	if (!board->lines[squareLines[0]] && !board->lines[squareLines[2]])
		ignoreLines.append(squareLines[0]);
	
	aiFunctions::linesFromSquare(board->width, board->height, squareLines, board->width*board->height-board->width);
	if (!board->lines[squareLines[1]] && !board->lines[squareLines[3]])
		ignoreLines.append(squareLines[1]);
	
	aiFunctions::linesFromSquare(board->width, board->height, squareLines, board->width*board->height-1);
	if (!board->lines[squareLines[2]] && !board->lines[squareLines[3]])
		ignoreLines.append(squareLines[3]);
	
	return ignoreLines;
}

QList<QList<int> > aiAlphaBeta::getMoveSequences(aiBoard::Ptr board, KSquares::BoardAnalysis &analysis)
{
	QList<QList<int> > moveSequences;
	QList<QList<int> > chainSacrificeSequences;
	
	// TODO: share with heuristic
	//KSquares::BoardAnalysis analysis = aiFunctions::analyseBoard(board);
	//board->analyseBoard();
	
	// find out if double dealing is possible and remember the id of the chain in which double dealing shall happen
	int doubleDealingChainIndex = -1;
	// double dealing in long chain
	if (analysis.capturableLongChains.size() > 0)
	{
		doubleDealingChainIndex = analysis.capturableLongChains[analysis.capturableLongChains.size()-1];
	}
	else 
	{ // try double dealing in short and loop chains
		QList<int> capturableShortAndLoopChains;
		capturableShortAndLoopChains.append(analysis.capturableShortChains);
		// TODO: sort loop chains by number of squares in descending order!
		// workaround: put the one with most squares at beginning
		QList<int> sortedCapturableLoopChains;
		for (int i = 0; i < analysis.capturableLoopChains.size(); i++)
		{
			if (sortedCapturableLoopChains.size() > 0)
				if (analysis.chains[sortedCapturableLoopChains[0]].squares.size() < analysis.chains[analysis.capturableLoopChains[i]].squares.size())
					sortedCapturableLoopChains.prepend(analysis.capturableLoopChains[i]);
				else
					sortedCapturableLoopChains.append(analysis.capturableLoopChains[i]);
			else
				sortedCapturableLoopChains.append(analysis.capturableLoopChains[i]);
		}
		sortedCapturableLoopChains.append(analysis.capturableLoopChains);
		capturableShortAndLoopChains.append(sortedCapturableLoopChains);
		
		for (int i = 0; i < capturableShortAndLoopChains.size(); i++)
		{
			QList<int> doubleDealingVariant = getDoubleDealingSequence(analysis.chains[capturableShortAndLoopChains[i]]);
			if (doubleDealingVariant.size() > 0)
			{
				doubleDealingChainIndex = capturableShortAndLoopChains[i];
				break;
			}
		}
	}
	
	// generate the basic move sequence - excluding the chain where double dealing will happen
	QList<int> baseMoveSequence;
	for (int i = 0; i < analysis.capturableShortChains.size(); i++)
	{
		if (doubleDealingChainIndex == analysis.capturableShortChains[i])
			continue;
		baseMoveSequence.append(analysis.chains[analysis.capturableShortChains[i]].lines);
	}
	for (int i = 0; i < analysis.capturableLoopChains.size(); i++)
	{
		if (doubleDealingChainIndex == analysis.capturableLoopChains[i])
			continue;
		baseMoveSequence.append(analysis.chains[analysis.capturableLoopChains[i]].lines);
	}
	for (int i = 0; i < analysis.capturableLongChains.size(); i++)
	{
		if (doubleDealingChainIndex == analysis.capturableLongChains[i])
			continue;
		baseMoveSequence.append(analysis.chains[analysis.capturableLongChains[i]].lines);
	}
	
	// add double dealing version
	if (doubleDealingChainIndex != -1)
	{
		// only double deal if there is sth left
		bool finalCapture = true;
		for (int i = 0; i < board->linesSize; i++)
		{
			if (!board->lines[i] && !baseMoveSequence.contains(i) && !analysis.chains[doubleDealingChainIndex].lines.contains(i))
			{
				finalCapture = false;
				break;
			}
		}
		if (!finalCapture)
		{
			QList<int> doubleDealingSequence;
			doubleDealingSequence.append(baseMoveSequence);
			doubleDealingSequence.append(getDoubleDealingSequence(analysis.chains[doubleDealingChainIndex]));
			moveSequences.append(doubleDealingSequence);
		}
		
		// add normal variant to basic move sequence
		baseMoveSequence.append(analysis.chains[doubleDealingChainIndex].lines);
	}
	
	// add full capture version
	// move sequences for open chains + free lines
	// get free lines and filter them
	QList<int> freeLines; // = aiFunctions::getFreeLines(board->lines, board->linesSize);
	QList<int> ignoreLines = ignoreCornerLines(board);
	//QList<int> ignoreLines;
	for (int i = 0; i < board->linesSize; i++)
		if (!board->lines[i] && !baseMoveSequence.contains(i) && !ignoreLines.contains(i))
			freeLines.append(i);
	//kDebug() << "free lines: " << freeLines;
	if (freeLines.size() == 0 && baseMoveSequence.size() > 0)
		moveSequences.append(baseMoveSequence);
	// add one sequence for each long and loop chain
	QList<int> openLongAndLoopChains;
	openLongAndLoopChains.append(analysis.openLongChains);
	openLongAndLoopChains.append(analysis.openLoopChains);
	for (int i = 0; i < openLongAndLoopChains.size(); i++)
	{
		QList<int> moveSequence;
		moveSequence.append(baseMoveSequence);
		moveSequence.append(analysis.chainsAfterCapture[openLongAndLoopChains[i]].lines[0]);
		for (int j = 0; j < analysis.chainsAfterCapture[openLongAndLoopChains[i]].lines.size(); j++)
		{
			freeLines.removeAll(analysis.chainsAfterCapture[openLongAndLoopChains[i]].lines[j]);
		}
		//moveSequences.append(moveSequence);
		chainSacrificeSequences.append(moveSequence);
	}
	// add half and hard hearted handouts for short chains
	for (int i = 0; i < analysis.openShortChains.size(); i++)
	{
		if (analysis.chainsAfterCapture[analysis.openShortChains[i]].squares.size() != 2)
			continue;
		QList<int> halfHeartedSequence;
		halfHeartedSequence.append(baseMoveSequence);
		halfHeartedSequence.append(analysis.chainsAfterCapture[analysis.openShortChains[i]].lines[0]);
		//moveSequences.append(halfHeartedSequence);
		chainSacrificeSequences.prepend(halfHeartedSequence);
		QList<int> hardHeartedSequence;
		hardHeartedSequence.append(baseMoveSequence);
		hardHeartedSequence.append(analysis.chainsAfterCapture[analysis.openShortChains[i]].lines[1]);
		//moveSequences.append(hardHeartedSequence);
		chainSacrificeSequences.prepend(hardHeartedSequence);
		for (int j = 0; j < analysis.chainsAfterCapture[analysis.openShortChains[i]].lines.size(); j++)
		{
			freeLines.removeAll(analysis.chainsAfterCapture[analysis.openShortChains[i]].lines[j]);
		}
	}
	for (int i = 0; i < analysis.specialLines.size(); i++)
		freeLines.append(analysis.specialLines[i]);
	// add all that's left
	for (int i = 0; i < freeLines.size(); i++)
	{
		QList<int> moveSequence;
		moveSequence.append(baseMoveSequence);
		moveSequence.append(freeLines[i]);
		moveSequences.append(moveSequence);
	}
	// add chain sacrifice moves
	moveSequences.append(chainSacrificeSequences);
	
	return moveSequences;
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

void aiAlphaBeta::setDebugDepth(int d)
{
	debugDepth = d;
}

QString aiAlphaBeta::getDebugDot()
{
	return debugDot;
}

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

#include "lineSorter.h"
#include "boardAnalysis.h"

aiAlphaBeta::aiAlphaBeta(int newPlayerId, int newMaxPlayerId, int newWidth, int newHeight, int newLevel, int thinkTime) : KSquaresAi(newWidth, newHeight), playerId(newPlayerId), maxPlayerId(newMaxPlayerId), level(newLevel), alphabetaTimeout(thinkTime)
{
	width = newWidth;
	height = newHeight;
	linesSize = toLinesSize(width, height);
	lines = new bool[linesSize];
	debug = false;
	maxEvalTime = 0;
	//alphabetaTimeout = 5000; // 5 sec timeout
	heuristic = new aiHeuristic(true, true, true);
	searchDepth = 300000; // only used for debugging, search is limited by time
	debugDepth = searchDepth;
	LineSorter sorter(width, height, linesSize);
	lineSortList = sorter.getSortMap();
	analysisHash = new QHash<aiBoard::Ptr, QPair<bool *, KSquares::BoardAnalysis> >();
	alphabetaTimer = QElapsedTimer();
	turn = 0;
}

aiAlphaBeta::~aiAlphaBeta()
{
	delete analysisHash;
	delete[] lines;
	delete heuristic;
}

int aiAlphaBeta::chooseLine(const QList<bool> &newLines, const QList<int> &newSquareOwners, const QList<Board::Move> &lineHistory)
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
	timerHasExpiredBefore = false;
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
	int line = -1;
	//float evalResult = alphabetaIterativeDeepening(board, searchDepth, &line);
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

float aiAlphaBeta::alphabetaIterativeDeepening(aiBoard::Ptr board, int depth, int *line)
{
	float lastResult = 0;
	for (int i = 1; i < depth && !alphabetaTimer.hasExpired(alphabetaTimeout); i++)
	{
		lastResult = alphabeta(board, i, line);
	}
	return lastResult;
}

/*
 * Sources:
 * http://www.fierz.ch/strategy1.htm
*/

float aiAlphaBeta::alphabeta(aiBoard::Ptr board, int depth, int *line, float alpha, float beta/*, int parentNode*/)
{
//	bool isEndgame = false;
	//KSquares::BoardAnalysis analysis = aiFunctions::analyseBoard(board);
	KSquares::BoardAnalysis analysis = getAnalysis(board);
	QSharedPointer<QList<QList<int> > > moveSequences = analysis.moveSequences;//getMoveSequences(board, analysis, &isEndgame);
	
// 	int thisNode = debugNodeCnt;
// 	debugNodeCnt++;
// 	QString debugLabel = "";
// 	if (debug && searchDepth - depth <= debugDepth)
// 	{
// 		QString boardStr = aiFunctions::boardToString(board->lines, board->linesSize, board->width, board->height).trimmed();
// 		boardStr.replace(QString("\n"), QString("\\l"));
// 		//boardStr.replace(QString(" "), QString(" ")); // space with non-breaking space
// 		/*
// 		debugDot.append("  n");
// 		debugDot.append(QString::number(thisNode));
// 		debugDot.append("[shape=box, label=\"p:");
// 		*/
// 		debugLabel.append("p:"+QString::number(board->playerId));
// 		debugLabel.append("\\l");
// 		debugLabel.append(boardStr);
// 		/*
// 		debugDot.append("\"];\n");
// 		if (parentNode != -1)
// 		{
// 			//kDebug() << debugDot;
// 			debugDot.append("  n");
// 			debugDot.append(QString::number(thisNode));
// 			debugDot.append(" -- n");
// 			debugDot.append(QString::number(parentNode));
// 			debugDot.append(";\n");
// 		}
// 		*/
// 	}
	
	if (moveSequences->size() == 0) // game is over
	{
		//kDebug() << "terminal node - board filled";
		// TODO: remove check
//		if (board->squareOwners.contains(-1))
//			kDebug() << "full board contains square without owner!";
		int result = 0;
		for (int i = 0; i < board->squareOwners.size(); i++)
		{
// 			if (board->squareOwners[i] < 0)
// 				kDebug() << "ERROR: no move sequences available, but untaken squares on board!!";
			
			result += board->squareOwners[i] == board->playerId ? 1 : -1;
		}
// 		if (debug && searchDepth - depth <= debugDepth && !debugEvalOnly)
// 		{
// 			QMap<int, int> scoreMap = aiFunctions::getScoreMap(board->squareOwners);
// 			QString resultStr = "\\n";
// 			for (int i = 0; i < scoreMap.keys().size(); i++)
// 			{
// 				resultStr += "p"; 
// 				resultStr += QString::number(scoreMap.keys()[i]);
// 				resultStr += ":";
// 				resultStr += QString::number(scoreMap[scoreMap.keys()[i]]);
// 				resultStr += "\\l";
// 			}
// 			debugDot.append("  n");
// 			debugDot.append(QString::number(thisNode));
// 			debugDot.append("[shape=box, label=\"end: ");
// 			debugDot.append(QString::number(result));
// 			debugDot.append(resultStr);
// 			debugDot.append(debugLabel);
// 			debugDot.append("\"];\n ");
// 			if (parentNode != -1)
// 			{
// 				//kDebug() << debugDot;
// 				debugDot.append("  n");
// 				debugDot.append(QString::number(thisNode));
// 				debugDot.append(" -- n");
// 				debugDot.append(QString::number(parentNode));
// 				debugDot.append(";\n");
// 			}
// 		}
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
		//kDebug() << "alphabeta timeout reached, not going deeper";
	}
	if (terminalNode) //|| isEndgame)
	{
		//kDebug() << "evaluating board:" << boardToString(board->lines, board->linesSize, board->width, board->height);
		heuristic->setAnalysis(analysis);
		float eval = evaluate(board);
// 		if (!timerHasExpiredBefore)
// 		{
// 			timerHasExpiredBefore = true;
// 			kDebug() << "analysis for first timer expiration: " << analysis;
// 			kDebug() << "eval for first timer expiration: " << eval;
// 		}
// 		if (debug && searchDepth - depth <= debugDepth && (!debugEvalOnly || (debugEvalOnly && depth == searchDepth - 1)))
// 		{
// 			//kDebug() << "result: " << eval;
// 			debugDot.append("  n");
// 			debugDot.append(QString::number(thisNode));
// 			debugDot.append("[shape=circle, label=\"");
// 			debugDot.append(QString::number(eval));
// 			debugDot.append("\\nsc:");
// 			debugDot.append(QString::number(analysis.openShortChains.size()));
// 			debugDot.append("\\nlc:");
// 			debugDot.append(QString::number(analysis.openLongChains.size()));
// 			debugDot.append("\\ncc:");
// 			debugDot.append(QString::number(analysis.openLoopChains.size()));
// 			debugDot.append("\\ncsc:");
// 			debugDot.append(QString::number(analysis.capturableShortChains.size()));
// 			debugDot.append("\\nclc:");
// 			debugDot.append(QString::number(analysis.capturableLongChains.size()));
// 			debugDot.append("\\nccc:");
// 			debugDot.append(QString::number(analysis.capturableLoopChains.size()));
// 			debugDot.append("\\n");
// 			if (!debugEvalOnly)
// 				debugDot.append(debugLabel);
// 			//debugDot.append("\\n");
// 			//debugDot.append(QString::number(lastEvalTime));
// 			//debugDot.append(" ms");
// 			debugDot.append("\"];\n ");
// 			if (parentNode != -1)
// 			{
// 				//kDebug() << debugDot;
// 				debugDot.append("  n");
// 				debugDot.append(QString::number(thisNode));
// 				debugDot.append(" -- n");
// 				debugDot.append(QString::number(parentNode));
// 				debugDot.append(";\n");
// 			}
// 		}
		return eval;
	}
	
	//int localLine = -1;
	float bestValue = -INFINITY;
	for (int i = 0; i < moveSequences->size() && (!alphabetaTimer.hasExpired(alphabetaTimeout) || line != NULL); i++)
	{
		//if ((*moveSequences)[i].size() == 0)
		//	kDebug() << "empty move sequence!";
		int prevPlayer = board->playerId;
		for (int j = 0; j < (*moveSequences)[i].size(); j++)
		{
			board->doMove((*moveSequences)[i][j]);
		}
		// TODO: remove this check
		if (prevPlayer == board->playerId && board->squareOwners.contains(-1))
		{
			kDebug() << "ERROR: sth went really wrong! player didn't change after move sequence: " << (*moveSequences)[i];
			kDebug() << "ERROR: board: " << aiFunctions::boardToString(board);
		}
		float val = -alphabeta(board, depth - 1, NULL, -beta, -alpha/*, thisNode*/);
		for (int j = (*moveSequences)[i].size() -1; j >= 0; j--)
		{
			board->undoMove((*moveSequences)[i][j]);
		}
		/*
		if (val == bestValue && line != NULL) // randomly select other result if it is as good as the best one found
		{
			linePool.append((*moveSequences)[i][0]);
			int poolIndex = ((float) rand()/(RAND_MAX + 1.0)) * (linePool.size()-1);
			*line = linePool[poolIndex];
		}
		*/
		if (val > bestValue)
		{
			bestValue = val;
			if (line != NULL)
			{
				//linePool.clear();
				//linePool.append((*moveSequences)[i][0]);
				*line = (*moveSequences)[i][0];
			}
			// put the current item in front (reordering for iterative deepening)
			moveSequences->prepend((*moveSequences)[i]);
			moveSequences->removeAt(i+1);
		}
		
		if (val > alpha)
			alpha = val;
		if (alpha >= beta)
		{
			if (line != NULL)
				kDebug() << "pruned at " << i;
			break;
		}
		
	}
	//kDebug() << localLine << " ";
	//if (bestValue == -INFINITY && moveSequences->size() > 0 && line != NULL)
	//	*line = (*moveSequences)[0][0];
	
// 	if (debug && searchDepth - depth <= debugDepth && !debugEvalOnly)
// 	{
// 		debugDot.append("  n");
// 		debugDot.append(QString::number(thisNode));
// 		debugDot.append("[shape=box, label=\"bv:");
// 		debugDot.append(QString::number(bestValue));
// 		debugDot.append(debugLabel);
// 		debugDot.append("\"];\n");
// 		if (parentNode != -1)
// 		{
// 			//kDebug() << debugDot;
// 			debugDot.append("  n");
// 			debugDot.append(QString::number(thisNode));
// 			debugDot.append(" -- n");
// 			debugDot.append(QString::number(parentNode));
// 			debugDot.append(";\n");
// 		}
// 	}
	
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

KSquares::BoardAnalysis aiAlphaBeta::getAnalysis(aiBoard::Ptr board)
{
	if (analysisHash->contains(board))
	{
		QList<QPair<bool *, KSquares::BoardAnalysis> > entries = analysisHash->values(board);
		for (int j = 0; j < entries.size(); j++)
		{
			QPair<bool *, KSquares::BoardAnalysis> entry = entries[j];//analysisHash->value(board);
			bool sameBoard = true;
			for (int i = 0; i < board->linesSize && sameBoard; i++)
			{
				if (entry.first[i] != board->lines[i])
					sameBoard = false;
			}
			if (sameBoard)
			{
				//kDebug() << "returning analysis from hash!";
				return entry.second;
			}
		}
	}
	
	KSquares::BoardAnalysis analysis = BoardAnalysisFunctions::analyseBoard(board, lineSortList);
	bool *linesCopy = new bool[board->linesSize];
	memcpy(linesCopy, board->lines, board->linesSize);
	analysisHash->insertMulti(board, QPair<bool *, KSquares::BoardAnalysis>(linesCopy, analysis));
	return analysis;
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

void aiAlphaBeta::setDebugEvalOnly(bool e)
{
	debugEvalOnly = e;
}

QString aiAlphaBeta::getDebugDot()
{
	return debugDot;
}

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

#include "boardAnalysis.h"

aiAlphaBeta::aiAlphaBeta(int newPlayerId, int newMaxPlayerId, int newWidth, int newHeight, int newLevel, int thinkTime) : KSquaresAi(newWidth, newHeight), playerId(newPlayerId), maxPlayerId(newMaxPlayerId), level(newLevel), alphabetaTimeout(thinkTime)
{
	width = newWidth;
	height = newHeight;
	linesSize = toLinesSize(width, height);
	lines = new bool[linesSize];
	heuristic = new aiHeuristic(true, true, true);
	searchDepth = 300000; // only used for debugging, search is limited by time
	analysisHash = new QHash<aiBoard::Ptr, QPair<TranspositionEntry, TranspositionEntry> >();
	alphabetaTimer = QElapsedTimer();
	hashLines = new QList<int>();
}

aiAlphaBeta::~aiAlphaBeta()
{
	clearTranspositionTable();
	delete analysisHash;
	delete[] lines;
	delete heuristic;
	delete hashLines;
}

void aiAlphaBeta::clearTranspositionTable()
{
	QList<QPair<TranspositionEntry, TranspositionEntry> > hashValues = analysisHash->values();
	for (int i = 0; i < hashValues.size(); i++)
	{
		delete[] hashValues[i].first.first;
		delete[] hashValues[i].second.first;
	}
	
	analysisHash->clear();
}

int aiAlphaBeta::chooseLine(const QList<bool> &newLines, const QList<int> &newSquareOwners, const QList<Board::Move> &/*lineHistory*/)
{
	QElapsedTimer moveTimer;
	moveTimer.start();
	
	if (newLines.size() != linesSize)
	{
		kFatal() << "something went terribly wrong: newLines.size() != linesSize";
	}
	// put lines into local board representation
	hashLines->clear();
	for (int i = 0; i < linesSize; ++i)
	{
		lines[i] = newLines[i];
		if (!lines[i] && hashLines->size() <= 32)
			hashLines->append(i);
	}
	// remember square owner table
	squareOwners = newSquareOwners;
	// clean up
	clearTranspositionTable();
	// do the ai stuff:
	aiBoard::Ptr board = aiBoard::Ptr(new aiBoard(lines, linesSize, width, height, squareOwners, playerId, maxPlayerId, hashLines));
	alphabetaTimer.start();
	int line = -1;
#ifdef KSQUARES_ALPHABETA_ITERATIVE_DEEPENING
	alphabetaIterativeDeepening(board, searchDepth, &line);
#else
	alphabeta(board, searchDepth, &line);
#endif
	
	if (line < 0 || line >= linesSize)
	{
		kDebug() << "alphabeta didn't return a correct line: " << line;
		kDebug() << "coosing random valid move";
		QList<int> freeLines = aiFunctions::getFreeLines(lines, linesSize);
		if (freeLines.size() <= 0)
		{
			kDebug() << "no valid lines left!";
			turnTime = moveTimer.elapsed();
			return 0;
		}
		turnTime = moveTimer.elapsed();
		return freeLines.at(qrand() % freeLines.size());
	}
	
	turnTime = moveTimer.elapsed();
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

float aiAlphaBeta::alphabeta(aiBoard::Ptr board, int depth, int *line, float alpha, float beta)
{
	KSquares::BoardAnalysis analysis = getAnalysis(board);
	
	if (analysis.moveSequences->size() == 0) // game is over
	{
		int result = 0;
		for (int i = 0; i < board->squareOwners.size(); i++)
		{
			result += board->squareOwners[i] == board->playerId ? 1 : -1;
		}
		// the player didn't change: return negative result!
		return -result;
	}
	
	bool terminalNode = false;
	if (depth == 0) terminalNode = true;
	if (line == NULL && alphabetaTimer.hasExpired(alphabetaTimeout)) 
	{
		terminalNode = true;
	}
	if (terminalNode)
	{
		heuristic->setAnalysis(analysis);
		float eval = evaluate(board);
		return eval;
	}
	
#ifdef KSQUARES_ALPHABETA_ITERATIVE_DEEPENING
	QList<QPair<int, float> > moveSeqValues;
#endif
	float bestValue = -INFINITY;
	int analyzedRootChildren = 0;
	for (int i = 0; i < analysis.moveSequences->size() && (!alphabetaTimer.hasExpired(alphabetaTimeout) || line != NULL); i++)
	{
		if (!alphabetaTimer.hasExpired(alphabetaTimeout) && line != NULL)
			analyzedRootChildren++;
		int prevPlayer = board->playerId;
		for (int j = 0; j < (*(analysis.moveSequences))[i].size(); j++)
		{
			board->doMove((*(analysis.moveSequences))[i][j]);
		}
		// TODO: remove this check
		if (prevPlayer == board->playerId && board->squareOwners.contains(-1))
		{
			kDebug() << "ERROR: sth went really wrong! player didn't change after move sequence: " << (*(analysis.moveSequences))[i];
			kDebug() << "ERROR: board: " << aiFunctions::boardToString(board);
		}
		float val = -alphabeta(board, depth - 1, NULL, -beta, -alpha);
		for (int j = (*(analysis.moveSequences))[i].size() -1; j >= 0; j--)
		{
			board->undoMove((*(analysis.moveSequences))[i][j]);
		}
#ifdef KSQUARES_ALPHABETA_ITERATIVE_DEEPENING
		moveSeqValues.append(QPair<int, float>(i, val));
#endif
		if (val > bestValue)
		{
			bestValue = val;
			if (line != NULL)
			{
				*line = (*(analysis.moveSequences))[i][0];
			}
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
	
#ifdef KSQUARES_ALPHABETA_ITERATIVE_DEEPENING
	qSort(moveSeqValues.begin(), moveSeqValues.end(), MoveSequenceSorter());
	QList<QList<int> > tmpMoveSeqs;
	for (int j = 0; j < moveSeqValues.size(); j++)
	{
		tmpMoveSeqs.append((*(analysis.moveSequences))[moveSeqValues[j].first]);
	}
	analysis.moveSequences->clear();
	analysis.moveSequences->append(tmpMoveSeqs);
#else
	// if search only analyzed very few (10%) children of root node do a shallow heuristic based search!
	if (line != NULL && alphabetaTimer.hasExpired(alphabetaTimeout) && 1.0 / (double)analysis.moveSequences->size() * (double)analyzedRootChildren < 0.1)
	{
		kDebug() << "original alphabeta search didn't analyze enough root children, doing shallow search";
		bestValue = -INFINITY;
		for (int i = 0; i < analysis.moveSequences->size(); i++)
		{
			for (int j = 0; j < (*(analysis.moveSequences))[i].size(); j++)
			{
				board->doMove((*(analysis.moveSequences))[i][j]);
			}
			KSquares::BoardAnalysis tmpAnalysis = getAnalysis(board);
			heuristic->setAnalysis(tmpAnalysis);
			float val = -evaluate(board);
			for (int j = (*(analysis.moveSequences))[i].size() -1; j >= 0; j--)
			{
				board->undoMove((*(analysis.moveSequences))[i][j]);
			}
			if (val > bestValue)
			{
				bestValue = val;
				if (line != NULL)
				{
					*line = (*(analysis.moveSequences))[i][0];
				}
			}
			
		}
	}
#endif
	
	return bestValue;
}

float aiAlphaBeta::evaluate(aiBoard::Ptr board)
{
	float ret = heuristic->evaluate(board);
	return ret;
}

KSquares::BoardAnalysis aiAlphaBeta::getAnalysis(aiBoard::Ptr board)
{
	KSquares::BoardAnalysis analysis;
	if (analysisHash->contains(board))
	{
		QPair<TranspositionEntry, TranspositionEntry> entries = analysisHash->value(board);
		for (int j = 0; j < 2; j++)
		{
			TranspositionEntry entry = j == 0 ? entries.first : entries.second;
			bool sameBoard = true;
			for (int i = 0; i < board->linesSize && sameBoard; i++)
			{
				if (entry.first[i] != board->lines[i])
					sameBoard = false;
			}
			if (sameBoard)
			{
				return entry.second;
			}
		}
		
		// same hash but not same board -> apply two big replacement scheme
		analysis = BoardAnalysisFunctions::analyseBoard(board);
		if (analysis.moveSequences->size() > entries.first.second.moveSequences->size())
		{
			delete[] entries.second.first;
			entries.second = entries.first;
			bool *linesCopy = new bool[board->linesSize];
			memcpy(linesCopy, board->lines, board->linesSize);
			entries.first = QPair<bool *, KSquares::BoardAnalysis>(linesCopy, analysis);
			analysisHash->insert(board, entries);
		}
		else
		{
			delete[] entries.second.first;
			bool *linesCopy = new bool[board->linesSize];
			memcpy(linesCopy, board->lines, board->linesSize);
			entries.second = QPair<bool *, KSquares::BoardAnalysis>(linesCopy, analysis);
			analysisHash->insert(board, entries);
		}
	}
	else
	{
		analysis = BoardAnalysisFunctions::analyseBoard(board);
		
		bool *linesCopy = new bool[board->linesSize];
		memcpy(linesCopy, board->lines, board->linesSize);
		bool *linesCopy2 = new bool[board->linesSize];
		memcpy(linesCopy2, board->lines, board->linesSize);
		
		QPair<TranspositionEntry, TranspositionEntry> newEntry;
		newEntry.first = QPair<bool *, KSquares::BoardAnalysis>(linesCopy, analysis);
		newEntry.second = QPair<bool *, KSquares::BoardAnalysis>(linesCopy2, analysis); // TODO: insert NULL pointer and check for them instead?
		
		analysisHash->insert(board, newEntry);
	}
	
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

bool MoveSequenceSorter::operator()(QPair<int, float> a, QPair<int, float> b) const
{
	return a.second > b.second;
}
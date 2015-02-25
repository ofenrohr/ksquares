/***************************************************************************
 *   Copyright (C) 2015 by Tom Vincent Peters    <kde@vincent-peters.de>   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "boardAnalysis.h"

#include "aifunctions.h"
#include "lineSorter.h"

KSquares::BoardAnalysis BoardAnalysisFunctions::analyseBoard(aiBoard::Ptr board)
{
	LineSorter sorter(board->width, board->height, board->linesSize);
	QList<int> lineSortList = sorter.getSortMap();
	return analyseBoard(board, lineSortList);
}

KSquares::BoardAnalysis BoardAnalysisFunctions::analyseBoard(aiBoard::Ptr board, QList<int> &lineSortList)
{
	KSquares::BoardAnalysis analysis;
	
	// look for capturable chains
	aiFunctions::findChains(board, &(analysis.chains), true);
	
	// sort capturable chains by classification
	for (int i = 0; i < analysis.chains.size(); i++)
	{
		switch (analysis.chains[i].type)
		{
			case KSquares::CHAIN_LONG:
				if (analysis.chains[i].ownChain)
					analysis.capturableLongChains.append(i);
			break;
			case KSquares::CHAIN_LOOP:
				if (analysis.chains[i].ownChain)
					analysis.capturableLoopChains.append(i);
			break;
			case KSquares::CHAIN_SHORT:
				if (analysis.chains[i].ownChain)
					analysis.capturableShortChains.append(i);
			break;
			case KSquares::CHAIN_SPECIAL:
				kDebug() << "ERROR: special own chain!" << analysis.chains[i];
			break;
			case KSquares::CHAIN_UNKNOWN:
			default:
				kDebug() << "WARNING: unknown chain! " << analysis.chains[i];
			break;
		}
		// capture everything that can be captured
		if (analysis.chains[i].ownChain)
			for (int j = 0; j < analysis.chains[i].lines.size(); j++)
				board->doMove(analysis.chains[i].lines[j]);
	}
	
	//kDebug() << "board after capture " << aiFunctions::boardToString(board);
	
	// look for chains a second time
	aiFunctions::findChains(board, &(analysis.chainsAfterCapture));
	
	// sort chains by classification
	for (int i = 0; i < analysis.chainsAfterCapture.size(); i++)
	{
		switch (analysis.chainsAfterCapture[i].type)
		{
			case KSquares::CHAIN_LONG:
				if (!analysis.chainsAfterCapture[i].ownChain)
					analysis.openLongChains.append(i);
				else
				{
					kDebug() << "ERROR: capturable chain found when there should be none! chain: " << aiFunctions::linelistToString(analysis.chainsAfterCapture[i].lines, board->linesSize, board->width, board->height) << " on board " << aiFunctions::boardToString(board);
					for (int j = 0; j < analysis.chains.size(); j++)
						kDebug() << "capture chain: " << aiFunctions::linelistToString(analysis.chains[j].lines, board->linesSize, board->width, board->height);
				}
			break;
			case KSquares::CHAIN_LOOP:
				if (!analysis.chainsAfterCapture[i].ownChain)
					analysis.openLoopChains.append(i);
				else
				{
					kDebug() << "ERROR: capturable chain found when there should be none! chain: " << aiFunctions::linelistToString(analysis.chainsAfterCapture[i].lines, board->linesSize, board->width, board->height) << " on board " << aiFunctions::boardToString(board);
					for (int j = 0; j < analysis.chains.size(); j++)
						kDebug() << "capture chain: " << aiFunctions::linelistToString(analysis.chains[j].lines, board->linesSize, board->width, board->height);
				}
			break;
			case KSquares::CHAIN_SHORT:
				if (!analysis.chainsAfterCapture[i].ownChain)
					analysis.openShortChains.append(i);
				else
				{
					kDebug() << "ERROR: capturable chain found when there should be none! chain: " << aiFunctions::linelistToString(analysis.chainsAfterCapture[i].lines, board->linesSize, board->width, board->height) << " on board " << aiFunctions::boardToString(board);
					for (int j = 0; j < analysis.chains.size(); j++)
						kDebug() << "capture chain: " << aiFunctions::linelistToString(analysis.chains[j].lines, board->linesSize, board->width, board->height);
				}
			break;
			case KSquares::CHAIN_SPECIAL:
				if (!analysis.chainsAfterCapture[i].ownChain)
				{
					for (int j = 0; j < analysis.chainsAfterCapture[i].lines.size(); j++)
					{
						analysis.specialLines.append(analysis.chainsAfterCapture[i].lines[j]);
					}
				}
				else
				{
					kDebug() << "ERROR: capturable chain found when there should be none! chain: " << aiFunctions::linelistToString(analysis.chainsAfterCapture[i].lines, board->linesSize, board->width, board->height) << " on board " << aiFunctions::boardToString(board);
					for (int j = 0; j < analysis.chains.size(); j++)
						kDebug() << "capture chain: " << aiFunctions::linelistToString(analysis.chains[j].lines, board->linesSize, board->width, board->height);
				}
			break;
			case KSquares::CHAIN_UNKNOWN:
			default:
				kDebug() << "WARNING: unknown chain! " << analysis.chainsAfterCapture[i].lines;
			break;
		}
	}
	
	// look for safe moves
	analysis.safeLines = aiFunctions::safeMoves(board->width, board->height, board->linesSize, board->lines);
	
	// undo capture moves
	for (int i = 0; i < analysis.chains.size(); i++)
	{
		if (!analysis.chains[i].ownChain)
			continue;
		for (int j = analysis.chains[i].lines.size()-1; j >= 0; j--)
			board->undoMove(analysis.chains[i].lines[j]);
	}
	
	analysis.moveSequences = getMoveSequences(board, analysis, lineSortList);
	
	return analysis;
}


QList<int> BoardAnalysisFunctions::getDoubleDealingSequence(KSquares::Chain &chain)
{
	QList<int> ret;
	
	if (chain.type != KSquares::CHAIN_LONG && chain.type != KSquares::CHAIN_LOOP && chain.type != KSquares::CHAIN_SHORT)
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
	
	if (chain.type == KSquares::CHAIN_SHORT)
	{
		if (chain.squares.size() < 2 || chain.lines.size() < 2)
		{
			ret.clear();
			return ret;
		}
		ret.removeAt(ret.size() - 2);
	}
	
	return ret;
}

QList<int> BoardAnalysisFunctions::ignoreCornerLines(aiBoard::Ptr board)
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

/*
QSharedPointer<QList<QList<int> > > BoardAnalysisFunctions::getMoveSequences(aiBoard::Ptr board, KSquares::BoardAnalysis &analysis, bool *isEndgame)
{
	return getMoveSequences(board, analysis, lineSortList, isEndgame);
}
*/

QSharedPointer<QList<QList<int> > > BoardAnalysisFunctions::getMoveSequences(aiBoard::Ptr board, KSquares::BoardAnalysis &analysis, QList<int> &lineSortList, bool *isEndgame)
{
	QSharedPointer<QList<QList<int> > > moveSequences(new QList<QList<int> >);
	QList<QList<int> > chainSacrificeSequences;
	
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
			//kDebug() << "short or loop chain: " << analysis.chains[capturableShortAndLoopChains[i]];
			//kDebug() << "doubleDealingVariant: " << doubleDealingVariant;
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
			moveSequences->append(doubleDealingSequence);
		}
		
		// add normal variant to basic move sequence
		baseMoveSequence.append(analysis.chains[doubleDealingChainIndex].lines);
	}
	else
	{
		//kDebug() << "no double dealing chain available";
	}
	
	// add full capture version
	// move sequences for open chains + free lines
	// get free lines and filter them
	QList<int> freeLines; // = aiFunctions::getFreeLines(board->lines, board->linesSize);
	QMap<int, int> freeLinesMap;
	QList<int> ignoreLines = ignoreCornerLines(board);
	//QList<int> ignoreLines;
	for (int i = 0; i < board->linesSize; i++)
		if (!board->lines[i] && !baseMoveSequence.contains(i) && !ignoreLines.contains(i))
			freeLines.append(i);
			//freeLinesMap.insert(lineSortList[i], i);
	//freeLines = freeLinesMap.values();
	qSort(freeLines.begin(), freeLines.end(), LineSorter(board->width,board->height,board->linesSize));
	//kDebug() << "free lines: " << freeLines;
	if (freeLines.size() == 0 && baseMoveSequence.size() > 0)
		moveSequences->append(baseMoveSequence);
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
		//moveSequences->append(moveSequence);
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
		//moveSequences->append(halfHeartedSequence);
		chainSacrificeSequences.prepend(halfHeartedSequence);
		QList<int> hardHeartedSequence;
		hardHeartedSequence.append(baseMoveSequence);
		hardHeartedSequence.append(analysis.chainsAfterCapture[analysis.openShortChains[i]].lines[1]);
		//moveSequences->append(hardHeartedSequence);
		chainSacrificeSequences.prepend(hardHeartedSequence);
		for (int j = 0; j < analysis.chainsAfterCapture[analysis.openShortChains[i]].lines.size(); j++)
		{
			freeLines.removeAll(analysis.chainsAfterCapture[analysis.openShortChains[i]].lines[j]);
		}
	}
	for (int i = 0; i < analysis.specialLines.size(); i++)
		freeLines.append(analysis.specialLines[i]);
	if (isEndgame != NULL)
		*isEndgame = freeLines.size() > 0;
	// add all that's left
	for (int i = 0; i < freeLines.size(); i++)
	{
		QList<int> moveSequence;
		moveSequence.append(baseMoveSequence);
		moveSequence.append(freeLines[i]);
		moveSequences->append(moveSequence);
	}
	// add chain sacrifice moves
	moveSequences->append(chainSacrificeSequences);
	
	return moveSequences;
}

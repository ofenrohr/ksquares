/***************************************************************************
 *   Copyright (C) 2006 by Matthew Williams    <matt@milliams.com>         *
 *   Copyright (C) 2014 by Tom Vincent Peters  <kde@vincent-peters.de>     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "aicontroller.h"

#include "aiEasyMediumHard.h"
//#include "aiMiniMax.h"
#include "aiAlphaBeta.h"
#include "dbgame.h"
//#include "dbgame-nohash.h"
#include "qdab.h"
#include "knox.h"
#include "aiMCTS.h"
#include "aiDabbleNative.h"
#include "aiConvNet.h"
#include "aiAlphaZeroMCTS.h"

#include <ctime>
#include <QDebug>

#include <QSet>
#include <alphaDots/ProtobufConnector.h>
#include <QtWidgets/QMessageBox>
#include <QtCore/QCoreApplication>

aiController::aiController(int newPlayerId, int newMaxPlayerId, int newWidth, int newHeight, int newLevel, int thinkTime, QString model, bool gpu) :
		playerId(newPlayerId),
		maxPlayerId(newMaxPlayerId),
		width(newWidth),
		height(newHeight),
		level(newLevel),
		aiThinkTime(thinkTime),
		useGPU(gpu)
{
	lastTurnTime = -2;
	if (!model.isEmpty()) {
		alphaDotsModel = AlphaDots::ProtobufConnector::getInstance().getModelByName(model);
		alphaDotsActive = true;
	} else {
		alphaDotsActive = false;
	}
}

aiController::~aiController()
{
	//delete[] lines;
}

QList<int> aiController::autoFill(int safeMovesLeft, int width, int height)
{
	auto *rng = gsl_rng_alloc(gsl_rng_taus);
	gsl_rng_set(rng, std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count());

	QList<int> fillLines;
	
	int linesSize = aiFunctions::toLinesSize(width, height);
	bool *lines = new bool[linesSize];
	for (int i = 0; i < linesSize; i++)
	{
		lines[i] = false;
	}
	// add a random safe moves while there are safe moves left
	QList<int> next;
	//qDebug() << safeMoves().isEmpty();
	while( !( (next = aiFunctions::safeMoves(width, height, linesSize, lines)).isEmpty() ) )
	{
		int nextLine = next[gsl_rng_uniform_int(rng, next.size())];
		lines[nextLine] = true;
		//qDebug() << nextLine;
		fillLines << nextLine;
	}
	
	// safeMovesLeft times delete a line from fillLines
	for (int i = 0; i<safeMovesLeft; ++i)
	{
		if (fillLines.isEmpty()) break;
		int index = gsl_rng_uniform_int(rng,fillLines.size());
		fillLines.removeAt(index);
	}

	delete[] lines;
	return fillLines;
}

int aiController::chooseLine(const QList<bool> &newLines, const QList<int> &newSquareOwners, const QList<Board::Move> &lineHistory)
{
	KSquaresAi::Ptr ai = getAi();
    lastTurnTime = ai->lastMoveTime();
    return ai->chooseLine(newLines, newSquareOwners, lineHistory);
}

KSquaresAi::Ptr aiController::getAi()
{
	switch (level)
	{
		default:
			qDebug() << "Unknown ai level " << level;
			assert(false);
		case KSquares::AI_EASY:
		case KSquares::AI_MEDIUM:
		case KSquares::AI_HARD:
			//qDebug() << "creating aiEasyMediumHard: w = " << width << ", h = " << height;
			ai = KSquaresAi::Ptr(new aiEasyMediumHard(playerId, width, height, level));
		break;
		case KSquares::AI_VERYHARD:
			//qDebug() << "creating aiAlphaBeta";
			if (ai.isNull())
				ai = KSquaresAi::Ptr(new aiAlphaBeta(playerId, maxPlayerId, width, height, level, aiThinkTime));
		break;
		case KSquares::AI_DABBLE:
			if (ai.isNull())
				ai = KSquaresAi::Ptr(new Dabble(playerId, maxPlayerId, width, height, 0, aiThinkTime));
		break;
		case KSquares::AI_DABBLENOHASH:
			if (ai.isNull())
				ai = KSquaresAi::Ptr(new Dabble(playerId, maxPlayerId, width, height, 1, aiThinkTime));
		case KSquares::AI_QDAB:
			if (ai.isNull())
				ai = KSquaresAi::Ptr(new QDab(playerId, maxPlayerId, width, height, level, aiThinkTime));
		break;
		case KSquares::AI_KNOX:
			if (ai.isNull())
				ai = KSquaresAi::Ptr(new Knox(playerId, maxPlayerId, width, height, level, aiThinkTime));
		break;
		case KSquares::AI_MCTS_A:
			if (ai.isNull())
				ai = KSquaresAi::Ptr(new aiMCTS(playerId, maxPlayerId, width, height, KSquares::AI_EASY, aiThinkTime));
		break;
		case KSquares::AI_MCTS_B:
			if (ai.isNull())
				ai = KSquaresAi::Ptr(new aiMCTS(playerId, maxPlayerId, width, height, KSquares::AI_MEDIUM, aiThinkTime));
		break;
		case KSquares::AI_MCTS_C:
			if (ai.isNull())
				ai = KSquaresAi::Ptr(new aiMCTS(playerId, maxPlayerId, width, height, KSquares::AI_HARD, aiThinkTime));
		break;
        case KSquares::AI_DABBLENATIVE:
			if (ai.isNull())
				ai = KSquaresAi::Ptr(new aiDabbleNative(playerId, maxPlayerId, width, height, level, aiThinkTime));
		break;
        case KSquares::AI_CONVNET:
			if (!alphaDotsActive) {
				QMessageBox::critical(nullptr, QObject::tr("AlphaDots missing"), QObject::tr("Can't execute AI without alphaDots"));
				QCoreApplication::exit(1);
			}
			if (ai.isNull())
				ai = KSquaresAi::Ptr(new aiConvNet(playerId, maxPlayerId, width, height, level, aiThinkTime, alphaDotsModel, useGPU));
		break;
		case KSquares::AI_MCTS_CONVNET:

			if (!alphaDotsActive) {
				QMessageBox::critical(nullptr, QObject::tr("AlphaDots missing"), QObject::tr("Can't execute AI without alphaDots"));
				QCoreApplication::exit(1);
			}
			if (ai.isNull())
				ai = KSquaresAi::Ptr(new aiMCTS(playerId, maxPlayerId, width, height, KSquares::AI_CONVNET, aiThinkTime));
		break;
		case KSquares::AI_MCTS_ALPHAZERO:
			if (!alphaDotsActive) {
				QMessageBox::critical(nullptr, QObject::tr("AlphaDots missing"), QObject::tr("Can't execute AI without alphaDots"));
				QCoreApplication::exit(1);
			}
			if (ai.isNull())
				ai = KSquaresAi::Ptr(new AlphaDots::aiAlphaZeroMCTS(playerId, maxPlayerId, width, height, aiThinkTime, alphaDotsModel, useGPU));
		break;
	}
	return ai;
}


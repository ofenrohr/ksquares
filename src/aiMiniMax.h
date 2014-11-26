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
#include "aiBoard.h"

#include <QString>


class aiMiniMax : public KSquaresAi
{
	public:
		aiMiniMax(int newPlayerId, int newMaxPlayerId, int newWidth, int newHeight, int newLevel);
		~aiMiniMax();
		
		int chooseLine(const QList<bool> &newLines, const QList<int> &newSquareOwners);
		
		float minimax(aiBoard &board, int depth, int *line, int parentNode = -1);
		static float evaluate(aiBoard &board);
		
		void setDebug(bool val);
		QString getDebugDot();
		
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
		
		/// enable debugging
		bool debug;
		QString debugDot;
		int debugNodeCnt;
};

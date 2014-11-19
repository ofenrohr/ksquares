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

class aiMiniMax : public KSquaresAi, public aiFunctions
{
	public:
		aiMiniMax(int newPlayerId, int newWidth, int newHeight, int newLevel);
		int chooseLine(const QList<bool> &newLines, const QList<int> &newSquareOwners) const;
	protected:
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
};

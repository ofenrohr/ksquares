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

class aiEasyMediumHard : public KSquaresAi
{
	public:
		aiEasyMediumHard(int newPlayerId, int newWidth, int newHeight, int newLevel);
		//~aiEasyMediumHard();
		int chooseLine(const QList<bool> &newLines, const QList<int> &newSquareOwners);
	protected:
		/**
			* @param choiceList list of indices (of lines) which have squares next to them with two lines drawn (relates to @ref lines )
			*
			* @return list of indices (of lines) which would be the least damaging in the short term
			*/
		QList<int> chooseLeastDamaging(const QList<int> &choiceList) const;
	
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
/***************************************************************************
 *   Copyright (C) 2015 by Tom Vincent Peters    <kde@vincent-peters.de>   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "lineSorter.h"

#include <cmath>
#include <algorithm>
#include <kdebug.h>

LineSorter::LineSorter(int width, int height, int linesSize) : w(width), h(height), ls(linesSize)
{
	kDebug() << "center: " << (w/2.0) << ", " << (h/2.0);
}

float LineSorter::distanceToCenter(const int index) const
{
	int p1x = 0;
	int p1y = 0;
	int index2 = index % ( ( 2 * w ) + 1 );
	p1y = index / ( ( 2 * w ) + 1);
	
	if (index % ((2*w) + 1) < w)
	{
		p1x = index2;
	}
	else 
	{
		p1x = index2 - w;
	}
	p1y = h - p1y;
	
	return sqrt((p1x - (w/2.0)) * (p1x - (w/2.0)) + ((p1y - (h/2.0)) * (p1y - (h/2.0))));
}


int LineSorter::operator()(int a, int b) const
{
	return distanceToCenter(b) - distanceToCenter(a);
}

QList<int> LineSorter::getSortMap()
{
	QList<int> lineList;
	for (int i = 0; i < ls; i++)
		lineList.append(i);
	qSort(lineList.begin(), lineList.end(), LineSorter(w,h,ls));
	return lineList;
}
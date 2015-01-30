/***************************************************************************
 *   Copyright (C) 2015 by Tom Vincent Peters    <kde@vincent-peters.de>   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef LINESORTER_H
#define LINESORTER_H

#include <QList>
#include <QPair>

class LineSorter
{
public:
	LineSorter(int width, int height, int linesSize);
	
	bool indexToPoints(const int index, QPair<float, float> *p1, QPair<float, float> *p2) const;
	float distanceToCenter(const int index) const;
	QList<int> getSortMap();
	bool operator()(int a, int b) const;
private:
	int w;
	int h;
	int ls;
	float cx;
	float cy;
};

#endif
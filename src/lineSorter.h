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

class LineSorter
{
public:
	LineSorter(int width, int height, int linesSize);
	
	float distanceToCenter(const int index) const;
	QList<int> getSortMap();
	int operator()(int a, int b) const;
private:
	int w;
	int h;
	int ls;
};

#endif
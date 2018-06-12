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
#include <QDebug>
#include <QMap>

LineSorter::LineSorter(int width, int height, int linesSize) : w(width), h(height), ls(linesSize)
{
	cx = w / 2.0;
	cy = h / 2.0;
}


bool LineSorter::indexToPoints(const int index, QPair<float, float> *p1, QPair<float, float> *p2) const
{
  int index2 = index % ( ( 2 * w ) + 1 );
  p1->second = index / ( ( 2 * w ) + 1);
  if (index % ((2*w) + 1) < w)
  {
    p1->first = index2;
    p2->second = p1->second;
    p2->first = p1->first + 1;
  }
  else 
  {
    p1->first = index2 - w;
    p2->second = p1->second + 1;
    p2->first = p1->first;
  }
  p1->second = h - p1->second;
  p2->second = h - p2->second;
  return true;
}


float LineSorter::distanceToCenter(const int index) const
{
	QPair<float, float> p1, p2, p3;
	indexToPoints(index, &p1, &p2);
	p3.first = (p1.first + p2.first) / 2;
	p3.second = (p1.second + p2.second) / 2;
	//qDebug() << "p3: " << p3.first << ", " << p3.second;
	return sqrt((p3.first-cx) * (p3.first-cx) + (p3.second-cy) * (p3.second-cy));
}


bool LineSorter::operator()(int a, int b) const
{
	return distanceToCenter(a) - distanceToCenter(b) < 0;
	/*
	if (res < 0) return 1;
	if (res > 0) return -1;
	return 0;
	*/
}

QList<int> LineSorter::getSortMap()
{
	QList<int> lineList;
	for (int i = 0; i < ls; i++)
		lineList.append(i);
	qSort(lineList.begin(), lineList.end(), LineSorter(w,h,ls));
	return lineList;
	/*
	QMap<float, int> lineMap;
	for (int i = 0; i < ls; i++)
		lineMap.insert(distanceToCenter(i), i);
	return lineMap.values();
	*/
}

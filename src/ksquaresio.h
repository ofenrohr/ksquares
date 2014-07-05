/***************************************************************************
 *   Copyright (C) 2014 by Tom Vincent Peters    <kde@vincent-peters.de>   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef KSQUARESIO_H
#define KSQUARESIO_H

// qt
#include <QObject>
#include <QString>
#include <QPoint>

// classes
#include "ksquaresgame.h"

class KSquaresIO : public QObject 
{
	Q_OBJECT
	
public:
	
	static bool loadGame(QString filename, KSquaresGame *sGame, QList<int> *lines);
	static bool saveGame(QString filename, KSquaresGame *sgame);
	
};
  
#endif // KSQUARESIO_H
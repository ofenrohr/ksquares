/***************************************************************************
 *   Copyright (C) 2014 by Tom Vincent Peters    <kde@vincent-peters.de>   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

// header
#include "ksquaresio.h"

// qt
#include <QFile>
#include <QTextStream>
#include <QStringList>
#include <QList>

// kde
#include <kdebug.h>
#include <KLocale>

bool KSquaresIO::loadGame(QString filename, KSquaresGame *sGame, QList<int> *lines)
{ 
  // open the file
  QFile file(filename);
  if (!file.open(QIODevice::ReadOnly))
  {
    kDebug() << "KSquaresIO::loadGame error: Can't open file";
    return false;
  }
  
  QVector<KSquaresPlayer> players;
  // check extension of file
  if (filename.endsWith(".dbl"))
  {
    // loading a dabble game
    QList<int> isHuman;
    isHuman.append(1);
    isHuman.append(1);
    players = KSquaresGame::createPlayers(2,isHuman);
    KSquaresPlayer p1 = players.at(0);
    p1.setName(i18n("Player 1"));
    KSquaresPlayer p2 = players.at(1);
    p2.setName(i18n("Player 2"));
  }
  else if (filename.endsWith(".ksq"))
  {
    // loading a ksquares game
  }
  else
  {
    kDebug() << "KSquaresIO::loadGame error: invalid file extension";
    return false;
  }
  
  QTextStream inStream(&file);
  
  bool sizeDefined = false;
  bool firstIteration = true;
  int width = -1;
  int height = -1;
  QList<int> lines_;
  
  while (!inStream.atEnd())
  {
    QString line = inStream.readLine();
    line = line.replace(" ", "");

    if (line.startsWith("#"))
      continue;
    
    if (!sizeDefined)
    {
      if (!firstIteration)
      {
        kDebug() << "KSquaresIO::loadGame error: board size not defined in first line";
        return false;
      }
      if (!line.contains(","))
      {
        kDebug() << "KSquaresIO::loadGame error: invalid board size";
        return false;
      }
      QStringList wh = line.split(",");
      bool ok;
      width = wh[0].toInt(&ok);
      if (!ok)
      {
        kDebug() << "KSquaresIO::loadGame error: board width invalid";
        return false;
      }
      height = wh[1].toInt(&ok);
      if (!ok)
      {
        kDebug() << "KSquaresIO::loadGame error: board height invalid";
        return false;
      }
      sizeDefined = true;
    }
    else
    {
      if (!line.contains("-"))
      {
        kDebug() << "KSquaresIO::loadGame error: invalid line definition";
        return false;
      }
      QStringList p12 = line.replace(")","").replace("(","").split("-");
      QPoint p1, p2;
      if (!p12[0].contains(",") || !p12[1].contains(","))
      {
        kDebug() << "KSquaresIO::loadGame error: invalid line point definition";
        return false;
      }
      QStringList p1s = p12[0].split(",");
      QStringList p2s = p12[1].split(",");
      bool ok;
      p1.setX(p1s[0].toInt(&ok));
      if (!ok)
      {
        kDebug() << "KSquaresIO::loadGame error: line point p1x invalid";
        return false;
      }
      p1.setY(p1s[1].toInt(&ok));
      if (!ok)
      {
        kDebug() << "KSquaresIO::loadGame error: line point p1y invalid";
        return false;
      }
      p2.setX(p2s[0].toInt(&ok));
      if (!ok)
      {
        kDebug() << "KSquaresIO::loadGame error: line point p2x invalid";
        return false;
      }
      p2.setY(p2s[1].toInt(&ok));
      if (!ok)
      {
        kDebug() << "KSquaresIO::loadGame error: line point p2y invalid";
        return false;
      }
      
      int idx = Board::pointsToIndex(p1, p2, width, height);
      if (idx < 0)
      {
        kDebug() << "KSquaresIO::loadGame error: line index invalid";
        return false;
      }
      lines_.append(idx);
    }
    
    firstIteration = false;
  }
  
  file.close();
  
  sGame->createGame(players, width, height);
  for (int i = 0; i < lines_.size(); i++) {
    lines->append(lines_.at(i));
  }
  return true;
}

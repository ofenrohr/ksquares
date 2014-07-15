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
#include <kmessagebox.h>

bool KSquaresIO::loadGame(QString filename, KSquaresGame *sGame, QList<int> *lines)
{ 
	// open the file
	QFile file(filename);
	if (!file.open(QIODevice::ReadOnly))
	{
		kDebug() << "KSquaresIO::loadGame error: Can't open file";
		return false;
	}
	
	QTextStream inStream(&file);
	
	bool sizeDefined = false;
	bool firstIteration = true;
	int width = -1;
	int height = -1;
	int playerCnt = -1;
	QVector<KSquaresPlayer> players;
	QList<int> lines_;
	bool success = true;
	
	// check extension of file
	if (filename.endsWith(".dbl"))
	{
		// loading a dabble game
		QList<int> isHuman;
		//KMessageBox::questionYesNo(NULL, i18n("Is the first dabble player human?"), i18n("Human player?"));
		isHuman.append(1); // TODO: use result of question
		//KMessageBox::questionYesNo(NULL, i18n("Is the second dabble player human?"), i18n("Human player?"));
		isHuman.append(1); // TODO: use result of question
		players = KSquaresGame::createPlayers(2,isHuman);
		KSquaresPlayer p1 = players.at(0);
		p1.setName(i18n("Player 1"));
		KSquaresPlayer p2 = players.at(1);
		p2.setName(i18n("Player 2"));
		playerCnt = 2;
	}
	else if (filename.endsWith(".ksq"))
	{
		// loading a ksquares game
		if (!inStream.atEnd())
		{
			QString line = inStream.readLine();
			bool ok;
			playerCnt = line.toInt(&ok);
			if (!ok || playerCnt < 2 || playerCnt > 4)
			{
				success = false;
			}
		}
		else
		{
			success = false;
		}
		
		int readingPlayer = 0;
		QList<int> isHuman;
		QList<QString> playerNames;
		while (!inStream.atEnd() && success)
		{
			// get player info (type and name)
			QString line = inStream.readLine();
			if (!line.contains(","))
			{
				success = false;
				break;
			}
			QStringList playerInfo = line.split(",");
			
			// read player type
			bool ok;
			int isPlayerHuman = playerInfo[0].toInt(&ok);
			if (!ok)
			{
				success = false;
				break;
			}
			isHuman.append(isPlayerHuman);
			
			// read player name
			if (playerInfo[1].isEmpty())
			{
				success = false;
				break;
			}
			playerNames.append(playerInfo[1]);
			
			readingPlayer++;
		}
		
		if (readingPlayer != playerCnt)
		{
			success = false;
		}
		else
		{
			players = KSquaresGame::createPlayers(playerCnt, isHuman);
			for (int i = 0; i < playerCnt; i++)
			{
				players[i].setName(playerNames[i]);
			}
		}
	}
	else
	{
		kDebug() << "KSquaresIO::loadGame error: invalid file extension";
		success = false;
	}
	
	
	while (!inStream.atEnd() && success)
	{
		QString line = inStream.readLine();
		line = line.replace(" ", "");

		if (line.startsWith("#"))
			continue;
		
		if (!sizeDefined)
		{
			// read board size
			if (!firstIteration)
			{
				kDebug() << "KSquaresIO::loadGame error: board size not defined in first line";
				success = false;
				break;
			}
			if (!line.contains(","))
			{
				kDebug() << "KSquaresIO::loadGame error: invalid board size";
				success = false;
				break;
			}
			QStringList wh = line.split(","); // wh = width/height
			bool ok;
			width = wh[0].toInt(&ok);
			if (!ok)
			{
				kDebug() << "KSquaresIO::loadGame error: board width invalid";
				success = false;
				break;
			}
			height = wh[1].toInt(&ok);
			if (!ok)
			{
				kDebug() << "KSquaresIO::loadGame error: board height invalid";
				success = false;
				break;
			}
			sizeDefined = true;
		}
		else
		{
			// read a line
			if (!line.contains("-"))
			{
				kDebug() << "KSquaresIO::loadGame error: invalid line definition";
				success = false;
				break;
			}
			QStringList p12 = line.replace(")","").replace("(","").split("-");
			QPoint p1, p2;
			if (!p12[0].contains(",") || !p12[1].contains(","))
			{
				kDebug() << "KSquaresIO::loadGame error: invalid line point definition";
				success = false;
				break;
			}
			QStringList p1s = p12[0].split(",");
			QStringList p2s = p12[1].split(",");
			bool ok;
			p1.setX(p1s[0].toInt(&ok));
			if (!ok)
			{
				kDebug() << "KSquaresIO::loadGame error: line point p1x invalid";
				success = false;
				break;
			}
			p1.setY(p1s[1].toInt(&ok));
			if (!ok)
			{
				kDebug() << "KSquaresIO::loadGame error: line point p1y invalid";
				success = false;
				break;
			}
			p2.setX(p2s[0].toInt(&ok));
			if (!ok)
			{
				kDebug() << "KSquaresIO::loadGame error: line point p2x invalid";
				success = false;
				break;
			}
			p2.setY(p2s[1].toInt(&ok));
			if (!ok)
			{
				kDebug() << "KSquaresIO::loadGame error: line point p2y invalid";
				success = false;
				break;
			}
			
			int idx = Board::pointsToIndex(p1, p2, width, height);
			if (idx < 0)
			{
				kDebug() << "KSquaresIO::loadGame error: line index invalid";
				success = false;
				break;
			}
			lines_.append(idx);
		}
		
		firstIteration = false;
	}
	
	file.close();
	
	success = success && sizeDefined;
	
	if (success)
	{
		sGame->createGame(players, width, height);
		for (int i = 0; i < lines_.size(); i++) 
		{
			lines->append(lines_.at(i));
		}
	}
	
	return success;
}

bool KSquaresIO::saveGame(QString filename, KSquaresGame *sGame)
{
  // open the file
	QFile file(filename);
	if (!file.open(QIODevice::ReadWrite | QIODevice::Truncate))
	{
		kDebug() << "KSquaresIO::saveGame error: Can't open file";
		return false;
	}
	
	QTextStream outStream(&file);

	if (filename.endsWith(".dbl"))
	{
		// save in dabble format
		outStream << sGame->board()->width() << "," <<  sGame->board()->height() << "\n";
		QList<Board::Move> history = sGame->board()->getLineHistory();

		for (int i = 0; i < history.size(); i++) 
		{
			QPoint p1;
			QPoint p2;
			if (!sGame->board()->indexToPoints(history[i].line, &p1, &p2))
			{
				kDebug() << "KSquaresIO::saveGame error: invalid line in history";
				file.close();
				return false;
			}
			outStream << "(" << p1.x() << ", " << p1.y() << ") - (" << p2.x() << ", " << p2.y() << ")\n";
		} 
	}
	else if (filename.endsWith(".ksq"))
	{
		// save in ksquares format
		// TODO: implement saving as ksq
	}
	else if (filename.endsWith(".tex"))
	{
		// save in tex format
		outStream << "\\begin{pgfpicture}\n";
		outStream << "  \\pgfsetlinewidth{1pt}\n";
		for (int x = 0; x <= sGame->board()->width(); x++)
		{
			for (int y = 0; y <= sGame->board()->height(); y++)
			{
				outStream << "  \\pgfcircle[fill]{\\pgfxy(" << x << "," << y << ")}{3pt}\n";
			}
		}
		for (int i = 0; i < sGame->board()->getLineHistory().size(); i++)
		{
			QPoint p1;
			QPoint p2;
			if (!sGame->board()->indexToPoints(sGame->board()->getLineHistory()[i].line, &p1, &p2))
			{
				kDebug() << "KSquaresIO::saveGame error: iQIODevice::Truncatenvalid line in history";
				file.close();
				return false;
			}
			outStream << "  \\pgfxyline(" << p1.x() << ", " << ( sGame->board()->height() - p1.y() ) << ")(" << p2.x() << ", " << ( sGame->board()->height() - p2.y() ) << ")\n";
		}
		for (int i = 0; i < sGame->board()->squares().size(); i++)
		{
			if (sGame->board()->squares()[i] >= 0)
			{
				outStream << "  \\pgfputat{\\pgfxy(" << ( i % sGame->board()->width() ) << ".5," << ( sGame->board()->height() - 1 - i / sGame->board()->height() ) << ".5)}{\\pgfbox[center,center]{{\\LARGE " << (char)(sGame->board()->squares()[i] + 'A') << "}}}\n";
			}
		}
		outStream << "\\end{pgfpicture}\n";
	}
	else
	{
		// TODO: error message: unknown filetype
	}
  file.close();

  return true;
}

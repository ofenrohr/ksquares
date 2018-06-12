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
#include <QDebug>

// kde
#include <kmessagebox.h>

bool KSquaresIO::loadGame(QString filename, KSquaresGame *sGame, QList<int> *lines)
{ 
	// open the file
	QFile file(filename);
	if (!file.open(QIODevice::ReadOnly))
	{
		qDebug() << "KSquaresIO::loadGame error: Can't open file";
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
	if (filename.endsWith(QStringLiteral(".dbl")))
	{
		// loading a dabble game
		QList<int> isHuman;
		//KMessageBox::questionYesNo(NULL, i18n("Is the first dabble player human?"), i18n("Human player?"));
		isHuman.append(1); // TODO: use result of question
		//KMessageBox::questionYesNo(NULL, i18n("Is the second dabble player human?"), i18n("Human player?"));
		isHuman.append(1); // TODO: use result of question
		players = KSquaresGame::createPlayers(2,isHuman);
		KSquaresPlayer p1 = players.at(0);
		p1.setName(QStringLiteral("Player 1"));
		KSquaresPlayer p2 = players.at(1);
		p2.setName(QStringLiteral("Player 2"));
		playerCnt = 2;
	}
	else if (filename.endsWith(QStringLiteral(".ksq")))
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
			if (!line.contains(QStringLiteral(",")))
			{
				success = false;
				break;
			}
			QStringList playerInfo = line.split(QStringLiteral(","));
			
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
		qDebug() << "KSquaresIO::loadGame error: invalid file extension";
		success = false;
	}
	
	
	while (!inStream.atEnd() && success)
	{
		QString line = inStream.readLine();
		line = line.replace(QStringLiteral(" "), QStringLiteral(""));

		if (line.startsWith(QStringLiteral("#")))
			continue;
		
		if (!sizeDefined)
		{
			// read board size
			if (!firstIteration)
			{
				qDebug() << "KSquaresIO::loadGame error: board size not defined in first line";
				success = false;
				break;
			}
			if (!line.contains(QStringLiteral(",")))
			{
				qDebug() << "KSquaresIO::loadGame error: invalid board size";
				success = false;
				break;
			}
			QStringList wh = line.split(QStringLiteral(",")); // wh = width/height
			bool ok;
			width = wh[0].toInt(&ok);
			if (!ok)
			{
				qDebug() << "KSquaresIO::loadGame error: board width invalid";
				success = false;
				break;
			}
			height = wh[1].toInt(&ok);
			if (!ok)
			{
				qDebug() << "KSquaresIO::loadGame error: board height invalid";
				success = false;
				break;
			}
			sizeDefined = true;
		}
		else
		{
			// read a line
			if (!line.contains(QStringLiteral("-")))
			{
				qDebug() << "KSquaresIO::loadGame error: invalid line definition";
				success = false;
				break;
			}
			QStringList p12 = line.replace(QStringLiteral(")"),QStringLiteral("")).replace(QStringLiteral("("),QStringLiteral("")).split(QStringLiteral("-"));
			QPoint p1, p2;
			if (!p12[0].contains(QStringLiteral(",")) || !p12[1].contains(QStringLiteral(",")))
			{
				qDebug() << "KSquaresIO::loadGame error: invalid line point definition";
				success = false;
				break;
			}
			QStringList p1s = p12[0].split(QStringLiteral(","));
			QStringList p2s = p12[1].split(QStringLiteral(","));
			bool ok;
			p1.setX(p1s[0].toInt(&ok));
			if (!ok)
			{
				qDebug() << "KSquaresIO::loadGame error: line point p1x invalid";
				success = false;
				break;
			}
			p1.setY(p1s[1].toInt(&ok));
			if (!ok)
			{
				qDebug() << "KSquaresIO::loadGame error: line point p1y invalid";
				success = false;
				break;
			}
			p2.setX(p2s[0].toInt(&ok));
			if (!ok)
			{
				qDebug() << "KSquaresIO::loadGame error: line point p2x invalid";
				success = false;
				break;
			}
			p2.setY(p2s[1].toInt(&ok));
			if (!ok)
			{
				qDebug() << "KSquaresIO::loadGame error: line point p2y invalid";
				success = false;
				break;
			}
			
			int idx = Board::pointsToIndex(p1, p2, width, height);
			if (idx < 0)
			{
				qDebug() << "KSquaresIO::loadGame error: line index invalid";
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
		qDebug() << "KSquaresIO::saveGame error: Can't open file";
		return false;
	}
	
	QTextStream outStream(&file);

	if (filename.endsWith(QStringLiteral(".dabble.dbl")))
	{
		// save in dabble format
		outStream << (sGame->board()->width()+1) << "," << (sGame->board()->height()+1) << "\n";
		QList<Board::Move> history = sGame->board()->getLineHistory();

		for (int i = 0; i < history.size(); i++) 
		{
			QPoint p1;
			QPoint p2;
			if (!Board::indexToPoints(history[i].line, &p1, &p2, sGame->board()->width(), sGame->board()->height(), false))
			{
				qDebug() << "KSquaresIO::saveGame error: invalid line in history";
				file.close();
				return false;
			}
			//qDebug() << "conversion step one: dots and boxes coordinates: " << p1 << ", " << p2;
			QPair<QPoint, QPoint> dblPoints = Board::pointsToCoins(p1, p2, sGame->board()->width(), sGame->board()->height());
			//qDebug() << "conversion step two: strings and coins coordinated: " << dblPoints.first << ", " << dblPoints.second;
			outStream << "(" << dblPoints.first.x() << ", " << dblPoints.first.y() << ") - (" << dblPoints.second.x() << ", " << dblPoints.second.y() << ")\n";
		} 
	}
	else if (filename.endsWith(QStringLiteral(".dbl")))
	{
		// save in wrong dabble format
		outStream << sGame->board()->width() << "," <<  sGame->board()->height() << "\n";
		QList<Board::Move> history = sGame->board()->getLineHistory();

		for (int i = 0; i < history.size(); i++) 
		{
			QPoint p1;
			QPoint p2;
			if (!sGame->board()->indexToPoints(history[i].line, &p1, &p2))
			{
				qDebug() << "KSquaresIO::saveGame error: invalid line in history";
				file.close();
				return false;
			}
			outStream << "(" << p1.x() << ", " << (sGame->board()->height() - p1.y()) << ") - (" << p2.x() << ", " << (sGame->board()->height() - p2.y()) << ")\n";
		} 
	}
	else if (filename.endsWith(QStringLiteral(".ksq")))
	{
		// save in ksquares format
		// TODO: implement saving as ksq
	}
	else if (filename.endsWith(QStringLiteral(".sc.tex")))
	{
		// save in tex format
		outStream << "\\begin{tikzpicture}\n";
		outStream << "  \\pgfsetlinewidth{1pt}\n";
		int squareIndex = 0;
		for (int y = sGame->board()->height() - 1; y >= 0; y--)
		{
			for (int x = 0; x < sGame->board()->width(); x++)
			{
				if (sGame->board()->squares()[squareIndex] == -1)
					outStream << "  \\draw (" << x << ".5," << y << ".5) circle (6pt);\n";
				squareIndex++;
			}
		}
		int width = sGame->board()->width();
		int height = sGame->board()->height();
		int linesSize = 2 * width * height + width + height;
		for (int i = 0; i < linesSize; i++)
		{
			bool lineDrawn = false;
			for (int j = 0; j < sGame->board()->getLineHistory().size(); j++)
			{
				if (sGame->board()->getLineHistory()[j].line == i)
				{
					lineDrawn = true;
				}
			}
			if (lineDrawn)
			{
				continue;
			}
			QPoint p1;
			QPoint p2;
			if (!sGame->board()->indexToPoints(i, &p1, &p2))
			{
				qDebug() << "KSquaresIO::saveGame error: invalid line in history";
				// TODO: remove unfinished file?
				file.close();
				return false;
			}
			float p1x = p1.x();
			float p1y = p1.y();
			float p2x = p2.x();
			float p2y = p2.y();
			if (sGame->board()->lineDirection(i) == KSquares::VERTICAL)
			{
				p1x -= 0.5;
				p1y -= 0.5;
				p2x += 0.5;
				p2y += 0.5;
				if (p1x < 0)
					p1x = 0;
				else
					p1x += 0.2;
				if (p2x > width)
					p2x = width;
				else
					p2x -= 0.2;
			}
			else
			{
				p1x += 0.5;
				p1y += 0.5;
				p2x -= 0.5;
				p2y -= 0.5;
				/*
				if (p2y < 0)
					p2y = 0;
				else
					p2y += 0.3;
				if (p1y < 0)
					p1y = 0;
				else
					p1y -= 0.3;
				*/
				
				if (p1y > height || p2y > height)
				{
					p1y = height;
					p2y = height - 0.3f;
				}
				else if ( p1y < 0 || p2y < 0)
				{
					p1y = 0;
					p2y = 0.3;
				}
				else
				{
					p1y -= 0.2;
					p2y += 0.2;
				}
				/*
				if (p2y > height)
					p2y = height;
				else
					p2y += 0.3;
				*/
			}
			outStream << "  \\draw (" << p1x << ", " << p1y << ") -- (" << p2x << ", " << p2y << ");\n";
		}
		for (int i = 0; i < sGame->board()->squares().size(); i++)
		{
			if (sGame->board()->squares()[i] >= 0)
			{
				outStream << "  \\node[draw] at (" << ( i % sGame->board()->width() ) << ".5," << ( sGame->board()->height() - i / sGame->board()->width() - 1 ) << ".5) {" << (char)(sGame->board()->squares()[i] + 'A') << "};\n";
			}
		}
		outStream << "\\end{tikzpicture}\n";
	}
	else if (filename.endsWith(QStringLiteral(".tex")))
	{
		// save in tex format
		outStream << "\\begin{tikzpicture}\n";
		outStream << "  \\pgfsetlinewidth{1pt}\n";
		for (int x = 0; x <= sGame->board()->width(); x++)
		{
			for (int y = 0; y <= sGame->board()->height(); y++)
			{
				//outStream << "  \\pgfcircle[fill]{\\pgfxy(" << x << "," << y << ")}{3pt}\n";
				outStream << "  \\fill (" << x << "," << y << ") circle (3pt);\n";
			}
		}
		for (int i = 0; i < sGame->board()->getLineHistory().size(); i++)
		{
			QPoint p1;
			QPoint p2;
			if (!sGame->board()->indexToPoints(sGame->board()->getLineHistory()[i].line, &p1, &p2))
			{
				qDebug() << "KSquaresIO::saveGame error: invalid line in history";
				file.close();
				return false;
			}
			//outStream << "  \\pgfxyline(" << p1.x() << ", " << p1.y() << ")(" << p2.x() << ", " << p2.y() << ")\n";
			outStream << "  \\draw (" << p1.x() << ", " << p1.y() << ") -- (" << p2.x() << ", " << p2.y() << ");\n";
		}
		for (int i = 0; i < sGame->board()->squares().size(); i++)
		{
			if (sGame->board()->squares()[i] >= 0)
			{
				//outStream << "  \\pgfputat{\\pgfxy(" << ( i % sGame->board()->width() ) << ".5," << ( sGame->board()->height() - i / sGame->board()->width() - 1 ) << ".5)}{\\pgfbox[center,center]{{\\LARGE " << (char)(sGame->board()->squares()[i] + 'A') << "}}}\n";
				outStream << "  \\node[align=center] at (" << ( i % sGame->board()->width() ) << ".5," << ( sGame->board()->height() - i / sGame->board()->width() - 1 ) << ".5) { {\\LARGE " << (char)(sGame->board()->squares()[i] + 'A') << "} };\n";
			}
		}
		outStream << "\\end{tikzpicture}\n";
	}
	else if (filename.endsWith(QStringLiteral(".png")))
	{
		QPixmap img(10,19);
	}
	else
	{
		// TODO: error message: unknown filetype
	}
  file.close();

  return true;
}

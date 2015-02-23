/***************************************************************************
 *   Copyright (C) 2015 by Tom Vincent Peters   <kde@vincent-peters.de>    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "dbgame.h"

#include <QString>
#include <QDataStream>
#include <QByteArray>
#include <QDateTime>
#include <QCoreApplication>

#include "ksquaresio.h"

// generated
#include "externalaipath.h"

Dabble::Dabble(int newPlayerId, int newMaxPlayerId, int newWidth, int newHeight, int newLevel, int thinkTime) : KSquaresAi(newWidth, newHeight)
{
	timeout = thinkTime;
	playerId = newPlayerId;
	isTainted = false;
	dabbleStdOutStream.setString(&dabbleStdOut);
	dabbleStdErrStream.setString(&dabbleStdErr);
	
	timeoutTimer = QElapsedTimer();
	
	dabbleExited = true;
	dabble = NULL;
	dabbleNohash = newLevel != 0;
}

Dabble::~Dabble()
{
	teardownProcess();
}

void Dabble::initProcess()
{
	kDebug() << "initProcess()";
	if (dabble)
	{
		kDebug() << "WARNING: dabble already running!!! tearing it down...";
		teardownProcess();
		if (dabble)
		{
			kDebug() << "ERROR: dabble still running, teardown didn't work! not starting dabble";
			return;
		}
	}
	QString wineExecutable = "wine";
	QString dabbleExecutable = QString(EXTERNALAIPATH) + (dabbleNohash ? "/dabble/dabble_nohash.exe" : "/dabble/dabble.exe");
	QStringList dabbleArguments;
	dabbleArguments << dabbleExecutable << "/tmp/input.dabble.dbl" << QString::number(timeout / 1000);
	dabble = new QProcess();
	connect(dabble, SIGNAL(error(QProcess::ProcessError)), this, SLOT(processError(QProcess::ProcessError)));
	connect(dabble, SIGNAL(stateChanged(QProcess::ProcessState)), this, SLOT(processStateChanged(QProcess::ProcessState)));
	connect(dabble, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(processFinished(int, QProcess::ExitStatus)));
	connect(dabble, SIGNAL(readyReadStandardError()), this, SLOT(processReadyReadStandardError()));
	connect(dabble, SIGNAL(readyReadStandardOutput()), this, SLOT(processReadyReadStandardOutput()));
	kDebug() << "starting dabble: " << wineExecutable << ", args: " << dabbleArguments;
	dabble->start(wineExecutable, dabbleArguments);
	dabble->setReadChannel(QProcess::StandardOutput);
	if (!dabble->waitForStarted())
	{
		kDebug() << "ERROR: starting dabble failed!";
	}
	dabbleExited = false;
}

void Dabble::teardownProcess()
{
	kDebug() << "teardownProcess()";
	if (dabble!=NULL)
	{
		/*
		disconnect(dabble, SIGNAL(error(QProcess::ProcessError)), this, SLOT(processError(QProcess::ProcessError)));
		disconnect(dabble, SIGNAL(stateChanged(QProcess::ProcessState)), this, SLOT(processStateChanged(QProcess::ProcessState)));
		disconnect(dabble, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(processFinished(int, QProcess::ExitStatus)));
		disconnect(dabble, SIGNAL(readyReadStandardError()), this, SLOT(processReadyReadStandardError()));
		disconnect(dabble, SIGNAL(readyReadStandardOutput()), this, SLOT(processReadyReadStandardOutput()));
		*/
		if (dabble->state() != QProcess::NotRunning)
		{
			kDebug() << "trying to kill dabble process";
			dabble->kill();
			dabble->terminate();
			if (dabble->waitForFinished())
				kDebug() << "killed dabble";
			else
				kDebug() << "killing dabble failed!";
		}
		delete dabble;
		dabble = NULL;
	}
}


void Dabble::processError(const QProcess::ProcessError &error)
{
	kDebug() << "Got error signal from dabble!";
	QString info = "";
	switch (error)
	{
		case QProcess::FailedToStart: info = "The process failed to start. Either the invoked program is missing, or you may have insufficient permissions to invoke the program."; break;
		case QProcess::Crashed: info = "The process crashed some time after starting successfully."; break;
		case QProcess::Timedout: info = "The last waitFor...() function timed out. The state of QProcess is unchanged, and you can try calling waitFor...() again."; break;
		case QProcess::WriteError: info = "An error occurred when attempting to write to the process. For example, the process may not be running, or it may have closed its input channel."; break;
		case QProcess::ReadError: info = "An error occurred when attempting to read from the process. For example, the process may not be running."; break;
		case QProcess::UnknownError: info = "An unknown error occurred. This is the default return value of error()."; break;
	}
	kDebug() << "****************************************************************";
	kDebug() << "***                      DABBLE ERROR                        ***";
	kDebug() << "****************************************************************";
	kDebug() << "dabble error: " << info;
}

void Dabble::processStateChanged(const QProcess::ProcessState &newState)
{
	kDebug() << "processStateChanged!";
	kDebug() << "****************************************************************";
	kDebug() << "***                  DABBLE STATE CHANGED                    ***";
	kDebug() << "****************************************************************";
	QString state = "";
	switch (newState)
	{
		case QProcess::NotRunning: state = "NotRunning"; break;
		case QProcess::Starting: state = "Starting"; break;
		case QProcess::Running: state = "Running"; break;
	}
	kDebug() << "dabble state: " << state;
}

void Dabble::processFinished(const int &exitCode, const QProcess::ExitStatus &exitStatus)
{
	kDebug() << "processFinished!";
	kDebug() << "dabble exit code: " << exitCode;
	kDebug() << "dabble exit status: " << (exitStatus == QProcess::NormalExit ? "normal" : "crash");
	
	dabbleExited = true;
}

void Dabble::processReadyReadStandardError()
{
	kDebug() << "processReadyReadStandardError!";
	dabble->setReadChannel(QProcess::StandardError);
	QByteArray dabbleStdErrTmp = dabble->readAll();
	kDebug() << dabbleStdErrTmp;
	dabbleStdErrStream << dabbleStdErrTmp;
	dabbleStdErrStream.flush();
}

void Dabble::processReadyReadStandardOutput()
{
	kDebug() << "processReadyReadStandardOutput!";
	dabble->setReadChannel(QProcess::StandardOutput);
	QByteArray dabbleStdOutTmp = dabble->readAll();
	//kDebug() << "dabble stdout: " << QString(dabbleStdOutTmp);
	dabbleStdOutStream << dabbleStdOutTmp;
}


int Dabble::randomMove(const QList<bool> &lines)
{
	kDebug() << "WARNING: returning random move that was not generated by dabble";
	isTainted = true;
	QList<int> freeLines;
	for (int i = 0; i < lines.size(); i++)
	{
		if (!lines[i])
			freeLines.append(i);
	}
	return freeLines.at(qrand() % freeLines.size());
}

int Dabble::chooseLine(const QList<bool> &newLines, const QList<int> &newSquareOwners, const QList<Board::Move> &lineHistory)
{
	kDebug() << "dabble choose line...";
	
	if (moveQueue.size() > 0)
	{
		kDebug() << "found move in queue, not calling dabble...";
		int dblMove = moveQueue.dequeue();
		return dblMove;
	}
	
	QCoreApplication::processEvents();
	
	// TODO: create savegame
	KSquaresGame ksqGame;
	QList<int> isHumanList;
	for (int i = 0; i < 2; i++)
		isHumanList.append(i != playerId ? 1 : 0);
	ksqGame.createGame(KSquaresGame::createPlayers(2, isHumanList), width, height);
	for (int i = 0; i < lineHistory.size(); i++)
		ksqGame.addLineToIndex(lineHistory[i].line);
	KSquaresIO::saveGame("/tmp/input.dabble.dbl", &ksqGame);
	initProcess();
	
	while (!dabbleExited)
	{
		QCoreApplication::processEvents();
	}
	
	teardownProcess();
	
	// TODO: read dabble.log
	QString line;
	int moveCnt = 0;
	
	do
	{
		line = dabbleStdOutStream.readLine();
		if (line.startsWith("DABBLE MOVE: "))
		{
			//kDebug() << "found a move by dabble...";
			moveCnt++;
			if (moveCnt > lineHistory.size())
			{
				QRegExp moveRegex("\\(([\\d]+), ([\\d]+)\\) - \\(([\\d]+), ([\\d]+)\\)");
				int pos = moveRegex.indexIn(line);
				if (pos < 0)
				{
					kDebug() << "sth went wrong when parsing dabble move in line: " << line;
				}
				else
				{
					QPoint p1(moveRegex.cap(1).toInt(), moveRegex.cap(2).toInt());
					QPoint p2(moveRegex.cap(3).toInt(), moveRegex.cap(4).toInt());
					kDebug() << "parsed new dabble move: " << p1 << ", " << p2;
					QPair<QPoint, QPoint> dnbPoints = Board::coinsToPoints(p1, p2, width, height);
					kDebug() << "dabble move converted to dots and boxes coordinates: " << dnbPoints.first << ", " << dnbPoints.second;
					int lineIdx = Board::pointsToIndex(dnbPoints.first, dnbPoints.second, width, height);
					kDebug() << "line index of dabble move: " << lineIdx;
					moveQueue.enqueue(lineIdx);
				}
			}
		}
	} while (!line.isNull());
	
	if (moveQueue.size() <= 0)
	{
		kDebug() << "ERROR: didn't parse any dabble move! is dabble setup correctly? take a look at aux/dabble/README";
		return randomMove(newLines);
	}
	
	int dblMove = moveQueue.dequeue();
	return dblMove;
}

#include "dbgame.moc"
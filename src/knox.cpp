/***************************************************************************
 *   Copyright (C) 2015 by Tom Vincent Peters   <kde@vincent-peters.de>    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "knox.h"

#include <QString>
#include <QDataStream>
#include <QByteArray>
#include <QDateTime>
#include <QCoreApplication>

// generated
#include "externalaipath.h"

Knox::Knox(int newPlayerId, int newMaxPlayerId, int newWidth, int newHeight, int newLevel, int thinkTime) : KSquaresAi(newWidth, newHeight)
{
	timeout = thinkTime;
	playerId = newPlayerId;
	tainted = false;
	knoxStdOutStream.setString(&knoxStdOut);
	knoxStdErrStream.setString(&knoxStdErr);
	opponentNameEntered = false;
	opponentName = "";
	goFirstEntered = false;
	enterMove = false;
	linesSentCnt = 0;
	knoxMoved = false;
	lastKnoxMove = "";
	lastKnoxMoveOffset = 0;
	
	QString knoxExecutable = QString(EXTERNALAIPATH) + "/knox/knox";
	kDebug() << "starting knox: " << knoxExecutable;
	knox = new QProcess();
	connect(knox, SIGNAL(error(QProcess::ProcessError)), this, SLOT(processError(QProcess::ProcessError)));
	connect(knox, SIGNAL(stateChanged(QProcess::ProcessState)), this, SLOT(processStateChanged(QProcess::ProcessState)));
	connect(knox, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(processFinished(int, QProcess::ExitStatus)));
	connect(knox, SIGNAL(readyReadStandardError()), this, SLOT(processReadyReadStandardError()));
	connect(knox, SIGNAL(readyReadStandardOutput()), this, SLOT(processReadyReadStandardOutput()));
	knox->start(knoxExecutable);
	knox->setReadChannel(QProcess::StandardOutput);
	if (!knox->waitForStarted())
	{
		kDebug() << "ERROR: starting knox failed!";
	}
	if (!knox->waitForReadyRead())
	{
		kDebug() << "Waiting for ready read failed";
	}
	
	timeoutTimer = QElapsedTimer();
}

Knox::~Knox()
{
	if (knox!=NULL)
	{
		disconnect(knox, SIGNAL(error(QProcess::ProcessError)), this, SLOT(processError(QProcess::ProcessError)));
		disconnect(knox, SIGNAL(stateChanged(QProcess::ProcessState)), this, SLOT(processStateChanged(QProcess::ProcessState)));
		disconnect(knox, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(processFinished(int, QProcess::ExitStatus)));
		disconnect(knox, SIGNAL(readyReadStandardError()), this, SLOT(processReadyReadStandardError()));
		disconnect(knox, SIGNAL(readyReadStandardOutput()), this, SLOT(processReadyReadStandardOutput()));
		if (knox->state() != QProcess::NotRunning)
		{
			kDebug() << "trying to kill knox process";
			knox->kill();
			knox->terminate();
			if (knox->waitForFinished())
				kDebug() << "killed knox";
			else
				kDebug() << "killing knox failed!";
		}
		delete knox;
	}
}


void Knox::processError(const QProcess::ProcessError &error)
{
	kDebug() << "Got error signal from knox!";
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
	kDebug() << "knox error: " << info;
}

void Knox::processStateChanged(const QProcess::ProcessState &newState)
{
	kDebug() << "processStateChanged!";
	QString state = "";
	switch (newState)
	{
		case QProcess::NotRunning: state = "NotRunning"; break;
		case QProcess::Starting: state = "Starting"; break;
		case QProcess::Running: state = "Running"; break;
	}
	kDebug() << "knox state: " << state;
}

void Knox::processFinished(const int &exitCode, const QProcess::ExitStatus &exitStatus)
{
	kDebug() << "processFinished!";
	kDebug() << "knox exit code: " << exitCode;
	kDebug() << "knox exit status: " << (exitStatus == QProcess::NormalExit ? "normal" : "crash");
}

void Knox::processReadyReadStandardError()
{
	kDebug() << "processReadyReadStandardError!";
	knox->setReadChannel(QProcess::StandardError);
	QByteArray knoxStdErrTmp = knox->readAll();
	kDebug() << knoxStdErrTmp;
	knoxStdErrStream << knoxStdErrTmp;
	knoxStdErrStream.flush();
}

void Knox::processReadyReadStandardOutput()
{
	kDebug() << "processReadyReadStandardOutput!";
	knox->setReadChannel(QProcess::StandardOutput);
	QByteArray knoxStdOutTmp = knox->readAll();
	kDebug() << knoxStdOutTmp;
	knoxStdOutStream << knoxStdOutTmp;
	knoxStdOutStream.flush();
	if (!opponentNameEntered)
	{
		if (knoxStdOut.contains("Enter Opponent's name: "))
		{
			uint stamp = QDateTime::currentDateTime().toTime_t();
			opponentName = "ksquares-" + QString::number(stamp) + "\n";
			knox->write(opponentName.toAscii());
			if (!knox->waitForBytesWritten())
			{
				kDebug() << "entering opponent name might have failed!";
			}
			
			kDebug() << "entered opponent name: " << opponentName;
			opponentNameEntered = true;
		}
	}
	if (opponentNameEntered && !goFirstEntered)
	{
		if (knoxStdOut.contains("Do You want to go first? (Y or N)"))
		{
			QString goFirst = playerId == 1 ? "Y\n" : "N\n";
			knox->write(goFirst.toAscii());
			if (!knox->waitForBytesWritten())
			{
				kDebug() << "entering answer to go first question failed!";
			}
			
			kDebug() << "going first: " << goFirst;
			goFirstEntered = true;
		}
	}
	
	if (knoxStdOut.endsWith("Enter move (e.g. a1-b1) "))
	{
		enterMove = true;
		kDebug() << "knox accepts move";
	}
	else
	{
		enterMove = false;
	}
	
	int movePos = knoxStdOut.indexOf("My move: ", lastKnoxMoveOffset);
	if (movePos >= 0)
	{
		lastKnoxMoveOffset = movePos+9;
		knoxMoved = true;
		lastKnoxMove = knoxStdOut.mid(lastKnoxMoveOffset, knoxStdOut.indexOf("\n", lastKnoxMoveOffset)-lastKnoxMoveOffset-9);
		kDebug() << "knox made a move: " << lastKnoxMove;
	}
}


int Knox::randomMove(const QList<bool> &lines)
{
	kDebug() << "WARNING: returning random move that was not generated by knox";
	tainted = true;
	QList<int> freeLines;
	for (int i = 0; i < lines.size(); i++)
	{
		if (!lines[i])
			freeLines.append(i);
	}
	return freeLines.at(qrand() % freeLines.size());
}

int Knox::chooseLine(const QList<bool> &newLines, const QList<int> &newSquareOwners, const QList<Board::Move> &lineHistory)
{
	QCoreApplication::processEvents();
	
	while (!opponentNameEntered || !goFirstEntered)
	{
		kDebug() << "waiting for knox setup to complete...";
		QCoreApplication::processEvents();
	}
	
	knoxMoved = false;
	lastKnoxMove = "";
	
	// desperation...
	connect(knox, SIGNAL(readyReadStandardOutput()), this, SLOT(processReadyReadStandardOutput()));
	
	// send new lines to knox
	while (linesSentCnt < lineHistory.size())
	{
		if (!enterMove)
		{
			QCoreApplication::processEvents();
			continue;
		}
		QPoint p1;
		QPoint p2;
		Board::indexToPoints(lineHistory[linesSentCnt].line, &p1, &p2, width, height);
		kDebug() << "line " << lineHistory[linesSentCnt].line << " = " << p1 << " - " << p2;
		QString knoxMove = "";
		knoxMove += p1.x() + 'a';
		knoxMove += QString::number(p1.y() + 1);
		knoxMove += "-";
		knoxMove += p2.x() + 'a';
		knoxMove += QString::number(p2.y() + 1);
		kDebug() << "converted to knox move: " << knoxMove;
		knoxMove += "\n";
		knoxStdOutStream << knoxMove;
		enterMove = false;
		
		linesSentCnt++;
		knox->write(knoxMove.toAscii());
		knox->write("\n");
		if (!knox->waitForBytesWritten())
		{
			kDebug() << "sending move might have failed!";
		}
	}
	
	timeoutTimer.restart();
	while (!knoxMoved)
	{
		if (knox->state() != QProcess::Running)
		{
			kDebug() << "ERROR: knox process is not running...";
			return randomMove(newLines);
		}
		//kDebug() << "enter move = " << enterMove;
		if (timeoutTimer.hasExpired(timeout))
		{
			kDebug() << "reached knox timeout, aborting...";
			kDebug() << "ERROR: this game is tainted!";
			kDebug() << "knox output" << knoxStdOut;
			return randomMove(newLines);
			//processReadyReadStandardOutput();
		}
		//kDebug() << "waiting for knox to make a move";
		QCoreApplication::processEvents();
	}
	
	QPoint p1(lastKnoxMove.at(0).toAscii()-'a', height - (lastKnoxMove.at(1).toAscii()-'0'-1) );
	QPoint p2(lastKnoxMove.at(3).toAscii()-'a', height - (lastKnoxMove.at(4).toAscii()-'0'-1) );
	kDebug() << "knox made move at: " << p1 << ", " << p2;
	int line = Board::pointsToIndex(p1, p2, width, height);
	linesSentCnt++;
	
	return line;
}

#include "knox.moc"
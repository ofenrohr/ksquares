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
#include <QElapsedTimer>

// generated
#include "externalaipath.h"

Knox::Knox(int newPlayerId, int newMaxPlayerId, int newWidth, int newHeight, int newLevel, int thinkTime) : KSquaresAi(newWidth, newHeight)
{
	timeout = thinkTime;
	playerId = newPlayerId;
	isTainted = false;
	knoxStdOutStream.setString(&knoxStdOut);
	knoxStdErrStream.setString(&knoxStdErr);
	linesSentCnt = 0;
	lastKnoxMoveOffset = 0;
	knoxMoveQueue.clear();
	knoxStartedCnt = 0;
	opponentName = "";
	goFirstEntered = false;

	knox = NULL;
	
	setupProcess();
	
	timeoutTimer = QElapsedTimer();
	
	if (newWidth > 9 || newHeight > 9)
	{
		kDebug() << "****************************************************************";
		kDebug() << "***                           ERROR                          ***";
		kDebug() << "****************************************************************";
		kDebug() << "knox only works with up to 9x9 boxes games!";
	}
}

Knox::~Knox()
{
	destroyProcess();
}


void Knox::setupProcess()
{
	knoxCrashed = false;
	opponentNameEntered = false;
	//opponentName = "";
	//goFirstEntered = false;
	enterMove = false;
	knoxStdOut = "";
	knoxStdErr = "";
	lastKnoxMoveOffset = 0;
	knoxMoveQueue.clear();
	if (linesSentCnt > 0) // looks like knox crashed before, resend last line
		linesSentCnt--;
	
	knoxStartedCnt++;
	
	QString knoxExecutable = QString(EXTERNALAIPATH) + "/knox/knox";
	QStringList knoxArguments;
	knoxArguments << QString::number(timeout / 1000) << QString::number(width) << QString::number(height);
	if (knox != NULL)
		destroyProcess();
	knox = new QProcess();
	qRegisterMetaType<QProcess::ProcessError>("QProcess::ProcessError");
	qRegisterMetaType<QProcess::ProcessState>("QProcess::ProcessState");
	qRegisterMetaType<QProcess::ExitStatus>("QProcess::ExitStatus");
	connect(knox, SIGNAL(error(QProcess::ProcessError)), this, SLOT(processError(QProcess::ProcessError)));
	connect(knox, SIGNAL(stateChanged(QProcess::ProcessState)), this, SLOT(processStateChanged(QProcess::ProcessState)));
	connect(knox, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(processFinished(int, QProcess::ExitStatus)));
	connect(knox, SIGNAL(readyReadStandardError()), this, SLOT(processReadyReadStandardError()));
	connect(knox, SIGNAL(readyReadStandardOutput()), this, SLOT(processReadyReadStandardOutput()));
	kDebug() << "starting knox: " << knoxExecutable << ", ARGS: " << knoxArguments;
	knox->start(knoxExecutable, knoxArguments);
	knox->setReadChannel(QProcess::StandardOutput);
	if (!knox->waitForStarted())
	{
		kDebug() << "ERROR: starting knox failed!";
	}
	if (!knox->waitForReadyRead())
	{
		kDebug() << "Waiting for ready read failed";
	}
}

void Knox::destroyProcess()
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
	knox = NULL;
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
	kDebug() << "****************************************************************";
	kDebug() << "***                        KNOX ERROR                        ***";
	kDebug() << "****************************************************************";
	kDebug() << "knox error: " << info;
	kDebug() << "entered opponent name: " << opponentName;
	knoxCrashed = true;
}

void Knox::processStateChanged(const QProcess::ProcessState &newState)
{
	kDebug() << "processStateChanged!";
	kDebug() << "****************************************************************";
	kDebug() << "***                    KNOX STATE CHANGED                    ***";
	kDebug() << "****************************************************************";
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
	kDebug() << "knox stdout: " << QString(knoxStdOutTmp);
	knoxStdOutStream << knoxStdOutTmp;
	knoxStdOutStream.flush();
	if (!opponentNameEntered)
	{
		if (knoxStdOut.contains("Enter Opponent's name: "))
		{
			uint stamp = QDateTime::currentDateTime().toTime_t();
			if (opponentName.isEmpty())
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
	
	int movePos = -1;
	do
	{
		movePos = knoxStdOut.indexOf("My move: ", lastKnoxMoveOffset);
		if (movePos < 0)
			continue;
		lastKnoxMoveOffset = movePos+9;
		QString mv = knoxStdOut.mid(lastKnoxMoveOffset, 5);
		knoxMoveQueue.enqueue(mv);
		kDebug() << "knox made a move: " << mv;
	} while (movePos >= 0);
}


int Knox::randomMove(const QList<bool> &lines)
{
	kDebug() << "WARNING: returning random move that was not generated by knox";
	isTainted = true;
	QList<int> freeLines;
	for (int i = 0; i < lines.size(); i++)
	{
		if (!lines[i])
			freeLines.append(i);
	}
	lastTurnTime = -1;
	return freeLines.at(qrand() % freeLines.size());
}

int Knox::chooseLine(const QList<bool> &newLines, const QList<int> &newSquareOwners, const QList<Board::Move> &lineHistory)
{
	kDebug() << "knox choose line...";
	QCoreApplication::processEvents();
	
	while (!opponentNameEntered || !goFirstEntered)
	{
		kDebug() << "waiting for knox setup to complete...";
		if (knox->state() != QProcess::Running || knoxCrashed)
		{
			kDebug() << "ERROR: knox process is not running...";
			
			if (knoxStartedCnt < 5)
			{
				kDebug() << "Knox crashed, trying to recover stuff... attempt " << knoxStartedCnt;
				setupProcess();
				return chooseLine(newLines, newSquareOwners, lineHistory);
			}
			return randomMove(newLines);
		}
		if (!knox->waitForReadyRead(1000))
		{
			kDebug() << "ERROR: knox doesn't answer";
		}
		QCoreApplication::processEvents();
	}
	
	QElapsedTimer turnTimer;
	turnTimer.start();
	
	if (knoxMoveQueue.size() <= 0)
	{
		kDebug() << "no knox move in queue";
		// send new lines to knox
		int sentMoveCnt = 0;
		while (linesSentCnt < lineHistory.size())
		{
			if (!enterMove)
			{
				if (knox->state() != QProcess::Running || knoxCrashed)
				{
					kDebug() << "ERROR: knox process is not running...";
					if (knoxStartedCnt < 5)
					{
						kDebug() << "Knox crashed, trying to recover stuff... attempt " << knoxStartedCnt;
						setupProcess();
						return chooseLine(newLines, newSquareOwners, lineHistory);
					}
					return randomMove(newLines);
				}
				if (!knox->waitForReadyRead(1000))
				{
					kDebug() << "ERROR: knox doesn't request move";
				}
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
			sentMoveCnt++;
			knox->write(knoxMove.toAscii());
			if (!knox->waitForBytesWritten(1000))
			{
				kDebug() << "sending move might have failed!";
			}
			knox->waitForReadyRead(1000);
		}
		if (sentMoveCnt == 0)
		{
			kDebug() << "didn't send any move to knox.";
			kDebug() << "knox move queue size: " << knoxMoveQueue.size();
			//kDebug() << "knox output: " << knoxStdOut;
		}
		
		timeoutTimer.restart();
		while (knoxMoveQueue.size() == 0)
		{
			if (knox->state() != QProcess::Running || knoxCrashed)
			{
				kDebug() << "ERROR: knox process is not running...";
				if (knoxStartedCnt < 5)
				{
					kDebug() << "Knox crashed, trying to recover stuff... attempt " << knoxStartedCnt;
					setupProcess();
					return chooseLine(newLines, newSquareOwners, lineHistory);
				}
				return randomMove(newLines);
			}
			//kDebug() << "enter move = " << enterMove;
			//if (timeoutTimer.hasExpired(timeout))
			//{
				//kDebug() << "reached knox timeout, doing nothing...";
			//}
			if (!knox->waitForReadyRead(1000))
			{
				//kDebug() << "ERROR: knox doesn't answer while waiting for move from knox";
			}
			//kDebug() << "waiting for knox to make a move";
			QCoreApplication::processEvents();
		}
	}
	
	QString knoxMv = knoxMoveQueue.dequeue();
	QPoint p1(knoxMv.at(0).toAscii()-'a', height - (knoxMv.at(1).toAscii()-'0'-1) );
	QPoint p2(knoxMv.at(3).toAscii()-'a', height - (knoxMv.at(4).toAscii()-'0'-1) );
	kDebug() << "knox made move at: " << p1 << ", " << p2;
	int line = Board::pointsToIndex(p1, p2, width, height);
	linesSentCnt++;
	
	lastTurnTime = turnTimer.elapsed();
	return line;
}

#include "knox.moc"
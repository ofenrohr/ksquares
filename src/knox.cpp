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
#include <QFile>
#include <QRegExp>

// generated
#include "externalaipath.h"

Knox::Knox(int newPlayerId, int /*newMaxPlayerId*/, int newWidth, int newHeight, int /*newLevel*/, int thinkTime) : KSquaresAi(newWidth, newHeight)
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
	opponentName = QStringLiteral("");
	goFirstEntered = false;
	knoxRecovering = false;

	knox = NULL;
	
	setupProcess();
	
	timeoutTimer = QElapsedTimer();
	
	if (newWidth > 9 || newHeight > 9)
	{
		qDebug() << "****************************************************************";
		qDebug() << "***                           ERROR                          ***";
		qDebug() << "****************************************************************";
		qDebug() << "knox only works with up to 9x9 boxes games!";
	}
	
	crashCnt = 0;
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
	knoxStdOut = QStringLiteral("");
	knoxStdErr = QStringLiteral("");
	lastKnoxMoveOffset = 0;
	knoxMoveQueue.clear();
	knoxRecovering = true;
	if (knoxRecovering) // looks like knox crashed before, analyze what knox knows about the game
	{
		QFile knoxlog(opponentName.trimmed());
		if (knoxlog.exists())
		{
			if (knoxlog.open(QIODevice::ReadOnly))
			{
				QTextStream inStream(&knoxlog);
				QString line;
				int lastTurnInKnoxLog = -1;
				QList<QString> knoxMovesInLog;
				do
				{
					line = inStream.readLine();
					qDebug() << "knoxlog: " << line;
					QRegExp moveRegex(QStringLiteral("^([\\d]+): ([\\w][\\d]-[\\w][\\d])"));
					int pos = moveRegex.indexIn(line);
					if (pos >= 0)
					{
						qDebug() << "knox move in log: " << moveRegex.cap(1) << " -> " << moveRegex.cap(2);
						lastTurnInKnoxLog = moveRegex.cap(1).toInt();
						knoxMovesInLog.append(moveRegex.cap(2));
					}
				} while (!line.isNull());
				if (lastTurnInKnoxLog > 0)
				{
					qDebug() << "knox log contains " << lastTurnInKnoxLog << " moves";
					for (int i = linesSentCnt; i < lastTurnInKnoxLog; i++)
					{
						qDebug() << "found move in knox log that hasn't been made in ksquares: " << knoxMovesInLog.at(i);
						knoxMoveQueue.enqueue(knoxMovesInLog.at(i));
					}
					linesSentCnt = lastTurnInKnoxLog - 1;
				}
				else
					linesSentCnt--;
				knoxlog.close();
			}
			else
				linesSentCnt--;
		}
		else
			linesSentCnt--;
	}
	if (linesSentCnt < 0)
		linesSentCnt = 0;
	
	knoxStartedCnt++;
	
	QString knoxExecutable = QStringLiteral(EXTERNALAIPATH) + QStringLiteral("/knox/knox");
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
	qDebug() << "starting knox: " << knoxExecutable << ", ARGS: " << knoxArguments;
	knox->start(knoxExecutable, knoxArguments);
	knox->setReadChannel(QProcess::StandardOutput);
	if (!knox->waitForStarted())
	{
		qDebug() << "ERROR: starting knox failed!";
	}
	if (!knox->waitForReadyRead())
	{
		qDebug() << "Waiting for ready read failed";
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
			qDebug() << "trying to kill knox process";
			knox->kill();
			knox->terminate();
			if (knox->waitForFinished())
				qDebug() << "killed knox";
			else
				qDebug() << "killing knox failed!";
		}
		delete knox;
	}
	//knox = NULL;
}

void Knox::processError(const QProcess::ProcessError &error)
{
	qDebug() << "Got error signal from knox!";
	QString info;
	switch (error)
	{
		case QProcess::FailedToStart: info = QStringLiteral("The process failed to start. Either the invoked program is missing, or you may have insufficient permissions to invoke the program."); break;
		case QProcess::Crashed: info = QStringLiteral("The process crashed some time after starting successfully."); break;
		case QProcess::Timedout: info = QStringLiteral("The last waitFor...() function timed out. The state of QProcess is unchanged, and you can try calling waitFor...() again."); break;
		case QProcess::WriteError: info = QStringLiteral("An error occurred when attempting to write to the process. For example, the process may not be running, or it may have closed its input channel."); break;
		case QProcess::ReadError: info = QStringLiteral("An error occurred when attempting to read from the process. For example, the process may not be running."); break;
		case QProcess::UnknownError: info = QStringLiteral("An unknown error occurred. This is the default return value of error()."); break;
	}
	qDebug() << "****************************************************************";
	qDebug() << "***                        KNOX ERROR                        ***";
	qDebug() << "****************************************************************";
	qDebug() << "knox error: " << info;
	qDebug() << "entered opponent name: " << opponentName;
	knoxCrashed = true;
	crashCnt ++;
}

void Knox::processStateChanged(const QProcess::ProcessState &newState)
{
	qDebug() << "processStateChanged!";
	qDebug() << "****************************************************************";
	qDebug() << "***                    KNOX STATE CHANGED                    ***";
	qDebug() << "****************************************************************";
	QString state;
	switch (newState)
	{
		case QProcess::NotRunning: state = QStringLiteral("NotRunning"); break;
		case QProcess::Starting: state = QStringLiteral("Starting"); break;
		case QProcess::Running: state = QStringLiteral("Running"); knoxRecovering = false; break;
	}
	qDebug() << "knox state: " << state;
}

void Knox::processFinished(const int &exitCode, const QProcess::ExitStatus &exitStatus)
{
	qDebug() << "processFinished!";
	qDebug() << "knox exit code: " << exitCode;
	qDebug() << "knox exit status: " << (exitStatus == QProcess::NormalExit ? "normal" : "crash");
}

void Knox::processReadyReadStandardError()
{
	qDebug() << "processReadyReadStandardError!";
	knox->setReadChannel(QProcess::StandardError);
	QByteArray knoxStdErrTmp = knox->readAll();
	qDebug() << knoxStdErrTmp;
	knoxStdErrStream << knoxStdErrTmp;
	knoxStdErrStream.flush();
}

void Knox::processReadyReadStandardOutput()
{
	qDebug() << "processReadyReadStandardOutput!";
	knox->setReadChannel(QProcess::StandardOutput);
	QByteArray knoxStdOutTmp;
	if (knox->bytesAvailable() > 0)
		knoxStdOutTmp = knox->readAll();
	qDebug() << "knox stdout: " << QString::fromStdString(knoxStdOutTmp.toStdString());
	knoxStdOutStream << knoxStdOutTmp;
	knoxStdOutStream.flush();
	if (!opponentNameEntered)
	{
		if (knoxStdOut.contains(QStringLiteral("Enter Opponent's name: ")))
		{
			uint stamp = QDateTime::currentDateTime().toTime_t();
			if (opponentName.isEmpty())
				opponentName = QStringLiteral("ksquares-") + QString::number(stamp) + QStringLiteral("\n");
			else
				goFirstEntered = true;
			knox->write(opponentName.toLatin1());
			if (!knox->waitForBytesWritten())
			{
				qDebug() << "entering opponent name might have failed!";
			}
			
			qDebug() << "entered opponent name: " << opponentName;
			opponentNameEntered = true;
		}
	}
	if (opponentNameEntered && !goFirstEntered)
	{
		if (knoxStdOut.contains(QStringLiteral("Do You want to go first? (Y or N)")))
		{
			QString goFirst = playerId == 1 ? QStringLiteral("Y\n") : QStringLiteral("N\n");
			knox->write(goFirst.toLatin1());
			if (!knox->waitForBytesWritten())
			{
				qDebug() << "entering answer to go first question failed!";
			}
			
			qDebug() << "going first: " << goFirst;
			goFirstEntered = true;
		}
	}
	
	if (knoxStdOut.endsWith(QStringLiteral("Enter move (e.g. a1-b1) ")))
	{
		enterMove = true;
		qDebug() << "knox accepts move";
	}
	else
	{
		enterMove = false;
	}
	
	int movePos = -1;
	do
	{
		movePos = knoxStdOut.indexOf(QStringLiteral("My move: "), lastKnoxMoveOffset);
		if (movePos < 0)
			continue;
		lastKnoxMoveOffset = movePos+9;
		QString mv = knoxStdOut.mid(lastKnoxMoveOffset, 5);
		knoxMoveQueue.enqueue(mv);
		qDebug() << "knox made a move: " << mv;
	} while (movePos >= 0);
}


int Knox::randomMove(const QList<bool> &lines)
{
	qDebug() << "WARNING: returning random move that was not generated by knox";
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
	qDebug() << "knox choose line...";
	QCoreApplication::processEvents();
	
	QElapsedTimer startupTimer;
	startupTimer.start();
	
	while (!opponentNameEntered || !goFirstEntered)
	{
		qDebug() << "waiting for knox setup to complete... opponentNameEntered = " << opponentNameEntered << ", goFirstEntered = " << goFirstEntered;
		if (startupTimer.hasExpired(timeout*5))
		{
			qDebug() << "Knox exeeded timeout*5, restarting knox...";
			setupProcess();
			return chooseLine(newLines, newSquareOwners, lineHistory);
		}
		if ((knox->state() != QProcess::Running || knoxCrashed) && !knoxRecovering)
		{
			qDebug() << "ERROR: knox process is not running...";
			
			if (knoxStartedCnt < 50)
			{
				qDebug() << "Knox crashed, trying to recover stuff... attempt " << knoxStartedCnt;
				setupProcess();
				return chooseLine(newLines, newSquareOwners, lineHistory);
			}
			return randomMove(newLines);
		}
		if (!knox->waitForReadyRead(1000))
		{
			qDebug() << "ERROR: knox doesn't answer";
		}
		QCoreApplication::processEvents();
	}
	
	QElapsedTimer turnTimer;
	turnTimer.start();
	
	if (knoxMoveQueue.size() <= 0)
	{
		qDebug() << "no knox move in queue";
		// send new lines to knox
		int sentMoveCnt = 0;
		while (linesSentCnt < lineHistory.size())
		{
			if (!enterMove)
			{
				if (knox->state() != QProcess::Running || knoxCrashed)
				{
					qDebug() << "ERROR: knox process is not running...";
					if (knoxStartedCnt < 5)
					{
						qDebug() << "Knox crashed, trying to recover stuff... attempt " << knoxStartedCnt;
						setupProcess();
						return chooseLine(newLines, newSquareOwners, lineHistory);
					}
					return randomMove(newLines);
				}
				if (!knox->waitForReadyRead(1000))
				{
					qDebug() << "ERROR: knox doesn't request move";
					//qDebug() << "knox stdout: " << knoxStdOut;
					if (turnTimer.hasExpired(timeout*5))
					{
						qDebug() << "Knox exeeded timeout*5, restarting knox...";
						setupProcess();
						return chooseLine(newLines, newSquareOwners, lineHistory);
					}
				}
				QCoreApplication::processEvents();
				continue;
			}
			QPoint p1;
			QPoint p2;
			Board::indexToPoints(lineHistory[linesSentCnt].line, &p1, &p2, width, height);
			qDebug() << "line " << lineHistory[linesSentCnt].line << " = " << p1 << " - " << p2;
			QString knoxMove;
			knoxMove += p1.x() + 'a';
			knoxMove += QString::number(p1.y() + 1);
			knoxMove += QStringLiteral("-");
			knoxMove += p2.x() + 'a';
			knoxMove += QString::number(p2.y() + 1);
			qDebug() << "converted to knox move: " << knoxMove;
			knoxMove += QStringLiteral("\n");
			knoxStdOutStream << knoxMove;
			enterMove = false;
			
			linesSentCnt++;
			sentMoveCnt++;
			knox->write(knoxMove.toLatin1());
			if (!knox->waitForBytesWritten(1000))
			{
				qDebug() << "sending move might have failed!";
			}
			knox->waitForReadyRead(1000);
		}
		if (sentMoveCnt == 0)
		{
			qDebug() << "didn't send any move to knox.";
			qDebug() << "knox move queue size: " << knoxMoveQueue.size();
			//qDebug() << "knox output: " << knoxStdOut;
		}
		
		timeoutTimer.restart();
		while (knoxMoveQueue.size() == 0)
		{
			if (turnTimer.hasExpired(timeout*5))
			{
				qDebug() << "Knox exeeded timeout*5, restarting knox...";
				setupProcess();
				return chooseLine(newLines, newSquareOwners, lineHistory);
			}
			if (knox->state() != QProcess::Running || knoxCrashed)
			{
				qDebug() << "ERROR: knox process is not running...";
				if (knoxStartedCnt < 5)
				{
					qDebug() << "Knox crashed, trying to recover stuff... attempt " << knoxStartedCnt;
					setupProcess();
					return chooseLine(newLines, newSquareOwners, lineHistory);
				}
				return randomMove(newLines);
			}
			//qDebug() << "enter move = " << enterMove;
			//if (timeoutTimer.hasExpired(timeout))
			//{
				//qDebug() << "reached knox timeout, doing nothing...";
			//}
			if (!knox->waitForReadyRead(1000))
			{
				//qDebug() << "ERROR: knox doesn't answer while waiting for move from knox";
			}
			//qDebug() << "waiting for knox to make a move";
			QCoreApplication::processEvents();
		}
	}
	
	QString knoxMv = knoxMoveQueue.dequeue();
	QPoint p1(knoxMv.at(0).toLatin1()-'a', height - (knoxMv.at(1).toLatin1()-'0'-1) );
	QPoint p2(knoxMv.at(3).toLatin1()-'a', height - (knoxMv.at(4).toLatin1()-'0'-1) );
	qDebug() << "knox made move at: " << p1 << ", " << p2;
	int line = Board::pointsToIndex(p1, p2, width, height);
	linesSentCnt++;
	
	lastTurnTime = turnTimer.elapsed();
	return line;
}

//#include "knox.moc"
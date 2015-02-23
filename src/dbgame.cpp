/***************************************************************************
 *   Copyright (C) 2015 by Tom Vincent Peters   <kde@vincent-peters.de>    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "dabble.h"

#include <QString>
#include <QDataStream>
#include <QByteArray>
#include <QDateTime>
#include <QCoreApplication>

// generated
#include "externalaipath.h"

Dabble::Dabble(int newPlayerId, int newMaxPlayerId, int newWidth, int newHeight, int newLevel, int thinkTime) : KSquaresAi(newWidth, newHeight)
{
	timeout = thinkTime;
	playerId = newPlayerId;
	tainted = false;
	dabbleStdOutStream.setString(&dabbleStdOut);
	dabbleStdErrStream.setString(&dabbleStdErr);
	
	
	if (!dabble->waitForReadyRead())
	{
		kDebug() << "Waiting for ready read failed";
	}
	
	timeoutTimer = QElapsedTimer();
}

Dabble::~Dabble()
{
	teardownProcess();
}

void Dabble::initProcess()
{
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
	QString dabbleExecutable = QString(EXTERNALAIPATH) + "/dabble/dabble.exe";
	QStringList dabbleArguments;
	dabbleArguments << "/tmp/dabble_input.dbl" << (thinkTime / 1000);
	kDebug() << "starting dabble: " << dabbleExecutable << ", args: " << dabbleArguments;
	dabble = new QProcess();
	connect(dabble, SIGNAL(error(QProcess::ProcessError)), this, SLOT(processError(QProcess::ProcessError)));
	connect(dabble, SIGNAL(stateChanged(QProcess::ProcessState)), this, SLOT(processStateChanged(QProcess::ProcessState)));
	connect(dabble, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(processFinished(int, QProcess::ExitStatus)));
	connect(dabble, SIGNAL(readyReadStandardError()), this, SLOT(processReadyReadStandardError()));
	connect(dabble, SIGNAL(readyReadStandardOutput()), this, SLOT(processReadyReadStandardOutput()));
	dabble->start(dabbleExecutable, dabbleArguments);
	dabble->setReadChannel(QProcess::StandardOutput);
	if (!dabble->waitForStarted())
	{
		kDebug() << "ERROR: starting dabble failed!";
	}
}

void Dabble::teardownProcess()
{
	if (dabble!=NULL)
	{
		disconnect(dabble, SIGNAL(error(QProcess::ProcessError)), this, SLOT(processError(QProcess::ProcessError)));
		disconnect(dabble, SIGNAL(stateChanged(QProcess::ProcessState)), this, SLOT(processStateChanged(QProcess::ProcessState)));
		disconnect(dabble, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(processFinished(int, QProcess::ExitStatus)));
		disconnect(dabble, SIGNAL(readyReadStandardError()), this, SLOT(processReadyReadStandardError()));
		disconnect(dabble, SIGNAL(readyReadStandardOutput()), this, SLOT(processReadyReadStandardOutput()));
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
	kDebug() << "dabble stdout: " << QString(dabbleStdOutTmp);
	dabbleStdOutStream << dabbleStdOutTmp;
}


int Dabble::randomMove(const QList<bool> &lines)
{
	kDebug() << "WARNING: returning random move that was not generated by dabble";
	tainted = true;
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
	QCoreApplication::processEvents();
	
	initProcess();
	
	while (!dabbleExited)
	{
		QCoreApplication::processEvents();
	}
	
	int line = -1;
	
	
	teardownProcess();
	
	return line;
}

#include "dabble.moc"
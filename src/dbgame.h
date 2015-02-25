/***************************************************************************
 *   Copyright (C) 2015 by Tom Vincent Peters   <kde@vincent-peters.de>    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef DBGAME_H
#define DBGAME_H

#include "aicontroller.h"

#include <QElapsedTimer>
#include <QProcess>
#include <QTextStream>
#include <QQueue>


class Dabble : public QObject, public KSquaresAi
{
	Q_OBJECT
	public:
		Dabble(int newPlayerId, int newMaxPlayerId, int newWidth, int newHeight, int newLevel, int thinkTime = 5000);
		~Dabble();
		
		int chooseLine(const QList<bool> &newLines, const QList<int> &newSquareOwners, const QList<Board::Move> &lineHistory);
		QString getName() { return dabbleNohash ? "dabble-nohash" : "dabble"; }
		virtual bool enabled() { return !dabbleNohash; }
		virtual bool tainted() { return isTainted; }
		virtual long lastMoveTime() { return 0; }
		
		void initProcess();
		void teardownProcess();
		
		bool isTainted;
	
	public slots:
		void processError(const QProcess::ProcessError &error);
		void processStateChanged(const QProcess::ProcessState &newState);
		void processFinished(const int &exitCode, const QProcess::ExitStatus &exitStatus);
		void processReadyReadStandardError();
		void processReadyReadStandardOutput();
		
	private:
		QElapsedTimer timeoutTimer;
		int timeout;
		int playerId;
		
		int randomMove(const QList<bool> &lines);
		
		QProcess *dabble;
		QTextStream dabbleStdErrStream;
		QTextStream dabbleStdOutStream;
		QString dabbleStdErr;
		QString dabbleStdOut;
		
		bool dabbleExited;
		QQueue<int> moveQueue;
		bool dabbleNohash;
};

#endif
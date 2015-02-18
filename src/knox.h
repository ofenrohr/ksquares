/***************************************************************************
 *   Copyright (C) 2015 by Tom Vincent Peters   <kde@vincent-peters.de>    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef KNOX_H
#define KNOX_H

#include "aicontroller.h"

#include <QElapsedTimer>
#include <QProcess>
#include <QTextStream>


class Knox : public QObject, public KSquaresAi
{
	Q_OBJECT
	public:
		Knox(int newPlayerId, int newMaxPlayerId, int newWidth, int newHeight, int newLevel, int thinkTime = 5000);
		~Knox();
		
		int chooseLine(const QList<bool> &newLines, const QList<int> &newSquareOwners, const QList<Board::Move> &lineHistory);
		QString getName() { return "knox"; }
		
		bool tainted;
	
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
		
		//QTcpSocket *socket;
		QProcess *knox;
		QTextStream knoxStdErrStream;
		QTextStream knoxStdOutStream;
		QString knoxStdErr;
		QString knoxStdOut;
		bool opponentNameEntered;
		QString opponentName;
		bool goFirstEntered;
		bool enterMove;
		int linesSentCnt;
		bool knoxMoved;
		QString lastKnoxMove;
		int lastKnoxMoveOffset;
};

#endif
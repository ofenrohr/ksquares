/***************************************************************************
 *   Copyright (C) 2015 by Tom Vincent Peters   <kde@vincent-peters.de>    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef QDAB_H
#define QDAB_H

#include "aicontroller.h"

#include <QElapsedTimer>
#include <QTcpSocket>
#include <QProcess>


class QDab : public QObject, public KSquaresAi
{
	Q_OBJECT
	public:
		QDab(int newPlayerId, int newMaxPlayerId, int newWidth, int newHeight, int newLevel, int thinkTime = 5000);
		~QDab();
		
		int chooseLine(const QList<bool> &newLines, const QList<int> &newSquareOwners, const QList<Board::Move> &lineHistory);
		QString getName() { return QStringLiteral("qdab"); }
		virtual bool enabled() { return false; }
		virtual bool tainted() { return isTainted; }
		virtual long lastMoveTime() { return lastTurnTime; }
		
		bool isTainted;
	
	public slots:
		void processError(const QProcess::ProcessError &error);
		void processStateChanged(const QProcess::ProcessState &newState);
		void processFinished(const int &exitCode, const QProcess::ExitStatus &exitStatus);
		void processReadyReadStandardError();
		void processReadyReadStandardOutput();
		
	private:
		QElapsedTimer qdabTimer;
		int timeout;
		int playerId;
		
		int randomMove(const QList<bool> &lines);
		
		//QTcpSocket *socket;
		QProcess *qdabServer;
		QTextStream qdabStdErrStream;
		QTextStream qdabStdOutStream;
		QString qdabStdErr;
		QString qdabStdOut;
		
		bool qdabServerListening;
		
		long lastTurnTime;
};

#endif
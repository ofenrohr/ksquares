/***************************************************************************
 *   Copyright (C) 2015 by Tom Vincent Peters   <kde@vincent-peters.de>    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "qdab.h"

#include <QString>
#include <QDataStream>
#include <QByteArray>
#include <QDateTime>
#include <QCoreApplication>

//#include <qjson/parser.h>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonParseError>

// generated
#include "externalaipath.h"

QDab::QDab(int newPlayerId, int /*newMaxPlayerId*/, int newWidth, int newHeight, int /*newLevel*/, int thinkTime) : KSquaresAi(newWidth, newHeight)
{
	timeout = thinkTime;
	playerId = newPlayerId;
	turn = playerId;
	isTainted = false;
	qdabServerListening = false;
	qdabStdOutStream.setString(&qdabStdOut);
	qdabStdErrStream.setString(&qdabStdErr);
	
	// start qdab server
	QString qdabWorkingDirectory = QStringLiteral(EXTERNALAIPATH) + QStringLiteral("/qdab");
	QString qdabServerExecutable = QStringLiteral(EXTERNALAIPATH) + QStringLiteral("/qdab/server");
	QStringList qdabServerArguments;
	QProcessEnvironment qdabEnvironment = QProcessEnvironment::systemEnvironment();
	QString libPath;
	if (qdabEnvironment.contains(QStringLiteral("LD_LIBRARY_PATH")))
		libPath = qdabEnvironment.value(QStringLiteral("LD_LIBRARY_PATH")) + QStringLiteral(":");
	libPath.append(qdabWorkingDirectory);
	qdabEnvironment.insert(QStringLiteral("LD_LIBRARY_PATH"), libPath);
	
	qdabServer = new QProcess();
	qRegisterMetaType<QProcess::ProcessError>("QProcess::ProcessError");
	qRegisterMetaType<QProcess::ProcessState>("QProcess::ProcessState");
	qRegisterMetaType<QProcess::ExitStatus>("QProcess::ExitStatus");
	connect(qdabServer, SIGNAL(error(QProcess::ProcessError)), this, SLOT(processError(QProcess::ProcessError)));
	connect(qdabServer, SIGNAL(stateChanged(QProcess::ProcessState)), this, SLOT(processStateChanged(QProcess::ProcessState)));
	connect(qdabServer, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(processFinished(int, QProcess::ExitStatus)));
	connect(qdabServer, SIGNAL(readyReadStandardError()), this, SLOT(processReadyReadStandardError()));
	connect(qdabServer, SIGNAL(readyReadStandardOutput()), this, SLOT(processReadyReadStandardOutput()));
	
	qDebug() << "starting qdabServer: " << qdabServerExecutable << ", ARGS: " << qdabServerArguments;
	qdabServer->setWorkingDirectory(qdabWorkingDirectory);
	qdabServer->setProcessEnvironment(qdabEnvironment);
	qdabServer->start(qdabServerExecutable, qdabServerArguments);
	qdabServer->setReadChannel(QProcess::StandardError);
	
	QCoreApplication::processEvents();
	if (!qdabServer->waitForStarted())
	{
		qDebug() << "ERROR: starting qdabServer failed!";
	}
	if (!qdabServer->waitForReadyRead())
	{
		qDebug() << "Waiting for ready read failed";
	}
	
	if (newWidth != 5 || newHeight != 5)
	{
		qDebug() << "ERROR: qdab only supports 5x5 boards!";
		isTainted = true;
	}
	
	lastTurnTime = -1;
}

QDab::~QDab()
{
	if (qdabServer!=NULL)
	{
		disconnect(qdabServer, SIGNAL(error(QProcess::ProcessError)), this, SLOT(processError(QProcess::ProcessError)));
		disconnect(qdabServer, SIGNAL(stateChanged(QProcess::ProcessState)), this, SLOT(processStateChanged(QProcess::ProcessState)));
		disconnect(qdabServer, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(processFinished(int, QProcess::ExitStatus)));
		disconnect(qdabServer, SIGNAL(readyReadStandardError()), this, SLOT(processReadyReadStandardError()));
		disconnect(qdabServer, SIGNAL(readyReadStandardOutput()), this, SLOT(processReadyReadStandardOutput()));
		if (qdabServer->state() != QProcess::NotRunning)
		{
			qDebug() << "trying to kill qdabServer process";
			qdabServer->kill();
			qdabServer->terminate();
			if (qdabServer->waitForFinished())
				qDebug() << "killed qdabServer";
			else
				qDebug() << "killing qdabServer failed!";
		}
		delete qdabServer;
	}
}


int QDab::randomMove(const QList<bool> &lines)
{
	qDebug() << "WARNING: returning random move that was not generated by qdab";
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

// def num2move(self, value, who, step=-1):
//         ty, x, y = 1, -1, -1
//         if (value&(1<<31)) != 0:
//             ty = 0 # horizon
//         for i in range(5)[::step]:
//             for j in range(6)[::step]:
//                 if (value&1) == 1:
//                     if ty == 0: x, y = j, i
//                     else: x, y = i, j
//                     break
//                 value >>= 1
//             if x != -1:
//                 break
//         return (ty, x, y, who)




int QDab::getMoveFromQueue(const QList<bool> linesList)
{
	qDebug() << "Current move queue: " << moveQueue;
	if (moveQueue.size() <= 0)
	{
		qDebug() << "ERROR: move queue is empty!";
		return -1;
	}
	for (int i = 0; i < moveQueue.size(); i++)
	{
		int line = moveQueue[i];
		QList<int> adjacentSquares = squaresFromLine(line);
		for (int j = 0; j < adjacentSquares.size(); j++)
		{
			int adjacentSquareLines[4];
			linesFromSquare(adjacentSquareLines, adjacentSquares[j]);
			int cnt = 0;
			for (int k = 0; k < 4; k++)
			{
				if (linesList[adjacentSquareLines[k]])
					cnt++;
			}
			if (cnt == 3)
			{
				qDebug() << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~";
				qDebug() << "returning move from queue: " << line;
				qDebug() << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~";
				
				moveQueue.removeAt(i);
				return line;
			}
		}
	}
	qDebug() << "returning last move from queue";
	return moveQueue.takeLast();
}


int QDab::chooseLine(const QList<bool> &newLines, const QList<int> &newSquareOwners, const QList<Board::Move> &/*lineHistory*/)
{
	QCoreApplication::processEvents();
	if (qdabServer->state() != QProcess::Running)
	{
		qDebug() << "WARNING: qdab server not running";
		qdabServer->waitForStarted();
		qDebug() << "wait for started returned, trying to get move";
	}
	if (!qdabServerListening)
	{
		qDebug() << "WARNING: qdab server might not be listening...";
		qdabServer->waitForReadyRead();
	}
	
	if (moveQueue.size() > 0)
	{
		return getMoveFromQueue(newLines);
	}
	
	uint h = 0;
	uint v = 0;
	// send all moves from history
	for (int i = 0; i < newLines.size(); i++)
	{
		if (!newLines[i])
			continue;
		QPoint p1, p2;
		// qdab origin is at bottom right
		// ksquares origin is at top left
		Board::indexToPoints(i, &p1, &p2, 5, 5, false);
		//qDebug() << "dir,x,y: ("<< (p1.x() != p2.x() ? 0 : 1) <<", " << p1.x() << "," << p1.y() << ")";
		if (p1.x() != p2.x())
			v |= (1<<(p1.x()*6+p1.y()));
		else
			h |= (1<<(p1.y()*6+p1.x()));
	}

	int s0 = 0;
	int s1 = 0;
	for (int i = 0; i < newSquareOwners.size(); i++)
	{
		if (newSquareOwners[i] == 0)
			s0++;
		if (newSquareOwners[i] == 1)
			s1++;
	}
	
	// send stuff to qdab server
	const int connectionTimeout = 1000;

	QTcpSocket socket;
	socket.connectToHost(QStringLiteral("0.0.0.0"), 12345);

	if (!socket.waitForConnected(connectionTimeout))
	{
		qDebug() << "error (wait for connected): " << socket.errorString();
		return randomMove(newLines);
	}
	
	uint id = QDateTime::currentDateTime().toTime_t();
	
	QString request =
			"{\"params\": [{\"Timeout\": "+
			QString::number(timeout)+
			", \"Board\": {\"H\": "+
			QString::number(h)+
			", \"S\": ["+
			QString::number(s0)+
			", "+
			QString::number(s1)+
			"], \"Now\": "+
			QString::number(playerId)+
			", \"Turn\": "+
			QString::number(turn)+
			", \"V\": "+
			QString::number(v)+
			"}, \"Algorithm\": \"quctann\"}], \"id\": "+
			QString::number(id)+
			", \"method\": \"Server.MakeMove\"}";
	QByteArray sendBA = request.toLatin1();
	socket.write(sendBA);
	//qDebug() << "request: " << sendBA;
	socket.waitForBytesWritten();
	
	QElapsedTimer turnTimer;
	turnTimer.start();
	
	turn += 2;
	
	// get response from qdab server
	QByteArray responseBA;
	bool done = false;
	while (!done)
	{
		if (socket.state() != QAbstractSocket::ConnectedState)
		{
			qDebug() << "state: " << socket.state();
			break;
		}
		socket.waitForReadyRead(100);
		//qDebug() << ".";
		if (socket.bytesAvailable() > 0)
		{
			responseBA.append(socket.readAll());
		}
		done = responseBA.count('{') > 0 && responseBA.count('{') == responseBA.count('}');
	}
	
	//qDebug() << "response: " << responseBA;
	
	lastTurnTime = turnTimer.elapsed();
	//qDebug() << "qdab turn time: " << lastTurnTime << ", timeout setting: " << timeout;
	
	// parse response from qdab server
    /*
	QJson::Parser jsonParser;
	bool parseOk;
	QVariantMap result = jsonParser.parse(responseBA, &parseOk).toMap();
	*/
	QJsonParseError jsonParseError;
	QJsonDocument json = QJsonDocument::fromJson(responseBA, &jsonParseError);
	if (jsonParseError.error != QJsonParseError::ParseError::NoError)
	{
		qDebug() << "parsing failed! json error: " << jsonParseError.errorString();
		return randomMove(newLines);
	}
	QVariantMap result = json.toVariant().toMap();

	//qDebug() << "parsed json: " << result;
	if (result[QStringLiteral("id")] != id)
	{
		qDebug() << "ids dont match, aborting. original id = " << id << ", recv id = " << result[QStringLiteral("id")];
		return randomMove(newLines);
	}
	if (!result[QStringLiteral("error")].isNull())
	{
		qDebug() << "qdab error: " << result[QStringLiteral("error")];
		return randomMove(newLines);
	}
	QVariantMap resultHV = result[QStringLiteral("result")].toMap();
	unsigned long long rh = resultHV[QStringLiteral("H")].toULongLong();
	unsigned long long rv = resultHV[QStringLiteral("V")].toULongLong();
	//qDebug() << "result hv: " << rh << ", " << rv;
	
	for (int i = 0; i < 30; i++)
	{
		if ( (1<<i) & rh )
		{
			QPoint pa((i % 6), (i / 6));
			QPoint pb(pa.x(), pa.y() + 1);
			int idx = Board::pointsToIndex(pa, pb, 5, 5);
			//qDebug() << "H: i: " << i << ", idx: " << idx << ", pa: " << pa << ", pb: " << pb;
			if (idx >= newLines.size() || idx < 0)
			{
				qDebug() << "error: invalid index from points! idx: " << idx << ", pa: " << pa << ", pb: " << pb;
				//continue;
			}
			else
			{
				if (!newLines[idx])
					moveQueue.append(idx);
				else
					qDebug() << "old line: " << idx << ", pa: " << pa << ", pb: " << pb;
			}
		}
		
		if ( (1<<i) & rv )
		{
			QPoint pa((i/ 6), (i % 6));
			QPoint pb(pa.x() + 1, pa.y());
			int idx = Board::pointsToIndex(pa, pb, 5, 5);
			//qDebug() << "V: i: " << i << ", idx: " << idx << ", pa: " << pa << ", pb: " << pb;
			if (idx >= newLines.size() || idx < 0)
			{
				qDebug() << "error: invalid index from points! idx: " << idx << ", pa: " << pa << ", pb: " << pb;
				//continue;
			}
			else
			{
				if (!newLines[idx])
					moveQueue.append(idx);
				else
					qDebug() << "old line: " << idx << ", pa: " << pa << ", pb: " << pb;
			}
		}
	}

	return getMoveFromQueue(newLines);
}

void QDab::processError(const QProcess::ProcessError &error)
{
	qDebug() << "Got error signal from qdab!";
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
	qDebug() << "***                        QDAB ERROR                        ***";
	qDebug() << "****************************************************************";
	qDebug() << "qdab error: " << info;
}

void QDab::processStateChanged(const QProcess::ProcessState &newState)
{
	qDebug() << "processStateChanged!";
	qDebug() << "****************************************************************";
	qDebug() << "***                    QDAB STATE CHANGED                    ***";
	qDebug() << "****************************************************************";
	QString state;
	switch (newState)
	{
		case QProcess::NotRunning: state = QStringLiteral("NotRunning"); break;
		case QProcess::Starting: state = QStringLiteral("Starting"); break;
		case QProcess::Running: state = QStringLiteral("Running"); break;
	}
	qDebug() << "qdab state: " << state;
}

void QDab::processFinished(const int &exitCode, const QProcess::ExitStatus &exitStatus)
{
	qDebug() << "processFinished!";
	qDebug() << "qdab exit code: " << exitCode;
	qDebug() << "qdab exit status: " << (exitStatus == QProcess::NormalExit ? "normal" : "crash");
}

void QDab::processReadyReadStandardError()
{
	qDebug() << "processReadyReadStandardError!";
	qdabServer->setReadChannel(QProcess::StandardError);
	QByteArray qdabStdErrTmp = qdabServer->readAll();
	qDebug() << qdabStdErrTmp;
	qdabStdErrStream << qdabStdErrTmp;
	qdabStdErrStream.flush();
	//qDebug() << "qdab stderr: " << QString(qdabStdErrTmp);
	if (qdabStdErr.contains(QStringLiteral("Server runing on")))
	{
		qDebug() << "qdab server is listening";
		qdabServerListening = true;
	}
}

void QDab::processReadyReadStandardOutput()
{
	qDebug() << "processReadyReadStandardOutput!";
	qdabServer->setReadChannel(QProcess::StandardOutput);
	QByteArray qdabStdOutTmp = qdabServer->readAll();
	qdabStdOutStream << qdabStdOutTmp;
	qdabStdOutStream.flush();
	//qDebug() << "qdab stdout: " << QString(qdabStdOutTmp);
}

//#include "qdab.moc"

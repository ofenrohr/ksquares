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

#include <qjson/parser.h>

// generated
#include "externalaipath.h"

QDab::QDab(int newPlayerId, int newMaxPlayerId, int newWidth, int newHeight, int newLevel, int thinkTime) : KSquaresAi(newWidth, newHeight)
{
	timeout = thinkTime;
	playerId = newPlayerId;
	isTainted = false;
	
	// start qdab server
	QString qdabServerExecutable = QString(EXTERNALAIPATH) + "/qdab/server";
	QStringList qdabServerArguments;
	qdabServerArguments << QString::number(timeout / 1000) << QString::number(width) << QString::number(height);
	qdabServer = new QProcess();
	connect(qdabServer, SIGNAL(error(QProcess::ProcessError)), this, SLOT(processError(QProcess::ProcessError)));
	connect(qdabServer, SIGNAL(stateChanged(QProcess::ProcessState)), this, SLOT(processStateChanged(QProcess::ProcessState)));
	connect(qdabServer, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(processFinished(int, QProcess::ExitStatus)));
	connect(qdabServer, SIGNAL(readyReadStandardError()), this, SLOT(processReadyReadStandardError()));
	connect(qdabServer, SIGNAL(readyReadStandardOutput()), this, SLOT(processReadyReadStandardOutput()));
	kDebug() << "starting qdabServer: " << qdabServerExecutable << ", ARGS: " << qdabServerArguments;
	qdabServer->start(qdabServerExecutable, qdabServerArguments);
	qdabServer->setReadChannel(QProcess::StandardOutput);
	if (!qdabServer->waitForStarted())
	{
		kDebug() << "ERROR: starting qdabServer failed!";
	}
}

QDab::~QDab()
{
}


int QDab::randomMove(const QList<bool> &lines)
{
	kDebug() << "WARNING: returning random move that was not generated by qdab";
	isTainted = true;
	QList<int> freeLines;
	for (int i = 0; i < lines.size(); i++)
	{
		if (!lines[i])
			freeLines.append(i);
	}
	return freeLines.at(qrand() % freeLines.size());
}

// def num2move(self, value, who, step=-1):
// ty, x, y = 1, -1, -1
// if (value&(1<<31)) != 0:
// 		ty = 0 # horizon
// for i in range(5)[::step]:
// 		for j in range(6)[::step]:
// 				if (value&1) == 1:
// 						if ty == 0: x, y = j, i
// 						else: x, y = i, j
// 						break
// 				value >>= 1
// 		if x != -1:
// 				break
// return (ty, x, y, who)

/*
int QDab::numToMove(int h, int v)
{
	int x = -1, y = -1;
	int val = h != 0 ? h : v;
	for (int i = 0; i < 5; i++)
	{
		for (int j = 0; j < 6; j++)
		{
			if (val&1)
			{
				x = h != 0 ? i : j;
				y = h != 0 ? j : i;
			}
			val >>= 1;
		}
		if (x != -1)
			break;
	}
	return 
*/

// self.dab.thinking = True
// self.dab.queue_draw()
// if self.dab.first == 0:
// 		s0, s1 = self.dab.human, self.dab.robot
// else:
// 		s1, s0 = self.dab.human, self.dab.robot
// now = 0
// if self.dab.who != self.dab.first:
// 		now = 1

// #algorithm = "alphabeta"
// #algorithm = "uct"
// #algorithm = "uctann"
// #algorithm = "quct"
// algorithm = "quctann"
// timeout = int(10 + 60 * self.dab.timeout_offset) * 1000
// s = socket.create_connection(("0.0.0.0", 12345))
// arg = {"id": int(time.time()), "method": "Server.MakeMove",
// 				"params": [{"Algorithm": algorithm,
// 										"Board": {"H": h, "V": v, "S": [s0, s1], "Now": now, "Turn": self.dab.turn},
// 									"Timeout": timeout}]}
// data = simplejson.dumps(arg).encode()
// print "send: " + data
// s.sendall(data)
// data = s.recv(1024).decode()
// s.close()
// print "recv: " + data
// res = simplejson.loads(data)
// ms = (res["result"]["H"], res["result"]["V"])
// moves = []
// for i in range(2):
// 		for n in range(30):
// 				if ((1<<n)&ms[i]) != 0:
// 						moves.append(self.dab.num2move(((1<<n)|(i<<31)), 1, 1))
// while len(moves) > 1:
// 		for m in moves:
// 				if not self.dab.change(m):
// 						self.dab.move(m)
// 						moves.remove(m)
// 						break
// self.dab.move(moves[0])
// moves.remove(moves[0])
// self.dab.thinking = False
// self.dab.queue_draw()

int QDab::chooseLine(const QList<bool> &newLines, const QList<int> &newSquareOwners, const QList<Board::Move> &lineHistory)
{
// h, v = 0, 0
// for move in self.dab.record:
// 		x, y = move[1], move[2]
// 		if move[0] == 0:
// 				v |= (1<<(y*6+x))
// 		else:
// 				h |= (1<<(x*6+y))
	uint h = 0;
	uint v = 0;
	for (int i = 0; i < newLines.size(); i++)
	{
		if (!newLines[i])
			continue;
		QPoint p1, p2;
		// qdab origin is at bottom right
		// ksquares origin is at top left
		Board::indexToPoints(i, &p1, &p2, 5, 5, false);
// 		p1.setX(5 - p1.x());
// 		p2.setX(5 - p2.x());
		kDebug() << "dir,x,y: ("<< (p1.x() != p2.x() ? 0 : 1) <<", " << p1.x() << "," << p1.y() << ")";
		if (p1.x() != p2.x())
			v |= (1<<(p1.x()*6+p1.y()));
		else
			h |= (1<<(p1.y()*6+p1.x()));
	}
	
	int turn = 1;
	int tmpPlr = 0;
	for (int i = 0; i < lineHistory.size(); i++)
	{
		if (tmpPlr != lineHistory[i].player)
		{
			tmpPlr = lineHistory[i].player;
			turn++;
		}
	}
	
	// send stuff to qdab server
	const int connectionTimeout = 1000;

	QTcpSocket socket;
	socket.connectToHost("0.0.0.0", 12345);

	if (!socket.waitForConnected(connectionTimeout))
	{
		kDebug() << "error (wait for connected): " << socket.errorString();
		return randomMove(newLines);
	}
	
	uint id = QDateTime::currentDateTime().toTime_t();
	
	QString request = "{\"params\": [{\"Timeout\": "+QString::number(timeout)+", \"Board\": {\"H\": "+QString::number(h)+", \"S\": [0, 0], \"Now\": "+QString::number(playerId)+", \"Turn\": "+QString::number(turn)+", \"V\": "+QString::number(v)+"}, \"Algorithm\": \"quctann\"}], \"id\": "+QString::number(id)+", \"method\": \"Server.MakeMove\"}";
	QByteArray sendBA = request.toAscii();
	socket.write(sendBA);
	kDebug() << "request: " << sendBA;
	socket.waitForBytesWritten();
	
	// get response from qdab server
	QByteArray responseBA;
	bool done = false;
	while (!done)
	{
		if (socket.state() != QAbstractSocket::ConnectedState)
		{
			kDebug() << "state: " << socket.state();
			break;
		}
		socket.waitForReadyRead(100);
		//kDebug() << ".";
		if (socket.bytesAvailable() > 0)
		{
			responseBA.append(socket.readAll());
		}
		done = responseBA.count('{') > 0 && responseBA.count('{') == responseBA.count('}');
	}
	
	kDebug() << "response: " << responseBA;
	
	// parse response from qdab server
	QJson::Parser jsonParser;
	bool parseOk;
	QVariantMap result = jsonParser.parse(responseBA, &parseOk).toMap();
	
	if (!parseOk)
	{
		kDebug() << "parsing failed! json error: " << jsonParser.errorString();
		return randomMove(newLines);
	}
	
	kDebug() << "parsed json: " << result;
	if (result["id"] != id)
	{
		kDebug() << "ids dont match, aborting. original id = " << id << ", recv id = " << result["id"];
		return randomMove(newLines);
	}
	if (!result["error"].isNull())
	{
		kDebug() << "qdab error: " << result["error"];
		return randomMove(newLines);
	}
	QVariantMap resultHV = result["result"].toMap();
	unsigned long long rh = resultHV["H"].toULongLong();
	unsigned long long rv = resultHV["V"].toULongLong();
	kDebug() << "result hv: " << rh << ", " << rv;
	
	for (int i = 0; i <= 30; i++)
	{
		if ( (1<<i) & rh )
		{
			QPoint pa((i % 6), (i / 6));
			QPoint pb(pa.x(), pa.y() + 1);
			int idx = Board::pointsToIndex(pa, pb, 5, 5);
			kDebug() << "H: i: " << i << ", idx: " << idx << ", pa: " << pa << ", pb: " << pb;
			if (idx >= newLines.size() || idx < 0)
			{
				kDebug() << "error: invalid index from points! idx: " << idx << ", pa: " << pa << ", pb: " << pb;
				continue;
			}
			if (!newLines[idx])
				return idx;
			else
				kDebug() << "old line: " << idx << ", pa: " << pa << ", pb: " << pb;
		}
		
		if ( (1<<i) & rv )
		{
			QPoint pa((i/ 6), (i % 6));
			QPoint pb(pa.x() + 1, pa.y());
			int idx = Board::pointsToIndex(pa, pb, 5, 5);
			kDebug() << "V: i: " << i << ", idx: " << idx << ", pa: " << pa << ", pb: " << pb;
			if (idx >= newLines.size() || idx < 0)
			{
				kDebug() << "error: invalid index from points! idx: " << idx << ", pa: " << pa << ", pb: " << pb;
				continue;
			}
			if (!newLines[idx])
				return idx;
			else
				kDebug() << "old line: " << idx << ", pa: " << pa << ", pb: " << pb;
		}
	}
// ms = (res["result"]["H"], res["result"]["V"])
// moves = []
// for i in range(2):
// 		for n in range(30):
// 				if ((1<<n)&ms[i]) != 0:
// 						moves.append(self.dab.num2move(((1<<n)|(i<<31)), 1, 1))
// while len(moves) > 1:
// 		for m in moves:
// 				if not self.dab.change(m):
// 						self.dab.move(m)
// 						moves.remove(m)
// 						break
// self.dab.move(moves[0])
// moves.remove(moves[0])

// def num2move(self, value, who, step=-1):
// ty, x, y = 1, -1, -1
// if (value&(1<<31)) != 0:
// 		ty = 0 # horizon
// for i in range(5)[::step]:
// 		for j in range(6)[::step]:
// 				if (value&1) == 1:
// 						if ty == 0: x, y = j, i
// 						else: x, y = i, j
// 						break
// 				value >>= 1
// 		if x != -1:
// 				break
// return (ty, x, y, who)
	return -1;
}

void QDab::processError(const QProcess::ProcessError &error)
{
	kDebug() << "Got error signal from qdab!";
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
	kDebug() << "***                        QDAB ERROR                        ***";
	kDebug() << "****************************************************************";
	kDebug() << "qdab error: " << info;
}

void QDab::processStateChanged(const QProcess::ProcessState &newState)
{
	kDebug() << "processStateChanged!";
	kDebug() << "****************************************************************";
	kDebug() << "***                    QDAB STATE CHANGED                    ***";
	kDebug() << "****************************************************************";
	QString state = "";
	switch (newState)
	{
		case QProcess::NotRunning: state = "NotRunning"; break;
		case QProcess::Starting: state = "Starting"; break;
		case QProcess::Running: state = "Running"; break;
	}
	kDebug() << "qdab state: " << state;
}

void QDab::processFinished(const int &exitCode, const QProcess::ExitStatus &exitStatus)
{
	kDebug() << "processFinished!";
	kDebug() << "qdab exit code: " << exitCode;
	kDebug() << "qdab exit status: " << (exitStatus == QProcess::NormalExit ? "normal" : "crash");
}

void QDab::processReadyReadStandardError()
{
	kDebug() << "processReadyReadStandardError!";
	qdabServer->setReadChannel(QProcess::StandardError);
	QByteArray qdabStdErrTmp = qdabServer->readAll();
	kDebug() << qdabStdErrTmp;
	qdabStdErrStream << qdabStdErrTmp;
	qdabStdErrStream.flush();
}

void QDab::processReadyReadStandardOutput()
{
	kDebug() << "processReadyReadStandardOutput!";
	qdabServer->setReadChannel(QProcess::StandardOutput);
	QByteArray qdabStdOutTmp = qdabServer->readAll();
	//kDebug() << "qdab stdout: " << QString(qdabStdOutTmp);
	qdabStdOutStream << qdabStdOutTmp;
	qdabStdOutStream.flush();
}

#include "qdab.moc"
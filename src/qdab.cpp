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

QDab::QDab(int newPlayerId, int newMaxPlayerId, int newWidth, int newHeight, int newLevel, int thinkTime) : KSquaresAi(newWidth, newHeight)
{
	timeout = thinkTime;
	playerId = newPlayerId;
}

QDab::~QDab()
{
}

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
		Board::indexToPoints(i, &p1, &p2, 5, 5, false);
		kDebug() << "xy: (" << p1.x() << "," << p1.y() << ")";
		if (p1.x() != p2.x())
			v |= (1<<(p1.y()*6+p1.x()));
		else
			h |= (1<<(p1.x()*6+p1.y()));
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
		return -1;
	}
	
	uint id = QDateTime::currentDateTime().toTime_t();
	
	QString request = "{\"params\": [{\"Timeout\": "+QString::number(timeout)+", \"Board\": {\"H\": "+QString::number(h)+", \"S\": [0, 0], \"Now\": "+QString::number(playerId)+", \"Turn\": "+QString::number(turn)+", \"V\": "+QString::number(v)+"}, \"Algorithm\": \"quctann\"}], \"id\": "+QString::number(id)+", \"method\": \"Server.MakeMove\"}";
	QByteArray sendBA = request.toAscii();
	socket.write(sendBA);
	kDebug() << "request: " << sendBA;
	socket.waitForBytesWritten();
	
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

	return -1;
}
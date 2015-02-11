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

QDab::QDab(int newPlayerId, int newMaxPlayerId, int newWidth, int newHeight, int newLevel, int thinkTime) : KSquaresAi(newWidth, newHeight)
{
	/*
	socket = new QTcpSocket(this);

	connect(socket, SIGNAL(readyRead()), this, SLOT(recvData()));
	connect(socket, SIGNAL(error(QAbstractSocket::SocketError)),
					this, SLOT(displayError(QAbstractSocket::SocketError)));
	*/
}

QDab::~QDab()
{
	//delete socket;
}

int QDab::chooseLine(const QList<bool> &newLines, const QList<int> &newSquareOwners, const QList<Board::Move> &lineHistory)
{
	const int Timeout = 5 * 1000;

	QTcpSocket socket;
	socket.connectToHost("0.0.0.0", 12345);

	if (!socket.waitForConnected(Timeout))
	{
		kDebug() << "error (wait for connected): " << socket.errorString();
		return -1;
	}
	kDebug() << "state: " << socket.state();
	
	QString testMessage = "{\"params\": [{\"Timeout\": 10000, \"Board\": {\"H\": 1, \"S\": [0, 0], \"Now\": 1, \"Turn\": 1, \"V\": 0}, \"Algorithm\": \"quctann\"}], \"id\": 1423674120, \"method\": \"Server.MakeMove\"}";
	QByteArray sendBA = testMessage.toAscii();
	kDebug() << "state before write: " << socket.state();
	socket.write(sendBA);
	kDebug() << "state after write: " << socket.state();
	kDebug() << "send: " << sendBA;
	
	socket.waitForBytesWritten();
	kDebug() << "test message sent";
	kDebug() << "state after bytes written: " << socket.state();

	/*
	while (!socket.waitForReadyRead(-1))
	{
		kDebug() << "state: " << socket.state();
		if (socket.state() != QAbstractSocket::ConnectedState)
			break;
	}
	*/
	
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
			kDebug() << "got sth!";
			responseBA.append(socket.readAll());
			kDebug() << ": " << responseBA;
		}
		done = responseBA.count('{') > 0 && responseBA.count('{') == responseBA.count('}');
	}
	
	kDebug() << "response ba: " << responseBA;

	/*
	QDataStream in(&socket);
	in.setVersion(QDataStream::Qt_4_0);
	
	QString response;
	in >> response;
	
	kDebug() << "got response: " << response;
	*/
	return -1;
}
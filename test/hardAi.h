#ifndef KSQUARES_HARDAI_H
#define KSQUARES_HARDAI_H

#include <QtTest>
#include <QtCore>

#include "aicontroller.h"
#include "ksquaresgame.h"
#include "ksquaresio.h"

//Qt
#include <QDebug>

// generated
#include "testboardpath.h"

class hardAi: public QObject
{
	Q_OBJECT
	private slots:
		void testBoard001();
		void testBoard002();
		void testBoard003();
		void testBoard005();
		//void testBoard006();
};

//#include "hardAi.moc"

#endif //KSQUARES_HARDAI_H

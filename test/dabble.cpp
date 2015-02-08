#include <QtTest>
#include <QtCore>

#include "dbgame-nohash.h"

#include <QList>
#include <QPair>
#include <QPoint>
#include <QElapsedTimer>
#include <KDebug>

// generated
#include "testboardpath.h"

class dabble : public QObject
{
	Q_OBJECT
	private slots:
		void testDabble001();
		void testDabble002();
		void testDabble003();
};


/**
 * w,h: size in boxes
 */
int pointsToIndex(QPoint a, QPoint b, int w, int h)
{
	if (a.x() > b.x() || a.y() > b.y())
		return -10;
	return (w+1)*a.y() + w*b.y() + a.x();
}

QPair<QPoint, QPoint> coinsToPoints(QPoint a, QPoint b, int w, int h)
{
	if (a.x() == b.x())
	{
		QPoint ra(a.x() - 1, a.y());
		QPoint rb(a.x(), a.y());
		return QPair<QPoint, QPoint>(ra, rb);
	}
	else
	{
		QPoint ra(a.x(), a.y() - 1);
		QPoint rb(a.x(), a.y());
		return QPair<QPoint, QPoint>(ra, rb);
	}
}

int coinsToIndex(QPoint a, QPoint b, int w, int h)
{
	QPair<QPoint, QPoint> points = coinsToPoints(a, b, w, h);
	return pointsToIndex(points.first, points.second, w, h);
}

QPair<QPoint, QPoint> pointsToCoins(QPoint a, QPoint b, int w, int h)
{
	if (a.y() == b.y())
		return QPair<QPoint, QPoint>(QPoint(a.x() + 1, a.y()), QPoint(a.x() + 1, a.y() + 1));
	else
		return QPair<QPoint, QPoint>(QPoint(a.x(), a.y() + 1), QPoint(a.x() + 1, a.y() + 1));
}

void dabble::testDabble001()
{
	QCOMPARE((coinsToPoints(QPoint(1,0), QPoint(1,1), 2, 1)), (QPair<QPoint, QPoint>(QPoint(0,0), QPoint(1,0))));
	QCOMPARE((coinsToPoints(QPoint(2,0), QPoint(2,1), 2, 1)), (QPair<QPoint, QPoint>(QPoint(1,0), QPoint(2,0))));
	QCOMPARE((coinsToPoints(QPoint(0,1), QPoint(1,1), 2, 1)), (QPair<QPoint, QPoint>(QPoint(0,0), QPoint(0,1))));
	QCOMPARE((coinsToPoints(QPoint(1,1), QPoint(2,1), 2, 1)), (QPair<QPoint, QPoint>(QPoint(1,0), QPoint(1,1))));
	QCOMPARE((coinsToPoints(QPoint(2,1), QPoint(3,1), 2, 1)), (QPair<QPoint, QPoint>(QPoint(2,0), QPoint(2,1))));
	QCOMPARE((coinsToPoints(QPoint(1,1), QPoint(1,2), 2, 1)), (QPair<QPoint, QPoint>(QPoint(0,1), QPoint(1,1))));
	QCOMPARE((coinsToPoints(QPoint(2,1), QPoint(2,2), 2, 1)), (QPair<QPoint, QPoint>(QPoint(1,1), QPoint(2,1))));
}

void dabble::testDabble002()
{
	QCOMPARE(coinsToIndex(QPoint(1,0), QPoint(1,1), 2, 1), 0);
	QCOMPARE(coinsToIndex(QPoint(2,0), QPoint(2,1), 2, 1), 1);
	QCOMPARE(coinsToIndex(QPoint(0,1), QPoint(1,1), 2, 1), 2);
	QCOMPARE(coinsToIndex(QPoint(1,1), QPoint(2,1), 2, 1), 3);
	QCOMPARE(coinsToIndex(QPoint(2,1), QPoint(3,1), 2, 1), 4);
	QCOMPARE(coinsToIndex(QPoint(1,1), QPoint(1,1), 2, 1), 5);
	QCOMPARE(coinsToIndex(QPoint(2,1), QPoint(2,1), 2, 1), 6);
}

void dabble::testDabble003()
{
	QCOMPARE((pointsToCoins(QPoint(0,0), QPoint(1,0), 2, 1)), (QPair<QPoint, QPoint>(QPoint(1,0), QPoint(1,1))));
	QCOMPARE((pointsToCoins(QPoint(1,0), QPoint(2,0), 2, 1)), (QPair<QPoint, QPoint>(QPoint(2,0), QPoint(2,1))));
	QCOMPARE((pointsToCoins(QPoint(0,0), QPoint(0,1), 2, 1)), (QPair<QPoint, QPoint>(QPoint(0,1), QPoint(1,1))));
	QCOMPARE((pointsToCoins(QPoint(1,0), QPoint(1,1), 2, 1)), (QPair<QPoint, QPoint>(QPoint(1,1), QPoint(2,1))));
	QCOMPARE((pointsToCoins(QPoint(2,0), QPoint(2,1), 2, 1)), (QPair<QPoint, QPoint>(QPoint(2,1), QPoint(3,1))));
	QCOMPARE((pointsToCoins(QPoint(0,1), QPoint(1,1), 2, 1)), (QPair<QPoint, QPoint>(QPoint(1,1), QPoint(1,2))));
	QCOMPARE((pointsToCoins(QPoint(1,1), QPoint(2,1), 2, 1)), (QPair<QPoint, QPoint>(QPoint(2,1), QPoint(2,2))));
}

QTEST_MAIN(dabble)
#include "dabble.moc"
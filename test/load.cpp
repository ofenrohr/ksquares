#include <QtTest>
#include <QtCore>

#include "aicontroller.h"
#include "ksquaresgame.h"
#include "ksquaresio.h"

// generated
#include "testboardpath.h"

class load: public QObject
{
	Q_OBJECT
	private slots:
		void testBoard005();
};

/**
 * Expectation: game is loaded correctly
 */
void load::testBoard005()
{
	QScopedPointer<KSquaresGame> sGame(new KSquaresGame());
	QList<int> lines;
	QVERIFY(KSquaresIO::loadGame(QString(TESTBOARDPATH) + "/3x2-test5.dbl", sGame.data(), &lines));
	QList<int> expectedLines;
	expectedLines.append(0);
	expectedLines.append(1);
	expectedLines.append(2);
	expectedLines.append(3);
	expectedLines.append(5);
	expectedLines.append(10);
	expectedLines.append(11);
	expectedLines.append(12);
	expectedLines.append(16);
	for (int i = 0; i < expectedLines.size(); i++)
	{
		QVERIFY(lines.contains(expectedLines.at(i)));
	}
	for (int i = 0; i < lines.size(); i++)
	{
		QVERIFY(expectedLines.contains(lines.at(i)));
	}
}

QTEST_MAIN(load)
#include "load.moc"
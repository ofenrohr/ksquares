#include <QtTest>
#include <QtCore>

#include "aifunctions.h"
#include "ksquaresgame.h"
#include "ksquaresio.h"

#include <KDebug>

// generated
#include "testboardpath.h"


class aiFunctionsTest : public aiFunctions
{
public:
	aiFunctionsTest(int w, int h) : aiFunctions(w,h) {}
	~aiFunctionsTest() {}
	void findOwnChainsTest(bool *lines, int linesSize, int width, int height, QList<QList<int> > *ownChains) const
	{
		findOwnChains(lines, linesSize, width, height, ownChains);
	}
};

class refactor : public QObject
{
	Q_OBJECT
	private slots:
		void testBoard001();
};

/**
 * test findOwnChains
 */
void refactor::testBoard001()
{
	QList<int> lines;
	bool linesb[24];
	for (int i = 0; i < 24; i++)
	{
		linesb[i] = false;
	}
	QScopedPointer<KSquaresGame> sGame(new KSquaresGame());
	kDebug() << "loading: " << QString(TESTBOARDPATH) << "/3x3_chaintest.dbl";
	QVERIFY(KSquaresIO::loadGame(QString(TESTBOARDPATH) + "/3x3_chaintest.dbl", sGame.data(), &lines));
	kDebug() << "drawn lines: " << lines;
	for (int i = 0; i < lines.size(); i++)
	{
		linesb[lines.at(i)] = true;
	}
	QList<QList<int> > ownChains;
	aiFunctionsTest aift(3,3);
	aift.findOwnChainsTest(linesb, 24, 3, 3, &ownChains);
	kDebug() << "ownChains.size() = " << ownChains.size();
	QList<int> chainLengths;
	for (int i = 0; i < ownChains.size(); i++)
	{
		kDebug() << "  " << ownChains.at(i);
		chainLengths.append(ownChains.at(i).size());
	}
	QVERIFY(chainLengths.size()==3);
	QVERIFY(chainLengths.contains(1));
	QVERIFY(chainLengths.contains(2));
	QVERIFY(chainLengths.contains(6));
}

QTEST_MAIN(refactor)
#include "refactor.moc"
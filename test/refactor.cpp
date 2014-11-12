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
	int findOwnChainsTest(bool *lines, int linesSize, int width, int height, QList<QList<int> > *ownChains) const
	{
		return findOwnChains(lines, linesSize, width, height, ownChains);
	}
	int classifyChainTest(const QList<int> chain, bool *lines) const
	{
    return classifyChain(chain, lines);
  }
  QString boardToStringTest(bool *lines)
  {
    return boardToString(lines);
  }
  QString linelistToStringTest(QList<int> lines)
  {
    return linelistToString(lines);
  }
};

class refactor : public QObject
{
	Q_OBJECT
	private slots:
		void testFindOwnChains001();
    void testClassifyChain002();
    void testClassifyChain003();
    void testFindOwnChains004();
};

/**
 * test findOwnChains
 */
void refactor::testFindOwnChains001()
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

/**
 * test classifyChain
 */
void refactor::testClassifyChain002()
{
  QList<int> lines;
  int linesSize = 2 * 4 * 4 + 4 + 4;
  bool linesb[linesSize];
  for (int i = 0; i < linesSize; i++)
  {
    linesb[i] = false;
  }
  QScopedPointer<KSquaresGame> sGame(new KSquaresGame());
  kDebug() << "loading: " << QString(TESTBOARDPATH) << "/4x4-classify.dbl";
  QVERIFY(KSquaresIO::loadGame(QString(TESTBOARDPATH) + "/4x4-classify.dbl", sGame.data(), &lines));
  QString tmpstr = "";
  for (int i = 0; i < lines.size(); i++)
  {
    linesb[lines.at(i)] = true;
    kDebug() << lines.at(i);
  }
  aiFunctionsTest aift(4,4);
  kDebug() << "input board: " << aift.linelistToStringTest(lines);
  //kDebug() << "input board: " << aift.boardToStringTest(linesb);
  
  int result;
  
  QList<int> singleChain;
  singleChain.append(0);
  singleChain.append(4);
  result = aift.classifyChainTest(singleChain, linesb);
  kDebug() << "single chain result: " << result;
  QVERIFY(result == 1);
  
  QList<int> longChain1;
  longChain1.append(13);
  longChain1.append(18);
  longChain1.append(27);
  longChain1.append(31);
  result = aift.classifyChainTest(longChain1, linesb);
  kDebug() << "long chain (shorter one) result: " << result;
  QVERIFY(result == 0);
  
  QList<int> quad;
  quad.append(6);
  quad.append(10);
  quad.append(11);
  quad.append(15);
  result = aift.classifyChainTest(quad, linesb);
  kDebug() << "quad result: " << result;
  QVERIFY(result == 2);
}

/**
 * test classifyChain
 */
void refactor::testClassifyChain003()
{
  QList<int> lines;
  int linesSize = 2 * 4 * 4 + 4 + 4;
  bool linesb[linesSize];
  for (int i = 0; i < linesSize; i++)
  {
    linesb[i] = false;
  }
  QScopedPointer<KSquaresGame> sGame(new KSquaresGame());
  kDebug() << "loading: " << QString(TESTBOARDPATH) << "/4x4-classif-2y.dbl";
  QVERIFY(KSquaresIO::loadGame(QString(TESTBOARDPATH) + "/4x4-classify-2.dbl", sGame.data(), &lines));
  QString tmpstr = "";
  for (int i = 0; i < lines.size(); i++)
  {
    linesb[lines.at(i)] = true;
    kDebug() << lines.at(i);
  }
  aiFunctionsTest aift(4,4);
  kDebug() << "input board: " << aift.linelistToStringTest(lines);
  //kDebug() << "input board: " << aift.boardToStringTest(linesb);
  
  int result;
  
  QList<int> shortChain1;
  shortChain1.append(0);
  shortChain1.append(9);
  shortChain1.append(13);
  result = aift.classifyChainTest(shortChain1, linesb);
  kDebug() << "shortChain1 result: " << result;
  QVERIFY(result == 1);
  
  QList<int> shortChain2;
  shortChain2.append(24);
  shortChain2.append(29);
  result = aift.classifyChainTest(shortChain2, linesb);
  kDebug() << "shortChain2 result: " << result;
  QVERIFY(result == 1);
}

/**
 * test findOwnChains
 */
void refactor::testFindOwnChains004()
{
  QList<int> lines;
  int linesSize = 2 * 4 * 4 + 4 + 4;
  bool linesb[linesSize];
  for (int i = 0; i < linesSize; i++)
  {
    linesb[i] = false;
  }
  QScopedPointer<KSquaresGame> sGame(new KSquaresGame());
  kDebug() << "loading: " << QString(TESTBOARDPATH) << "/4x4-chaintest.dbl";
  QVERIFY(KSquaresIO::loadGame(QString(TESTBOARDPATH) + "/4x4-chaintest.dbl", sGame.data(), &lines));
  kDebug() << "drawn lines: " << lines;
  for (int i = 0; i < lines.size(); i++)
  {
    linesb[lines.at(i)] = true;
  }
  QList<QList<int> > ownChains;
  aiFunctionsTest aift(4,4);
  int squaresCnt = aift.findOwnChainsTest(linesb, linesSize, 4, 4, &ownChains);
  kDebug() << "returned squaresCnt = " << squaresCnt;
  kDebug() << "ownChains.size() = " << ownChains.size();
  kDebug() << "input board: " << aift.boardToStringTest(linesb);
  QList<int> chainLengths;
  for (int i = 0; i < ownChains.size(); i++)
  {
    //kDebug() << "  " << ownChains.at(i);
    chainLengths.append(ownChains.at(i).size());
    kDebug() << "board " << i << ": " << aift.linelistToStringTest(ownChains.at(i));
  }
  QVERIFY(chainLengths.size()==1);
  QVERIFY(chainLengths.contains(3));
  QVERIFY(squaresCnt == 4);
}

QTEST_MAIN(refactor)
#include "refactor.moc"
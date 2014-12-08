#include <QtTest>
#include <QtCore>

#include "aifunctions.h"
#include "ksquaresgame.h"
#include "ksquaresio.h"
#include "aiBoard.h"
#include "aiAlphaBeta.h"

#include <QElapsedTimer>
#include <KDebug>

// generated
#include "testboardpath.h"

class alphabeta : public QObject
{
	Q_OBJECT
	private slots:
		void testAlphaBeta001();
};

/**
 * Test getMoveSequences
 */
void alphabeta::testAlphaBeta001()
{
	aiAlphaBeta ai;
	
}

QTEST_MAIN(alphabeta)
#include "alphabeta.moc"
#ifndef KSQUARES_TEST_BERLEKAMP
#define KSQUARES_TEST_BERLEKAMP

#include <QtTest>
#include <QtCore>

#include "aifunctions.h"
#include "ksquaresgame.h"
#include "ksquaresio.h"
#include "aiBoard.h"
#include "aicontroller.h"

#include <QElapsedTimer>
#include <QTextStream>
#include <QDebug>

// generated
#include "testboardpath.h"

// http://wilson.engr.wisc.edu/boxes/solve/3.html
class berlekamp : public QObject {
Q_OBJECT

    static const int testAIs[];

    void executeAi(Board *board, int player, QString name, QList<int> expectedLines);

private slots:

    void testBerlekamp001(); // 16
    void testBerlekamp002(); // 17, 21
    void testBerlekamp003(); // 7
    void testBerlekamp004(); // 12 (22, 23)
    void testBerlekamp005(); // 0
    void testBerlekamp006(); // 10 (7, 19, 22)
    void testBerlekamp007(); // 9
    void testBerlekamp008(); // 0
    void testBerlekamp009(); // 0
    void testBerlekamp010(); // 11
    void testBerlekamp011(); // 2 (20, 21)
    void testBerlekamp012(); // 15
    void testBerlekamp013(); // 17
};

#endif

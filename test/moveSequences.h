//
// Created by ofenrohr on 29.10.18.
//

#ifndef KSQUARES_MOVESEQUENCES_H
#define KSQUARES_MOVESEQUENCES_H

#include <QtCore>
#include <QtTest>

#include "aifunctions.h"
#include "ksquaresio.h"
#include "aiBoard.h"
#include "aicontroller.h"

#include <QElapsedTimer>
#include <QTextStream>
#include <QDebug>

// generated
#include "testboardpath.h"

// http://wilson.engr.wisc.edu/boxes/solve/3.html
class TestMoveSequences : public QObject {
Q_OBJECT

    static const int testAIs[];

private slots:

    void testMoveSeqs001();
    void testMoveSeqs002();
};

#endif //KSQUARES_MOVESEQUENCES_H

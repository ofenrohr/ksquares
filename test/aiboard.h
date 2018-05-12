#ifndef KSQUARES_AIBOARD_TEST_H
#define KSQUARES_AIBOARD_TEST_H

#include <QtTest>
#include <QtCore>

#include "aiBoard.h"
#include "aifunctions.h"

#include <QDebug>

// generated
#include "testboardpath.h"

class aiboard : public QObject {
Q_OBJECT
private slots:
    void testAiBoard001();
    void testAiBoard002();
};


#endif //KSQUARES_AIBOARD_H

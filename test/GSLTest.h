//
// Created by ofenrohr on 22.05.18.
//

#ifndef KSQUARES_GSLTEST_H
#define KSQUARES_GSLTEST_H

#include <QtCore>
#include <QtTest>
//#include <qt/QtTest/QtTest>
//#include <qt/QtCore/QObject>
//#include <qt/QtCore/QArgument>

class GSLTest: public QObject {
    Q_OBJECT
private slots:
    void testGSL001();
    void testGSL002();
    void testGSL003();
};

#endif //KSQUARES_GSL_H

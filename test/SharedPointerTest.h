//
// Created by ofenrohr on 31.05.18.
//

#ifndef KSQUARES_SHAREDPOINTERTEST_H
#define KSQUARES_SHAREDPOINTERTEST_H

#include <QtCore>
#include <QtTest>

class VerboseTestClass {
public:
    typedef QSharedPointer<VerboseTestClass> Ptr;
    VerboseTestClass() {
        printf("VerboseTestClass()\n");
    }
    ~VerboseTestClass() {
        printf("~VerboseTestClass\n");
    }
};

class SharedPointerTest : public QObject {
Q_OBJECT
private slots:
    void testSharedPointer();
};


#endif //KSQUARES_SHAREDPOINTERTEST_H

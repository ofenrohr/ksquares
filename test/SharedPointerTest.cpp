//
// Created by ofenrohr on 31.05.18.
//

#include "SharedPointerTest.h"

void SharedPointerTest::testSharedPointer() {
    QList<VerboseTestClass::Ptr> list;
    list.append(VerboseTestClass::Ptr(new VerboseTestClass()));
    list.append(VerboseTestClass::Ptr(new VerboseTestClass()));
    list.clear();
}

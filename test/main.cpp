#include <QtTest/QtTest>
#include "berlekamp.h"
#include "hardAi.h"

int main(int argc, char *argv[])
{
    int status = 0;
    status |= QTest::qExec(new berlekamp, argc, argv);
    status |= QTest::qExec(new hardAi, argc, argv);
    // status |= QTest::qExec(new TestObject, argc, argv);
    // status |= QTest::qExec(new ..., argc, argv);

    return status;
}

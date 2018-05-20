#include <cstring>
#include <QtTest/QtTest>
#include "berlekamp.h"
#include "hardAi.h"
#include "aiboard.h"
#include "alphazero.h"

int main(int argc, char *argv[])
{
    int subarg_start = 0;
    int status = 0;
    std::vector<std::string> argvec(argc);
    for (int i = 0; i < argc; i++) {
        std::string arg(argv[i]);
        if (arg == "--") {
            subarg_start = i+1;
        }
        if (subarg_start == 0) {
            argvec[i] = arg;
        } else {
            argvec[i] = "";
        }
    }
    if (subarg_start == 0) { // no args for qtest
        subarg_start = argc;
    }
    int subargc = argc - subarg_start + 1;
    char **subargv = new char*[subargc];
    subargv[0] = new char[strlen(argv[0])];
    memcpy(subargv[0], argv[0], strlen(argv[0]));
    for (int i = 0; i < subargc-1; i++) {
        int len = strlen(argv[i+subarg_start]);
        char *arg = new char[len];
        memcpy(arg, argv[i+subarg_start], len);
        subargv[i+1] = argv[i+subarg_start];
    }
    printf("argc: %d, subargc: %d, subarg_start: %d\nargs:\n", argc, subargc, subarg_start);
    for (int i = 0; i < argc; i++) {
        printf("%d: %s\n", i, argv[i]);
    }
    printf("subargs:\n");
    for (int i = 0; i < subargc; i++) {
        printf("%d: %s\n", i, subargv[i]);
    }


    bool all = false;
    if (argc == 1) { // no arguments, do all tests
        all = true;
    }
    for (int i = 0; i < argvec.size(); i++) {
        if (all || argvec[i] == "berlekamp") {
            printf("~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=\n");
            printf(" EXECUTING: berlekamp\n");
            printf("~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=\n");
            status |= QTest::qExec(new berlekamp, subargc, subargv);
        }
        if (all || argvec[i] == "hardai") {
            printf("~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=\n");
            printf(" EXECUTING: hardai\n");
            printf("~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=\n");
            status |= QTest::qExec(new hardAi, subargc, subargv);
        }
        if (all || argvec[i] == "aiboard") {
            printf("~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=\n");
            printf(" EXECUTING: aiboard\n");
            printf("~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=\n");
            status |= QTest::qExec(new aiboard, subargc, subargv);
        }
        if (all || argvec[i] == "alphazero") {
            printf("~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=\n");
            printf(" EXECUTING: alphazero\n");
            printf("~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=\n");
            status |= QTest::qExec(new alphazero, subargc, subargv);
        }
    }
    return status;
}

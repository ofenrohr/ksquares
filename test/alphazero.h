//
// Created by ofenrohr on 17.05.18.
//

#ifndef KSQUARES_ALPHAZERO_H
#define KSQUARES_ALPHAZERO_H


#include <QtCore/QObject>
#include <QtCore/QString>

class alphazero : public QObject {
Q_OBJECT
private:
    void runTest(KSquares::AILevel ai, QString testName, QString modelName, QList<QString> allNames, QList<QString> boardPaths, QList<QList<int>> allExpectedLines, bool gpu);
private slots:
    void testAlphaZero001();
    void testAlphaZero002();
    void testAlphaZero003();
    void testAlphaZero004();
    void testAlphaZero005();
    void testAlphaZero006();
    void testAlphaZero007();
    void testAlphaZero008();
    void testAlphaZero009();
    void testAlphaZero010();
    void testAlphaZero011();
    void testAlphaZero012();
    void testAlphaZero013();
};


#endif //KSQUARES_ALPHAZERO_H

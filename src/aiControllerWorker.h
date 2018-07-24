//
// Created by ofenrohr on 24.07.18.
//

#ifndef KSQUARES_AICONTROLLERWORKER_H
#define KSQUARES_AICONTROLLERWORKER_H

#include <QtCore/QObject>
#include "aicontroller.h"

// see http://qt-project.org/doc/qt-4.8/qthread.html#details
class aiControllerWorker : public QObject {
Q_OBJECT
    QThread workerThread;
    aiController::Ptr aicontroller;
    QList<bool> lines;
    QList<int> squares;
    QList<Board::Move> lineHistory;

public:
    aiControllerWorker(aiController::Ptr aic, const QList<bool> &newLines, const QList<int> &newSquareOwners,
                       const QList<Board::Move> &newLineHistory);
    ~aiControllerWorker();

public slots:
    void process();

signals:

    void lineChosen(const int &result);

    void finished();
};


#endif //KSQUARES_AICONTROLLERWORKER_H

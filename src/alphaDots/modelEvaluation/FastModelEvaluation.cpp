//
// Created by ofenrohr on 30.03.18.
//

#include "FastModelEvaluation.h"
#include "FastModelEvaluationWorker.h"
#include <QThread>
#include <qdebug.h>

using namespace AlphaDots;

FastModelEvaluation::FastModelEvaluation(int threads) : QObject() {
    qDebug() << "[FastModelEvaluation] init";
    setupManager = nullptr;
    threadCnt = threads;
}

FastModelEvaluation::~FastModelEvaluation() {
    if (setupManager != nullptr) {
        delete setupManager;
    }
}

void FastModelEvaluation::startEvaluation(QList<AITestSetup> *testSetups, TestResultModel *resultModel) {
    qDebug() << "[FastModelEvaluation] starting fast evaluation";
    setupManager = new AITestSetupManager(testSetups);
    for (int t = 0; t < threadCnt; t++) {
        // https://mayaposch.wordpress.com/2011/11/01/how-to-really-truly-use-qthreads-the-full-explanation/
        auto *thread = new QThread();
        auto *worker = new FastModelEvaluationWorker(setupManager, resultModel);
        worker->moveToThread(thread);
        connect(thread, SIGNAL(started()), worker, SLOT(process()));
        connect(worker, SIGNAL(finished(int)), thread, SLOT(quit()));
        connect(worker, SIGNAL(finished(int)), worker, SLOT(deleteLater()));
        connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
        thread->start();
    }
}

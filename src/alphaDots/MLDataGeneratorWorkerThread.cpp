//
// Created by ofenrohr on 10/19/17.
//

#include "MLDataGeneratorWorkerThread.h"
#include "aiBoard.h"
#include "MLDataGenerator.h"
#include "aiEasyMediumHard.h"

#include <QDebug>
#include <QtCore/QUuid>
#include <alphaDots/datasets/FirstTryDataset.h>

using namespace AlphaDots;

MLDataGeneratorWorkerThread::MLDataGeneratorWorkerThread(long examples, DatasetGenerator::Ptr generator,
                                                         int threadID) {
    sampleCnt = examples;
    dataGenerator = generator;
    threadId = threadID;
}

MLDataGeneratorWorkerThread::~MLDataGeneratorWorkerThread() {
    //dataGenerator->cleanup();
}

void MLDataGeneratorWorkerThread::process() {
    //qDebug() << "thread";

    int oldProgress = 0;

    for (int i = 0; i < sampleCnt; i++) {

        dataGenerator->generateDataset();

        int progr = (int) ((100.0 / sampleCnt) * (i+1.0));
        if (progr != oldProgress) {
            //qDebug() << "Progress: " << QString::number(progr);
            emit progress(progr, threadId);
            oldProgress = progr;
        }
    }

    //dataGenerator->cleanup();

    emit finished(threadId);
}

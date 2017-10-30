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

MLDataGeneratorWorkerThread::MLDataGeneratorWorkerThread(long examples, DatasetGenerator::Ptr generator) {
    sampleCnt = examples;
    dataGenerator = generator;
}

MLDataGeneratorWorkerThread::~MLDataGeneratorWorkerThread() {
}

void MLDataGeneratorWorkerThread::process() {
    qDebug() << "thread";

    int oldProgress = 0;

    for (int i = 0; i < sampleCnt; i++) {

        dataGenerator->generateDataset();

        int progr = (int) ((100.0 / sampleCnt) * (i+1.0));
        if (progr != oldProgress) {
            emit progress(progr);
            oldProgress = progr;
        }
    }

    emit finished();
}

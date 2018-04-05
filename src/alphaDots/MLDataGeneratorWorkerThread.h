//
// Created by ofenrohr on 10/19/17.
//

#ifndef KSQUARES_MLDATAGENERATORWORKERTHREAD_H
#define KSQUARES_MLDATAGENERATORWORKERTHREAD_H


#include <QtCore/QObject>
#include <alphaDots/datasets/DatasetGenerator.h>

namespace AlphaDots {
    class MLDataGeneratorWorkerThread : public QObject {
    Q_OBJECT
    public:
        MLDataGeneratorWorkerThread(long examples, DatasetGenerator::Ptr &generator, int threadID);

        ~MLDataGeneratorWorkerThread();

    public slots:

        void process();

    signals:

        void progress(const int &progress, const int &threadid);

        void finished(int threadID);

    private:
        long sampleCnt;
        int threadId;
        DatasetGenerator::Ptr dataGenerator;
    };

}

#endif //KSQUARES_MLDATAGENERATORWORKERTHREAD_H

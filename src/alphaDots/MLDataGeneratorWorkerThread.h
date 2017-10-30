//
// Created by ofenrohr on 10/19/17.
//

#ifndef KSQUARES_MLDATAGENERATORWORKERTHREAD_H
#define KSQUARES_MLDATAGENERATORWORKERTHREAD_H


#include <QtCore/QObject>
#include <alphaDots/datasets/DatasetGenerator.h>

class MLDataGeneratorWorkerThread : public QObject
{
	Q_OBJECT
public:
	MLDataGeneratorWorkerThread(long examples, DatasetGenerator::Ptr generator);
	~MLDataGeneratorWorkerThread();

public slots:
	void process();

signals:
	void progress(const int &p);
	void finished();

private:
	long sampleCnt;
    DatasetGenerator::Ptr dataGenerator;
};


#endif //KSQUARES_MLDATAGENERATORWORKERTHREAD_H

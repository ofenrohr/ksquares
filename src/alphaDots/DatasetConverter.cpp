//
// Created by ofenrohr on 30.10.17.
//

#include "DatasetConverter.h"

#include <QDebug>

DatasetConverter::DatasetConverter(QString pythonPath, QString converterPath, int sampleCnt, QString outputFile) {
    pythonExecutable = pythonPath;
    converterExecutable = converterPath;
	samples = sampleCnt;
	datasetPath = outputFile;
    converter = NULL;
}

DatasetConverter::~DatasetConverter() {
    if (!stopDatasetConverter()) {
        qDebug() << "failed to stop dataset converter";
    }
}

bool DatasetConverter::startDatasetConverter() {
    qDebug() << "startDatasetConverter()";
	if (converter)
	{
		qDebug() << "WARNING: converter already running!!! tearing it down...";
		stopDatasetConverter();
		if (converter)
		{
			qDebug() << "ERROR: dabble still running, teardown didn't work! not starting dabble";
			return false;
		}
	}
	QStringList converterArguments;
	converterArguments << converterExecutable << QStringLiteral("--zmq") << QString::number(samples) << QStringLiteral("--output-file") << datasetPath;
	converter = new QProcess();
	qRegisterMetaType<QProcess::ProcessError>("QProcess::ProcessError");
	qRegisterMetaType<QProcess::ProcessState>("QProcess::ProcessState");
	qRegisterMetaType<QProcess::ExitStatus>("QProcess::ExitStatus");
	connect(converter, SIGNAL(error(QProcess::ProcessError)), this, SLOT(processError(QProcess::ProcessError)));
	connect(converter, SIGNAL(stateChanged(QProcess::ProcessState)), this, SLOT(processStateChanged(QProcess::ProcessState)));
	connect(converter, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(processFinished(int, QProcess::ExitStatus)));
	connect(converter, SIGNAL(readyReadStandardError()), this, SLOT(processReadyReadStandardError()));
	connect(converter, SIGNAL(readyReadStandardOutput()), this, SLOT(processReadyReadStandardOutput()));
	qDebug() << "starting dataset converter: " << pythonExecutable << ", args: " << converterArguments;
	converter->start(pythonExecutable, converterArguments);
	converter->setReadChannel(QProcess::StandardOutput);
	if (!converter->waitForStarted())
	{
		qDebug() << "ERROR: starting converter failed!";
		return false;
	}
    return true;
}

bool DatasetConverter::stopDatasetConverter() {
	qDebug() << "stopDatasetConverter()";
	if (converter!=NULL)
	{
		disconnect(converter, SIGNAL(error(QProcess::ProcessError)), this, SLOT(processError(QProcess::ProcessError)));
		disconnect(converter, SIGNAL(stateChanged(QProcess::ProcessState)), this, SLOT(processStateChanged(QProcess::ProcessState)));
		disconnect(converter, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(processFinished(int, QProcess::ExitStatus)));
		disconnect(converter, SIGNAL(readyReadStandardError()), this, SLOT(processReadyReadStandardError()));
		disconnect(converter, SIGNAL(readyReadStandardOutput()), this, SLOT(processReadyReadStandardOutput()));

		if (converter->state() != QProcess::NotRunning)
		{
			qDebug() << "trying to kill converter process";
			converter->kill();
			converter->terminate();
			if (converter->waitForFinished()) {
				qDebug() << "killed converter";
			} else {
				qDebug() << "killing converter failed!";
				return false;
			}
		}
		delete converter;
		converter = NULL;
	}
	converterRunning = true;
    return true;
}

void DatasetConverter::processError(const QProcess::ProcessError error) {
    qDebug() << "Got error signal from converter!";
	QString info;
	switch (error)
	{
		case QProcess::FailedToStart: info = QStringLiteral("The process failed to start. Either the invoked program is missing, or you may have insufficient permissions to invoke the program."); break;
		case QProcess::Crashed: info = QStringLiteral("The process crashed some time after starting successfully."); break;
		case QProcess::Timedout: info = QStringLiteral("The last waitFor...() function timed out. The state of QProcess is unchanged, and you can try calling waitFor...() again."); break;
		case QProcess::WriteError: info = QStringLiteral("An error occurred when attempting to write to the process. For example, the process may not be running, or it may have closed its input channel."); break;
		case QProcess::ReadError: info = QStringLiteral("An error occurred when attempting to read from the process. For example, the process may not be running."); break;
		case QProcess::UnknownError: info = QStringLiteral("An unknown error occurred. This is the default return value of error()."); break;
	}
	qDebug() << "****************************************************************";
	qDebug() << "***                    CONVERTER ERROR                       ***";
	qDebug() << "****************************************************************";
	qDebug() << "converter error: " << info;
}

void DatasetConverter::processStateChanged(const QProcess::ProcessState newState) {
    qDebug() << "processStateChanged!";
	qDebug() << "****************************************************************";
	qDebug() << "***                CONVERTER STATE CHANGED                   ***";
	qDebug() << "****************************************************************";
	QString state;
	switch (newState)
	{
		case QProcess::NotRunning: state = QStringLiteral("NotRunning"); break;
		case QProcess::Starting:
			state = QStringLiteral("Starting");
		break;
		case QProcess::Running: state = QStringLiteral("Running"); break;
	}
	qDebug() << "converter state: " << state;
}

void DatasetConverter::processFinished(const int &exitCode, const QProcess::ExitStatus exitStatus) {
    qDebug() << "processFinished!";
	qDebug() << "exit code: " << exitCode;
	qDebug() << "exit status: " << (exitStatus == QProcess::NormalExit ? "normal" : "crash");

	converterRunning = false;
}

void DatasetConverter::processReadyReadStandardError() {
	converter->setReadChannel(QProcess::StandardError);
	QByteArray dabbleStdOutTmp = converter->readAll();
	qDebug() << "stdout: " << dabbleStdOutTmp.toStdString().c_str();
}

void DatasetConverter::processReadyReadStandardOutput() {
	converter->setReadChannel(QProcess::StandardOutput);
	QByteArray dabbleStdOutTmp = converter->readAll();
	qDebug() << "stdout: " << dabbleStdOutTmp.toStdString().c_str();
}

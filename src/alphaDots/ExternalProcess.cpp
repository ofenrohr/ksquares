//
// Created by ofenrohr on 30.10.17.
//

#include "ExternalProcess.h"

#include <QDebug>

ExternalProcess::ExternalProcess(QString processPath, QStringList arguments) {
	processExecutablePath = processPath;
	processArguments = arguments;
	process = nullptr;
}

ExternalProcess::~ExternalProcess() {
    if (!stopExternalProcess()) {
        qDebug() << "failed to stop process";
    }
}

bool ExternalProcess::startExternalProcess() {
    qDebug() << "startExternalProcess()";
	if (process)
	{
		qDebug() << "WARNING: process already running!!! tearing it down...";
		stopExternalProcess();
		if (process)
		{
			qDebug() << "ERROR: dabble still running, teardown didn't work! not starting dabble";
			return false;
		}
	}
	//converterArguments << converterExecutable << QStringLiteral("--zmq") << QString::number(samples) << QStringLiteral("--output-file") << datasetPath;
	process = new QProcess();
	qRegisterMetaType<QProcess::ProcessError>("QProcess::ProcessError");
	qRegisterMetaType<QProcess::ProcessState>("QProcess::ProcessState");
	qRegisterMetaType<QProcess::ExitStatus>("QProcess::ExitStatus");
	connect(process, SIGNAL(error(QProcess::ProcessError)), this, SLOT(processError(QProcess::ProcessError)));
	connect(process, SIGNAL(stateChanged(QProcess::ProcessState)), this, SLOT(processStateChanged(QProcess::ProcessState)));
	connect(process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(processFinished(int, QProcess::ExitStatus)));
	connect(process, SIGNAL(readyReadStandardError()), this, SLOT(processReadyReadStandardError()));
	connect(process, SIGNAL(readyReadStandardOutput()), this, SLOT(processReadyReadStandardOutput()));
	qDebug() << "starting process: " << processExecutablePath << ", args: " << processArguments;
	process->start(processExecutablePath, processArguments);
	process->setReadChannel(QProcess::StandardOutput);
	if (!process->waitForStarted())
	{
		qDebug() << "ERROR: starting process failed!";
		return false;
	}
    return true;
}

bool ExternalProcess::stopExternalProcess() {
	qDebug() << "stopExternalProcess()";
	if (process!=nullptr)
	{
		disconnect(process, SIGNAL(error(QProcess::ProcessError)), this, SLOT(processError(QProcess::ProcessError)));
		disconnect(process, SIGNAL(stateChanged(QProcess::ProcessState)), this, SLOT(processStateChanged(QProcess::ProcessState)));
		disconnect(process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(processFinished(int, QProcess::ExitStatus)));
		disconnect(process, SIGNAL(readyReadStandardError()), this, SLOT(processReadyReadStandardError()));
		disconnect(process, SIGNAL(readyReadStandardOutput()), this, SLOT(processReadyReadStandardOutput()));

		if (process->state() != QProcess::NotRunning)
		{
			qDebug() << "trying to kill process";
			process->kill();
			process->terminate();
			if (process->waitForFinished()) {
				qDebug() << "killed process";
			} else {
				qDebug() << "killing process failed!";
				return false;
			}
		}
		delete process;
		process = nullptr;
	}
	processRunning = true;
    return true;
}

void ExternalProcess::addEnvironmentVariable(QString name, QString value) {
	if (process == nullptr) {
		return;
	}
	QProcessEnvironment env = process->processEnvironment();
	env.insert(name, value);
	process->setProcessEnvironment(env);
}

void ExternalProcess::processError(const QProcess::ProcessError error) {
    qDebug() << "Got error signal from process!";
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
	qDebug() << "process error: " << info;
}

void ExternalProcess::processStateChanged(const QProcess::ProcessState newState) {
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
	qDebug() << "process state: " << state;
}

void ExternalProcess::processFinished(const int &exitCode, const QProcess::ExitStatus exitStatus) {
    qDebug() << "processFinished!";
	qDebug() << "exit code: " << exitCode;
	qDebug() << "exit status: " << (exitStatus == QProcess::NormalExit ? "normal" : "crash");

	processRunning = false;
}

void ExternalProcess::processReadyReadStandardError() {
	process->setReadChannel(QProcess::StandardError);
	QByteArray dabbleStdOutTmp = process->readAll();
	qDebug() << "stdout: " << dabbleStdOutTmp.toStdString().c_str();
}

void ExternalProcess::processReadyReadStandardOutput() {
	process->setReadChannel(QProcess::StandardOutput);
	QByteArray dabbleStdOutTmp = process->readAll();
	qDebug() << "stdout: " << dabbleStdOutTmp.toStdString().c_str();
}

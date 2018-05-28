//
// Created by ofenrohr on 30.10.17.
//

#include "ExternalProcess.h"

#include <QDebug>
#include <QtCore/QCoreApplication>
#include <QtCore/QTimer>

ExternalProcess::ExternalProcess(QString processPath, QStringList arguments, QString workingDirectory) {
	processExecutablePath = processPath;
	processArguments = arguments;
	processWorkingDirectory = workingDirectory;
	process = nullptr;
}

ExternalProcess::~ExternalProcess() {
    if (!stopExternalProcess()) {
        qDebug() << "failed to stop process";
    }
	QCoreApplication::processEvents();
}

bool ExternalProcess::startExternalProcess() {
    //qDebug() << "startExternalProcess()";
	if (process)
	{
		qDebug() << "WARNING: process already running!!! tearing it down...";
		stopExternalProcess();
		if (process)
		{
			qDebug() << "ERROR: process still running, teardown didn't work! not starting another process";
			return false;
		}
	}
	//converterArguments << converterExecutable << QStringLiteral("--zmq") << QString::number(samples) << QStringLiteral("--output-file") << datasetPath;
	process = new QProcess();
	QProcessEnvironment env = process->processEnvironment();
    for (int i = 0; i < envVars.count(); i++) {
        env.insert(envVars[i].first, envVars[i].second);
    }
	process->setProcessEnvironment(env);
	process->setWorkingDirectory(processWorkingDirectory);
	qRegisterMetaType<QProcess::ProcessError>("QProcess::ProcessError");
	qRegisterMetaType<QProcess::ProcessState>("QProcess::ProcessState");
	qRegisterMetaType<QProcess::ExitStatus>("QProcess::ExitStatus");
	connect(process, SIGNAL(error(QProcess::ProcessError)), this, SLOT(processError(QProcess::ProcessError)));
	connect(process, SIGNAL(stateChanged(QProcess::ProcessState)), this, SLOT(processStateChanged(QProcess::ProcessState)));
	connect(process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(processFinished(int, QProcess::ExitStatus)));
	if (!connect(process, SIGNAL(readyReadStandardError()), this, SLOT(processReadyReadStandardError()))) {
		qDebug() << "connect stderr failed";
	}
	if (!connect(process, SIGNAL(readyReadStandardOutput()), this, SLOT(processReadyReadStandardOutput()))) {
        qDebug() << "connect stdout failed";
    }
	/*
    if (!connect(process, SIGNAL(readyRead()), this, SLOT(processReadyReadStandardOutput()))) {
		qDebug() << "connect ready read stdout failed";
	}
	 */
	qDebug().noquote() << "starting process: " << processExecutablePath << processArguments;
	process->start(processExecutablePath, processArguments, QIODevice::ReadWrite);
	//process->setProcessChannelMode(QProcess::MergedChannels);
	//process->setReadChannel(QProcess::StandardOutput);
	if (!process->waitForStarted())
	{
		qDebug() << "ERROR: starting process failed!";
		return false;
	}

	QTimer::singleShot(1000, this, &ExternalProcess::processEvents);

    return true;
}

bool ExternalProcess::stopExternalProcess() {
	if (process != nullptr)
	{
		qDebug() << "stopExternalProcess " << processExecutablePath << processArguments;

		if (process->state() != QProcess::NotRunning)
		{
			//qDebug() << "trying to kill process";
			process->terminate();
			process->kill();
			// don't wait for it
			if (process->waitForFinished()) {
				qDebug() << "killed process";
			} else {
				qDebug() << "killing process failed!";
				return false;
			}
		}

        QCoreApplication::processEvents();

		disconnect(process, SIGNAL(error(QProcess::ProcessError)), this, SLOT(processError(QProcess::ProcessError)));
		disconnect(process, SIGNAL(stateChanged(QProcess::ProcessState)), this, SLOT(processStateChanged(QProcess::ProcessState)));
		disconnect(process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(processFinished(int, QProcess::ExitStatus)));
		disconnect(process, SIGNAL(readyReadStandardError()), this, SLOT(processReadyReadStandardError()));
		disconnect(process, SIGNAL(readyReadStandardOutput()), this, SLOT(processReadyReadStandardOutput()));

		process->deleteLater();

		QCoreApplication::processEvents();
	}
	processRunning = false;
    return true;
}

void ExternalProcess::addEnvironmentVariable(QString name, QString value) {
	envVars.append(QPair<QString,QString>(name, value));
}

void ExternalProcess::processError(const QProcess::ProcessError error) {
    //qDebug() << "Got error signal from process!";
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
	qDebug() << "***                    PROCESS   ERROR                       ***";
	qDebug() << "****************************************************************";
	qDebug() << "process error: " << info;
}

void ExternalProcess::processStateChanged(const QProcess::ProcessState newState) {
    //qDebug() << "processStateChanged!";
	//qDebug() << "****************************************************************";
	//qDebug() << "***                PROCESS STATE CHANGED                     ***";
	//qDebug() << "****************************************************************";
	QString state;
	switch (newState)
	{
		case QProcess::NotRunning:
			state = QStringLiteral("NotRunning");
            processRunning = false;
			break;
		case QProcess::Starting:
			state = QStringLiteral("Starting");
			processRunning = false;
            break;
		case QProcess::Running:
			state = QStringLiteral("Running");
			processRunning = true;
			break;
	}
	qDebug() << "process state: " << state;
}

void ExternalProcess::processFinished(const int &exitCode, const QProcess::ExitStatus exitStatus) {
    qDebug() << "processFinished!";
	qDebug() << "exit code: " << exitCode;
	qDebug() << "exit status: " << (exitStatus == QProcess::NormalExit ? "normal" : "crash");

	processRunning = false;
	emit processFinished();
}

void ExternalProcess::processReadyReadStandardError() {
	process->setReadChannel(QProcess::StandardError);
	QByteArray outputData = process->readAll();
	qDebug() << "stderr: " << outputData.toStdString().c_str();
}

void ExternalProcess::processReadyReadStandardOutput() {
	process->setReadChannel(QProcess::StandardOutput);
	QByteArray outputData = process->readAll();
	qDebug() << "stdout: " << outputData.toStdString().c_str();
}

void ExternalProcess::processEvents() {
	QCoreApplication::processEvents();
	if (processRunning) {
        QTimer::singleShot(1000, this, &ExternalProcess::processEvents);
	}
}
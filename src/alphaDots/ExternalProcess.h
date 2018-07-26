//
// Created by ofenrohr on 30.10.17.
//

#ifndef KSQUARES_DATASETCONVERTER_H
#define KSQUARES_DATASETCONVERTER_H


#include <QObject>
#include <QString>
#include <QSharedPointer>
#include <QProcess>

/**
 * This class is a wrapper to start the dataset converter service from the alphaDots repository.
 */
class ExternalProcess : public QObject {
Q_OBJECT
public:
    typedef QSharedPointer<ExternalProcess> Ptr;

    ExternalProcess(QString processPath, QStringList arguments, QString workingDirectory = tr("./"));
    ~ExternalProcess();

    /**
     * Launch the external process
     * @return true on success, false otherwise
     */
    bool startExternalProcess();

    /**
     * Stop the external process
     * @param terminate Send SIGTERM with QProcess::terminate() ?
     * @param kill Send SIGKILL with QProcess:kill() ?
     * @param wait use QProcess::waitForFinished() ?
     * @return true if process was stopped successfully
     */
    bool stopExternalProcess(bool terminate = true, bool kill = true, bool wait = true);

    bool isRunning() { return processRunning; }

    void addEnvironmentVariable(QString name, QString value);

    void setStdCapture(bool captureOut, bool captureErr);

    QString getStdErr();
    QString getStdOut();

public slots:
    void processError(QProcess::ProcessError error);
    void processStateChanged(QProcess::ProcessState newState);
    void processFinished(const int &exitCode, QProcess::ExitStatus exitStatus);
    void processReadyReadStandardError();
    void processReadyReadStandardOutput();

    void processEvents();

    void stopProcessTerm();
    void stopProcessKill();

signals:
    void processFinished();

protected:
    QString processExecutablePath;
    QStringList processArguments;
    QString processWorkingDirectory;
    QProcess *process;
    bool processRunning;
    QList<QPair<QString, QString>> envVars;
    bool captureStdOut;
    QString stdOut;
    bool captureStdErr;
    QString stdErr;
};


#endif //KSQUARES_DATASETCONVERTER_H

//
// Created by ofenrohr on 30.10.17.
//

#ifndef KSQUARES_DATASETCONVERTER_H
#define KSQUARES_DATASETCONVERTER_H


#include <QtCore/Q_PID>

/**
 * This class is a wrapper to start the dataset converter service from the alphaDots repository.
 */
class ExternalProcess : public QObject {
Q_OBJECT
public:
    ExternalProcess(QString processPath, QStringList arguments);
    ~ExternalProcess();
    /**
     * Launch the dataset converter.
     * @return true on success, false otherwise
     */
    bool startExternalProcess();
    bool stopExternalProcess();

    bool isRunning() { return processRunning; }

    void addEnvironmentVariable(QString name, QString value);

public slots:
    void processError(const QProcess::ProcessError error);
    void processStateChanged(const QProcess::ProcessState newState);
    void processFinished(const int &exitCode, const QProcess::ExitStatus exitStatus);
    void processReadyReadStandardError();
    void processReadyReadStandardOutput();

protected:
    QString processExecutablePath;
    QStringList processArguments;
    QProcess *process;
    bool processRunning;
    QList<QPair<QString, QString>> envVars;
};


#endif //KSQUARES_DATASETCONVERTER_H

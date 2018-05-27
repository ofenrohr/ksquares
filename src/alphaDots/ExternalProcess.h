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
    typedef QSharedPointer<ExternalProcess> Ptr;

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
    void processError(QProcess::ProcessError error);
    void processStateChanged(QProcess::ProcessState newState);
    void processFinished(const int &exitCode, QProcess::ExitStatus exitStatus);
    void processReadyReadStandardError();
    void processReadyReadStandardOutput();

    void processEvents();

signals:
    void processFinished();

protected:
    QString processExecutablePath;
    QStringList processArguments;
    QProcess *process;
    bool processRunning;
    QList<QPair<QString, QString>> envVars;
};


#endif //KSQUARES_DATASETCONVERTER_H

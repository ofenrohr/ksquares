//
// Created by ofenrohr on 30.10.17.
//

#ifndef KSQUARES_DATASETCONVERTER_H
#define KSQUARES_DATASETCONVERTER_H


#include <QtCore/Q_PID>

/**
 * This class is a wrapper to start the dataset converter service from the alphaDots repository.
 */
class DatasetConverter : public QObject {
Q_OBJECT
public:
    DatasetConverter(QString pythonPath, QString converterPath, int sampleCnt, QString outputFile);
    ~DatasetConverter();
    /**
     * Launch the dataset converter.
     * @return true on success, false otherwise
     */
    bool startDatasetConverter();
    bool stopDatasetConverter();

    bool isRunning() { return converterRunning; }

public slots:
    void processError(const QProcess::ProcessError error);
    void processStateChanged(const QProcess::ProcessState newState);
    void processFinished(const int &exitCode, const QProcess::ExitStatus exitStatus);
    void processReadyReadStandardError();
    void processReadyReadStandardOutput();

protected:
    QString pythonExecutable;
    QString converterExecutable;
    int samples;
    QString datasetPath;
    QProcess *converter;

    bool converterRunning;
};


#endif //KSQUARES_DATASETCONVERTER_H

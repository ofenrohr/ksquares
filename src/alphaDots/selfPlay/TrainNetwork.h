//
// Created by ofenrohr on 16.09.18.
//

#ifndef KSQUARES_TRAINNETWORK_H
#define KSQUARES_TRAINNETWORK_H


#include <QObject>
#include <QDateTime>
#include <alphaDots/ExternalProcess.h>

namespace AlphaDots {
    class TrainNetwork : public QObject {
        Q_OBJECT
    public:
        TrainNetwork(int epochs, bool gpu, bool upload, QString datasetDestDir);

        bool trainingInProgress() const;

        // getter for gui label strings
        const QString &getStatusStr() const;
        const QString &getLogLink() const;
        const QString &getEpochStr() const;
        const QString &getEtaStr() const;
        const QString &getLossStr() const;
        const QString &getPolicyLossStr() const;
        const QString &getValueLossStr() const;

    public slots:
        void startTraining(QString datasetPath, int iteration, QString initModelPath, QString targetModelPath,
                           QString iterationModelDir);
        void updateTrainingInfo();

    signals:
        void trainingFinished();
        void infoChanged();

    private:
        // training process
        int trainEpochs;
        bool trainOnGPU;
        bool upload;
        ExternalProcess *trainingProcess;
        QDateTime trainingStartTime;
        QString trainingLogBasename;
        int iteration;
        QString datasetDest;

        // info data
        QString statusStr;
        QString logLink;
        QString epochStr;
        QString etaStr;
        QString lossStr;
        QString policyLossStr;
        QString valueLossStr;
    };
}

#endif //KSQUARES_TRAINNETWORK_H

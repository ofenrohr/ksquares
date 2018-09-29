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
        TrainNetwork(int epochs, bool gpu, bool upload, QString &datasetDestDir, bool noAugmentation);

        bool trainingInProgress() const;

        // getter for gui label strings
        const QString &getStatusStr() const;
        const QString &getLogLink() const;
        const QString &getEpochStr() const;
        const QString &getEtaStr() const;
        const QString &getLossStr() const;
        const QString &getPolicyLossStr() const;
        const QString &getValueLossStr() const;

        // other getters
        const QString &getTrainingLogPath() const;
        int getEpochs() const;

    public slots:
        void startTraining(const QString &datasetPath, int iteration, const QString &initModelPath, const QString &targetModelPath);
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
        QString trainingLogPath;
        bool disableAugmentation;

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

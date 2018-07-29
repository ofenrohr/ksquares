//
// Created by ofenrohr on 02.05.18.
//

#ifndef KSQUARES_SELFPLAY_H
#define KSQUARES_SELFPLAY_H

#include <KXmlGui/KXmlGuiWindow>
#include <QtCore/QMutexLocker>
#include <QtCore/QDateTime>
#include <alphaDots/ModelInfo.h>
#include <alphaDots/ExternalProcess.h>
#include <alphaDots/datasets/StageFourDataset.h>
#include <alphaDots/MLDataGenerator.h>
#include "ui_SelfPlayForm.h"

namespace AlphaDots {
    class SelfPlay : public KXmlGuiWindow, public Ui::SelfPlayForm {
    Q_OBJECT
    public:
        SelfPlay(QString datasetDest, int threads, QString &initialModel, QString &targetModelName,
                 int iterations, int gamesPerIteration, int epochs, bool gpuTraining, DatasetType dataset,
                 bool doUpload, QList<QPoint> boardSizes, bool waitForTrainingToFinish);

        void initObject();

        void updateInfo();

    public slots:
        void recvProgress(int progress, int thread);

        void threadFinished(int thread);

        void trainingFinished();

        void updateTrainingInfo();

        void setupIteration();

        void finishIteration();

    private:
        QWidget *m_view;

        // general configuration
        QString datasetDirectory;
        int threadCnt;
        QList<QPoint> availableBoardSizes;
        int iterationCnt;
        int iterationSize;
        QString targetModelName;
        QString logdest;
        DatasetType datasetType;
        bool upload;
        bool waitForTraining;

        // current state infos
        ModelInfo currentModel;
        QPoint currentBoardSize;
        int iteration;
        int gamesCompleted;
        QList<bool> threadRunning;
        QList<StageFourDataset *> threadGenerators;
        QDateTime lastInfoUpdate;

        // mutex lockers
        mutable QMutex recvProgressMutex;
        mutable QMutex threadFinishedMutex;

        // data container for one iteration
        std::vector<uint8_t> *input;
        std::vector<uint8_t> *output;
        std::vector<double> *value;

        // training process
        int trainEpochs;
        bool trainOnGPU;
        ExternalProcess *alphaZeroV10Training;
        QDateTime trainingStartTime;
        QString trainingLogBasename;
    };
}


#endif //KSQUARES_SELFPLAY_H

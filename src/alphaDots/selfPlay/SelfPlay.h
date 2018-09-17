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
#include "GenerateData.h"
#include "TrainNetwork.h"

namespace AlphaDots {
    class SelfPlay : public KXmlGuiWindow, public Ui::SelfPlayForm {
    Q_OBJECT
    public:
        SelfPlay(QString &datasetDest, int threads, QString &initialModel, QString &targetModelName,
                 int iterations, int gamesPerIteration, int epochs, bool gpuTraining, DatasetType dataset,
                 bool doUpload, QList<QPoint> &boardSizes, bool waitForTrainingToFinish);

        void initObject();


    public slots:
        void trainingFinished();

        void updateTrainingInfo();

        void setupIteration();

        void generateDataFinished();

        void updateDataGenInfo();

        void finishIteration();

    private:
        QWidget *m_view;

        // general configuration
        QString datasetDirectory;
        int threadCnt;
        QList<QPoint> availableBoardSizes;
        int iterationCnt;
        QString targetModelName;
        QString logdest;
        bool upload;
        bool waitForTraining;

        // current state infos
        int iteration;

        // data generator part
        GenerateData *dataGen;
        TrainNetwork *trainNetwork;
    };
}


#endif //KSQUARES_SELFPLAY_H

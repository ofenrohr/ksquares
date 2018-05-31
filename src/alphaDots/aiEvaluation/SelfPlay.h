//
// Created by ofenrohr on 02.05.18.
//

#ifndef KSQUARES_SELFPLAY_H
#define KSQUARES_SELFPLAY_H

#include <KXmlGui/KXmlGuiWindow>
#include <alphaDots/ModelInfo.h>
#include <alphaDots/ExternalProcess.h>
#include <alphaDots/datasets/StageFourDataset.h>
#include <QtCore/QMutexLocker>
#include "ui_SelfPlayForm.h"

namespace AlphaDots {
    class SelfPlay : public KXmlGuiWindow, public Ui::SelfPlayForm {
    Q_OBJECT
    public:
        SelfPlay(QString datasetDest, int threads, QString &initialModel, QString &targetModelName, int gamesPerIteration, QString &logdest);

        void initObject();

        void updateInfo();

        void setupIteration();

        void finishIteration();

    public slots:
        void recvProgress(int progress, int thread);

        void threadFinished(int thread);

        void trainingFinished();

    private:
        QWidget *m_view;

        // general configuration
        QString datasetDirectory;
        int threadCnt;
        QList<QPoint> availableBoardSizes;
        int iterationSize;
        QString targetModelName;
        QString logdest;

        // current state infos
        ModelInfo currentModel;
        QPoint currentBoardSize;
        int iteration;
        int gamesCompleted;
        QList<bool> threadRunning;
        QList<StageFourDataset *> threadGenerators;

        // mutex lockers
        mutable QMutex recvProgressMutex;
        mutable QMutex threadFinishedMutex;

        // data container for one iteration
        std::vector<uint8_t> *input;
        std::vector<uint8_t> *output;
        std::vector<double> *value;

        // training process
        ExternalProcess *alphaZeroV10Training;
    };
}


#endif //KSQUARES_SELFPLAY_H

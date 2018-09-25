//
// Created by ofenrohr on 02.05.18.
//

#ifndef KSQUARES_SELFPLAY_H
#define KSQUARES_SELFPLAY_H

#include <KXmlGui/KXmlGuiWindow>
#include <QMutexLocker>
#include <QDateTime>
#include <QElapsedTimer>
#include <alphaDots/ModelInfo.h>
#include <alphaDots/ExternalProcess.h>
#include <alphaDots/datasets/StageFourDataset.h>
#include <alphaDots/MLDataGenerator.h>
#include <alphaDots/ReportLogger.h>
#include "ui_SelfPlayForm.h"
#include "GenerateData.h"
#include "TrainNetwork.h"
#include "EvaluateNetwork.h"

namespace AlphaDots {
    class SelfPlay : public KXmlGuiWindow, public Ui::SelfPlayForm {
    Q_OBJECT
    public:
        SelfPlay(QString &datasetDest, int threads, QString &initialModel, QString &targetModelName,
                 int iterations, int gamesPerIteration, int epochs, bool gpuTraining, DatasetType dataset,
                 bool doUpload, QList<QPoint> &boardSizes, int gamesPerEvaluation, bool noEvaluation,
                 QString &reportDir, bool doQuickStart);
        ~SelfPlay();

        void initObject();

        enum SelfPlayMode {
            GENERATE,
            TRAIN,
            EVALUATE
        };

    public slots:
        void updateOverview();

        void setupIteration();

        void generateDataFinished();

        void updateDataGenInfo();

        void trainingFinished();

        void updateTrainingInfo();

        void evaluationFinished();

        void updateEvaluationInfo();

        void finishIteration();

    private:
        QWidget *m_view;

        // GSL random number generator
        gsl_rng *rng;

        // general configuration
        QString datasetDirectory;
        int threadCnt;
        QList<QPoint> availableBoardSizes;
        int iterationCnt;
        QString targetModelName;
        QString logdest;
        bool upload;
        bool disableEvaluation;
        QDir reportDir;
        bool quickStart;

        // report stream
        QString reportId;
        QString reportFilePath;
        QSharedPointer<ReportLogger> report;
        QElapsedTimer timer;

        // current state infos
        SelfPlayMode mode;
        int iteration;
        ModelInfo bestModel;
        ModelInfo contendingModel;

        // data generator part
        GenerateData *dataGen;
        // train network part
        TrainNetwork *trainNetwork;
        // evaluate network part
        EvaluateNetwork *evaluateNetwork;
    };
}


#endif //KSQUARES_SELFPLAY_H

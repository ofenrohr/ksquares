//
// Created by ofenrohr on 16.09.18.
//

#ifndef KSQUARES_GENERATEDATA_H
#define KSQUARES_GENERATEDATA_H


#include <QtCore/QObject>
#include <QtCore/QPoint>
#include <QtCore/QMutex>
#include <QtCore/QDateTime>
#include <alphaDots/ModelInfo.h>
#include <alphaDots/datasets/StageFourDataset.h>
#include <alphaDots/MLDataGenerator.h>

namespace AlphaDots {
    class GenerateData : public QObject {
        Q_OBJECT
    public:
        GenerateData(QString &initialModelName, QPoint &boardSize, int gamesPerIteraion, DatasetType datasetType,
                     int threads, QString &datasetDestination, KSquares::AILevel aiLevel);
        ~GenerateData();

        std::vector<uint8_t> *getInput() {return input;}
        std::vector<uint8_t> *getOutput() {return output;}
        std::vector<double> *getValue() {return value;}

        // getter
        ModelInfo getCurrentModel() {return currentModel;}
        QString modelName() {return currentModel.name();}
        QPoint boardSize() {return currentBoardSize;}
        QString boardSizeStr() {return tr("%1 x %2").arg(currentBoardSize.x()).arg(currentBoardSize.y());}
        int completedGames() {return gamesCompleted;}
        int gamesPerIteration() {return iterationSize;}
        DatasetType getDatasetType() {return datasetType;}
        const QString &getDatasetPath() const {return datasetPath;}

        // setter
        void setBoardSize(QPoint &size) {currentBoardSize = size;}
        void setCurrentModel(ModelInfo &model) {currentModel = model;}

    public slots:
        void startIteration();

        void recvProgress(int progress, int thread);

        void threadFinished(int thread);

        void finishIteration();

    signals:

        void infoChanged();

        void iterationFinished();

    private:
        // data container for one iteration
        std::vector<uint8_t> *input;
        std::vector<uint8_t> *output;
        std::vector<double> *value;

        // configuration
        QString datasetDirectory;
        int iterationSize;
        DatasetType datasetType;
        int threadCnt;
        KSquares::AILevel aiLevel;

        // state data
        ModelInfo currentModel;
        QPoint currentBoardSize;
        int gamesCompleted;
        QList<bool> threadRunning;
        QList<StageFourDataset *> threadGenerators;
        QDateTime lastInfoUpdate;

        // result data
        QString datasetPath;

        // mutex lockers
        mutable QMutex recvProgressMutex;
        mutable QMutex threadFinishedMutex;

    };
}

#endif //KSQUARES_GENERATEDATA_H

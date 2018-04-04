//
// Created by ofenrohr on 10/15/17.
//

#ifndef KSQUARES_MLDATAGENERATOR_H
#define KSQUARES_MLDATAGENERATOR_H


#include <QtCore/QArgument>
#include <KXmlGuiWindow>
#include "ui_mldataview.h"
#include <QtWidgets/QLabel>
#include <QtCore/QDir>
#include <alphaDots/datasets/DatasetGenerator.h>
#include "aiBoard.h"
#include "aicontroller.h"
#include "gameboardscene.h"
#include "gameboardview.h"
#include "ExternalProcess.h"


namespace AlphaDots {
    enum DatasetType {
        FirstTry,
        StageOne,
        BasicStrategy,
        LSTM,
        LSTM2
    };

    class MLDataGenerator : public KXmlGuiWindow, public Ui::MLDataView {
    Q_OBJECT

    public:
        MLDataGenerator();

        /**
         * Generate examplesCnt training samples.
         * @param samples number of training examples
         */
        MLDataGenerator(long samples, DatasetType datasetType, int width, int height, QString destinationDirectory, int threads);

        ~MLDataGenerator();

        void initObject();

        /**
         * Define colors to use in images
         */
        static const int MLImageBackground = 0;
        static const int MLImageBoxA = 65;
        static const int MLImageBoxB = 150;
        static const int MLImageDot = 215;
        static const int MLImageLine = 255;
        /// number of threads
        int threadCnt;

        static aiBoard::Ptr createEmptyBoard(int width, int height);

        /**
         * Generates board with random (usrful) state with some safe moves left
         * @param width in boxes
         * @param height in boxes
         * @param safeMoves number of safe lines left
         * @return board with requested features
         */
        static aiBoard::Ptr generateRandomBoard(int width, int height, int safeMoves);

        static int makeAiMove(aiBoard::Ptr board, KSquaresAi::Ptr ai);

        static QList<int> makeAiMoves(aiBoard::Ptr board, KSquaresAi::Ptr ai, int freeLinesLeft);

        static QImage generateInputImage(aiBoard::Ptr board);

        static QImage generateOutputImage(aiBoard::Ptr board, KSquaresAi::Ptr ai);

        static QImage generateOutputImage(aiBoard::Ptr board, QList<int> lines, bool drawDots = false);

        static void saveImage(QString dataSetName, QString instanceName, QString dest, QImage &img);

        /**
         * Converts boxes (width or height) to pixels
         * @param boxes
         * @return
         */
        static int boxesToImgSize(int boxes);

    public slots:

        /**
         * next button clicked in GUI
         */
        void nextBtnClicked();

        /**
         * Get info that a thread finished generating data
         * @param threadIdx
         */
        void dataGeneratorFinished(int threadIdx);

        /**
         * Get progress updates from threads while they generate data
         * @param progress
         * @param thread
         */
        void recvProgress(int progress, int thread);

        /**
         * Select GUI generator
         * @param generator 0: firstTry, 1: stageOne, 2: basicStrategy, 3: sequence
         */
        void selectGenerator(int generator);

        void turnSliderChanged(int turnIdx);

    private:
        //QLabel *m_view;
        QWidget *m_view;
        GameBoardScene *gbs;


        // vars for displaying datasets
        Dataset guiDataset;
        int frameCnt;
        int displayFrame;

        // vars for generating datasets
        /// dataset destination directory
        QString datasetDestDir;
        /// number of samples in dataset
        long examplesCnt;
        /// width of sample in boxes
        int datasetWidth;
        /// height of sample in boxes
        int datasetHeight;
        /// number of running threads
        int runningThreadCnt;
        /// progress for each thread
        QList<int> threadProgr;
        /// Dataset type to generate en masse
        DatasetType generateDatasetType;
        /// One Dataset generator for each thread
        QList<DatasetGenerator::Ptr> threadGenerators;

        QImage inputImage;
        QImage outputImage;

        /// Dataset generator to use in GUI
        DatasetGenerator::Ptr guiGenerator;

        static void drawBackgroundAndDots(QImage &img, bool drawDots = true);
        static void drawLineAt(QImage &img, int lineIdx, int w, int h);
        static void drawLines(QImage &img, aiBoard::Ptr board);
        static void drawBoxes(QImage &img, aiBoard::Ptr board);

        void initConstructor();

        void setupGeneratorThreads();

        DatasetGenerator::Ptr getDatasetGenerator();
        DatasetGenerator::Ptr getFirstTryDatasetGenerator();
        DatasetGenerator::Ptr getStageOneDatasetGenerator();
        DatasetGenerator::Ptr getBasicStrategyDatasetGenerator();
        DatasetGenerator::Ptr getSequenceDatasetGenerator();
        DatasetGenerator::Ptr getTrainingSequenceDatasetGenerator();

        void generateGUIexample();
    };

}

#endif //KSQUARES_MLDATAGENERATOR_H

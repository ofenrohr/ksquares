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
    class MLDataGenerator : public KXmlGuiWindow, public Ui::MLDataView {
    Q_OBJECT

    public:
        MLDataGenerator();

        /**
         * Generate examplesCnt training samples.
         * @param samples number of training examples
         */
        MLDataGenerator(long samples);

        ~MLDataGenerator();

        void initObject();

        static const int MLImageBackground = 0;
        static const int MLImageBoxA = 65;
        static const int MLImageBoxB = 150;
        static const int MLImageDot = 215;
        static const int MLImageLine = 255;

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

        static QImage generateOutputImage(aiBoard::Ptr board, QList<int> lines);

        static void saveImage(QString dataSetName, QString instanceName, QString dest, QImage &img);

        static int boxesToImgSize(int boxes);

    public slots:

        void nextBtnClicked();

        void dataGeneratorFinished();

        void recvProgress(int progress, int thread);

        void selectGenerator(int generator);

    private:
        //QLabel *m_view;
        QWidget *m_view;
        GameBoardScene *gbs;

        long examplesCnt;
        int threadCnt;
        QList<int> threadProgr;

        QImage inputImage;
        QImage outputImage;

        DatasetGenerator::Ptr guiGenerator;

        static void drawBackgroundAndDots(QImage &img, bool drawDots = true);

        static void drawLineAt(QImage &img, int lineIdx, int w, int h);

        static void drawLines(QImage &img, aiBoard::Ptr board);

        static void drawBoxes(QImage &img, aiBoard::Ptr board);

        void initConstructor();

        void setupThread(DatasetGenerator::Ptr generator);

        void generateFirstTryDataset();

        void generateStageOneDataset();

        void generateBasicStrategyDataset();

        void generateGUIexample();
    };

}

#endif //KSQUARES_MLDATAGENERATOR_H

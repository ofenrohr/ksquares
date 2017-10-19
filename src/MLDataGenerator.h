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
#include "aiBoard.h"
#include "aicontroller.h"
#include "gameboardscene.h"
#include "gameboardview.h"

class MLDataGenerator : public KXmlGuiWindow, public Ui::MLDataView
{
    Q_OBJECT

public:
    MLDataGenerator();
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
    static QImage generateInputImage(aiBoard::Ptr board);
    static QImage generateOutputImage(aiBoard::Ptr board, KSquaresAi::Ptr ai);
    static void saveImage(QString dataSetName, QString instanceName, QString dest, QImage &img);

public slots:
    void nextBtnClicked();

private:
    //QLabel *m_view;
    QWidget *m_view;
    GameBoardScene *gbs;

    QImage inputImage;
    QImage outputImage;

    static int makeAiMove(aiBoard::Ptr board, KSquaresAi::Ptr ai);
    static QList<int> makeAiMoves(aiBoard::Ptr board, KSquaresAi::Ptr ai, int freeLinesLeft);

    static void drawBackgroundAndDots(QImage &img, bool drawDots = true);
    static void drawLines(QImage &img, aiBoard::Ptr board);
    static void drawBoxes(QImage &img, aiBoard::Ptr board);
};


#endif //KSQUARES_MLDATAGENERATOR_H

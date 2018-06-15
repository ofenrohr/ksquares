//
// Created by ofenrohr on 19.04.18.
//

#ifndef KSQUARES_MLIMAGEGENERATOR_H
#define KSQUARES_MLIMAGEGENERATOR_H


#include <QtCore/QArgument>
#include <KXmlGuiWindow>
#include <QImage>
#include <QSharedPointer>
#include <QList>

#include "datasets/DatasetGenerator.h"
#include "ui_mldataview.h"
#include "aicontroller.h"

namespace AlphaDots {
    class MLImageGenerator {

        static void drawBackgroundAndDots(QImage *img, bool drawDots = true);

        static void drawLineAt(QImage *img, int lineIdx, int w, int h);

        static void drawLines(QImage *img, QSharedPointer<aiBoard> board);

        static void drawBoxes(QImage *img, QSharedPointer<aiBoard> board);

    public:
        /**
         * Converts boxes (width or height) to pixels
         * @param boxes
         * @return
         */
        static int boxesToImgSize(int boxes);

        static QImage* generateInputImage(QSharedPointer<aiBoard> board);

        static QImage* generateOutputImage(QSharedPointer<aiBoard> board, QSharedPointer<KSquaresAi> ai, int *line=nullptr);

        static QImage* generateOutputImage(QSharedPointer<aiBoard> board, QList<int> lines, bool drawDots = false);

        /**
         * Define colors to use in images
         */
        static const int MLImageBackground = 0;
        static const int MLImageBoxA = 65;
        static const int MLImageBoxB = 150;
        static const int MLImageDot = 215;
        static const int MLImageLine = 255;
    };
}


#endif //KSQUARES_MLIMAGEGENERATOR_H

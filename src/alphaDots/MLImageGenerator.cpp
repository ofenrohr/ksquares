//
// Created by ofenrohr on 19.04.18.
//

#include "MLImageGenerator.h"
#include <QTimer>
#include <QImage>
#include <QRgb>
#include <QPoint>
#include "MLDataGenerator.h"
#include "board.h"
#include <QSharedPointer>
#include <QList>

using namespace AlphaDots;

void MLImageGenerator::drawBackgroundAndDots(QImage & img, bool drawDots, bool categorical) {
    int background = categorical ? MLImageBackgroundCat : MLImageBackground;
    int dot = categorical ? MLImageDotCat : MLImageDot;
    for (int y = 0; y < img.height(); y++) {
        for (int x = 0; x < img.width(); x++) {
            int color = background;

            if (x % 2 == 1 && y % 2 == 1 && drawDots) {
                color = dot;
            }

            img.setPixel(x,y, qRgb(color, color, color));
        }
    }
}

void MLImageGenerator::drawLineAt(QImage & img, int lineIdx, int w, int h, bool categorical) {
    int line = categorical ? MLImageLineCat : MLImageLine;
    QPoint p1,p2;
    if (!Board::indexToPoints(lineIdx, &p1, &p2, w, h, false)) {
        qDebug() << "fail!";
    }
    img.setPixel(p2.x()*2+(p2.y()-p1.y()), p2.y()*2+(p2.x()-p1.x()), qRgb(line, line, line));
}

void MLImageGenerator::drawLines(QImage & img, QSharedPointer < aiBoard > board, bool categorical) {
    for (int i = 0; i < board->linesSize; i++) {
        if (board->lines[i]) {
            drawLineAt(img, i, board->width, board->height, categorical);
        }
    }
}

void MLImageGenerator::drawBoxes(QImage & img, QSharedPointer < aiBoard > board, bool categorical) {
    int boxa = categorical ? MLImageBoxACat : MLImageBoxA;
    int boxb = categorical ? MLImageBoxBCat : MLImageBoxB;
    for (int i = 0; i < board->squareOwners.count(); i++) {
        int squareOwner = board->squareOwners[i];
        if (squareOwner >= 0) {
            int x,y,c;
            x = (i % board->width) * 2 + 2;
            y = (i / board->width) * 2 + 2;
            c = squareOwner == 0 ? boxa : boxb;
            img.setPixel(x,y, qRgb(c,c,c));
        }
    }
}

int MLImageGenerator::boxesToImgSize(int boxes) {
    return boxes * 2 + 3;
}

QImage MLImageGenerator::generateInputImage(QSharedPointer <aiBoard> board, bool categorical) {
    int imgWidth = boxesToImgSize(board->width); // 1px border
    int imgHeight = boxesToImgSize(board->height);

    QImage img(imgWidth, imgHeight, QImage::Format_ARGB32);

    drawBackgroundAndDots(img, true, categorical);
    drawLines(img, board, categorical);
    drawBoxes(img, board, categorical);

    return img;
}

QImage MLImageGenerator::generateOutputImage(aiBoard::Ptr board, QSharedPointer <KSquaresAi> ai, int *line) {
    int imgWidth = board->width*2+3; // 1px border
    int imgHeight = board->height*2+3;

    QImage img(imgWidth, imgHeight, QImage::Format_ARGB32);

    int nextLine = ai->chooseLine(board->linesAsList(), board->squareOwners, QList<Board::Move_t>());
    if (line != nullptr) {
        *line = nextLine;
    }

    drawBackgroundAndDots(img, false);
    drawLineAt(img, nextLine, board->width, board->height);

    return img;
}

QImage MLImageGenerator::generateOutputImage(aiBoard::Ptr board, QList<int> lines, bool drawDots) {
    int imgWidth = board->width*2+3; // 1px border
    int imgHeight = board->height*2+3;

    QImage img(imgWidth, imgHeight, QImage::Format_ARGB32);

    drawBackgroundAndDots(img, drawDots);
    foreach (int line, lines) {
        drawLineAt(img, line, board->width, board->height);
    }

    return img;
}
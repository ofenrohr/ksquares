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

void MLImageGenerator::drawBackgroundAndDots(QImage *img, bool drawDots) {
    for (int y = 0; y < img->height(); y++) {
        for (int x = 0; x < img->width(); x++) {
            int color = MLImageBackground;

            if (x % 2 == 1 && y % 2 == 1 && drawDots) {
                color = MLImageDot;
            }

            img->setPixel(x,y, qRgb(color, color, color));
        }
    }
}

void MLImageGenerator::drawLineAt(QImage *img, int lineIdx, int w, int h) {
    QPoint p1,p2;
    if (!Board::indexToPoints(lineIdx, &p1, &p2, w, h, false)) {
        qDebug() << "fail!";
    }
    img->setPixel(p2.x()*2+(p2.y()-p1.y()), p2.y()*2+(p2.x()-p1.x()), qRgb(MLImageLine, MLImageLine, MLImageLine));
}

void MLImageGenerator::drawLines(QImage *img, QSharedPointer < aiBoard > board) {
    for (int i = 0; i < board->linesSize; i++) {
        if (board->lines[i]) {
            drawLineAt(img, i, board->width, board->height);
        }
    }
}

void MLImageGenerator::drawBoxes(QImage *img, QSharedPointer < aiBoard > board) {
    for (int i = 0; i < board->squareOwners.count(); i++) {
        int squareOwner = board->squareOwners[i];
        if (squareOwner >= 0) {
            int x,y,c;
            x = (i % board->width) * 2 + 2;
            y = (i / board->width) * 2 + 2;
            c = squareOwner == 0 ? MLImageBoxA : MLImageBoxB;
            img->setPixel(x,y, qRgb(c,c,c));
        }
    }
}

int MLImageGenerator::boxesToImgSize(int boxes) {
    return boxes * 2 + 3;
}

QImage* MLImageGenerator::generateInputImage(QSharedPointer <aiBoard> board) {
    int imgWidth = boxesToImgSize(board->width); // 1px border
    int imgHeight = boxesToImgSize(board->height);

    QImage *img = new QImage(imgWidth, imgHeight, QImage::Format_ARGB32);

    drawBackgroundAndDots(img);
    drawLines(img, board);
    drawBoxes(img, board);

    return img;
}

QImage* MLImageGenerator::generateOutputImage(aiBoard::Ptr board, QSharedPointer <KSquaresAi> ai, int *line) {
    int imgWidth = board->width*2+3; // 1px border
    int imgHeight = board->height*2+3;

    QImage *img = new QImage(imgWidth, imgHeight, QImage::Format_ARGB32);

    int nextLine = ai->chooseLine(board->linesAsList(), board->squareOwners, QList<Board::Move_t>());
    if (line != nullptr) {
        *line = nextLine;
    }

    drawBackgroundAndDots(img, false);
    drawLineAt(img, nextLine, board->width, board->height);

    return img;
}

QImage* MLImageGenerator::generateOutputImage(aiBoard::Ptr board, QList<int> lines, bool drawDots) {
    int imgWidth = board->width*2+3; // 1px border
    int imgHeight = board->height*2+3;

    QImage *img = new QImage(imgWidth, imgHeight, QImage::Format_ARGB32);

    drawBackgroundAndDots(img, drawDots);
    foreach (int line, lines) {
        drawLineAt(img, line, board->width, board->height);
    }

    return img;
}
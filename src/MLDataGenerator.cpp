//
// Created by ofenrohr on 10/15/17.
//

#include <QtCore/QTimer>
#include "MLDataGenerator.h"
#include "aicontroller.h"

MLDataGenerator::MLDataGenerator() : KXmlGuiWindow(), m_view (new QLabel(QStringLiteral("Hello World!"))){
    setCentralWidget(m_view);
    QTimer::singleShot(0, this, &MLDataGenerator::initObject);
}

MLDataGenerator::~MLDataGenerator() {}

void MLDataGenerator::initObject() {
    m_view->setPixmap(QPixmap::fromImage(generateImage()).scaled(m_view->width(), m_view->height(), Qt::KeepAspectRatio));
}

QImage MLDataGenerator::generateImage() {

    // generate the board with auto fill
    int width = 5; // size in boxes
    int height = 4;
    int linesSize = aiFunctions::toLinesSize(width, height);
    bool *lines = new bool[linesSize];
    for (int i = 0; i < linesSize; i++) {
        lines[i] = false;
    }
    QList<int> squareOwners;
    for (int i = 0; i < width*height; i++) {
        squareOwners.append(-1);
    }
    aiBoard::Ptr board = aiBoard::Ptr(new aiBoard(lines, linesSize, width, height, squareOwners, 0, 1));
    QList<int> autoFillLines = aiController::autoFill(5, width, height);
    foreach (int line, autoFillLines) {
        board->doMove(line);
    }
    qDebug().noquote().nospace() << aiFunctions::boardToString(board);

    // generate the image
    int imgWidth = width*2+3; // 1px border
    int imgHeight = height*2+3;
    QImage img(imgWidth, imgHeight, QImage::Format_ARGB32);
    for (int y = 0; y < imgHeight; y++) {
        for (int x = 0; x < imgWidth; x++) {
            int color = 0;

            if (x % 2 == 1 && y % 2 == 1) {
                color = 255;
            }

            img.setPixel(x,y, qRgb(color, color, color));
        }
    }

    return img;
}
//
// Created by ofenrohr on 10/15/17.
//

#include <QtCore/QTimer>
#include "MLDataGenerator.h"
#include "aicontroller.h"
#include "aiEasyMediumHard.h"
#include "gameboardscene.h"
#include "gameboardview.h"

MLDataGenerator::MLDataGenerator() : KXmlGuiWindow(), m_view(new QWidget()) {
    gbs = NULL;

    setupUi(m_view);
    setCentralWidget(m_view);
    setupGUI();

    // next button
    connect(nextBtn, SIGNAL(clicked()), this, SLOT(nextBtnClicked()));

    QTimer::singleShot(0, this, &MLDataGenerator::initObject);
}

MLDataGenerator::~MLDataGenerator() {}

void MLDataGenerator::initObject() {

    // setup
    int width = 5;
    int height = 4;

    // draw stuff
    if (gbs != NULL) {
        delete gbs;
    }
    gbs = new GameBoardScene(5, 4, this);
    gameStateView->setScene(gbs);

    // generate data
    aiBoard::Ptr board = generateRandomBoard(width, height, 5);

    // make some more moves
    KSquaresAi::Ptr ai = KSquaresAi::Ptr(new aiEasyMediumHard(0, width, height, 2));
    makeAiMoves(board, ai, 20);

    // send board to game board view
    for (int i = 0; i < board->linesSize; i++) {
        if (board->lines[i]) {
            gbs->drawLine(i, QColor::fromRgb(0, 0, 0));
        }
    }
    for (int i = 0; i < board->width * board->height; i++) {
        if (board->squareOwners[i] >= 0) {
            gbs->drawSquare(i, board->squareOwners[i] == 0 ? QColor::fromRgb(255, 0, 0) : QColor::fromRgb(0, 0, 255));
        }
    }

    // print board
    qDebug().noquote().nospace() << aiFunctions::boardToString(board);

    inputImage = generateInputImage(board);
    outputImage = generateOutputImage(board, ai);

    inputLbl->setPixmap(QPixmap::fromImage(inputImage).scaled(inputLbl->width(), inputLbl->height(), Qt::KeepAspectRatio));
    outputLbl->setPixmap(QPixmap::fromImage(outputImage).scaled(outputLbl->width(), outputLbl->height(), Qt::KeepAspectRatio));
}

void MLDataGenerator::nextBtnClicked() {
    initObject();
}

aiBoard::Ptr MLDataGenerator::generateRandomBoard(int width, int height, int safeMoves) {
    // generate the board with auto fill
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
    QList<int> autoFillLines = aiController::autoFill(safeMoves, width, height);
    foreach (int line, autoFillLines) {
        board->doMove(line);
    }
    return board;
}

int MLDataGenerator::makeAiMove(aiBoard::Ptr board, KSquaresAi::Ptr ai) {
    int nextLine = ai->chooseLine(board->linesAsList(), board->squareOwners, QList<Board::Move>());//board->lines);
    board->doMove(nextLine);
    return nextLine;
}

QList<int> MLDataGenerator::makeAiMoves(aiBoard::Ptr board, KSquaresAi::Ptr ai, int freeLinesLeft) {
    QList<int> moves;
    while (aiFunctions::getFreeLines(board->lines, board->linesSize).count() > freeLinesLeft) {
        moves.append(makeAiMove(board, ai));
    }
    return moves;
}

void MLDataGenerator::drawBackgroundAndDots(QImage &img, bool drawDots) {
    for (int y = 0; y < img.height(); y++) {
        for (int x = 0; x < img.width(); x++) {
            int color = MLImageBackground;

            if (x % 2 == 1 && y % 2 == 1 && drawDots) {
                color = MLImageDot;
            }

            img.setPixel(x,y, qRgb(color, color, color));
        }
    }
}

void MLDataGenerator::drawLines(QImage &img, aiBoard::Ptr board) {
    for (int i = 0; i < board->linesSize; i++) {
        QPoint p1,p2;

        if (board->lines[i]) {
            if (!Board::indexToPoints(i, &p1, &p2, board->width, board->height, false)) {
                qDebug() << "fail!";
            }
            img.setPixel(p2.x()*2+(p2.y()-p1.y()), p2.y()*2+(p2.x()-p1.x()), qRgb(MLImageLine,MLImageLine,MLImageLine));
        }
    }
}

void MLDataGenerator::drawBoxes(QImage &img, aiBoard::Ptr board) {
    for (int i = 0; i < board->squareOwners.count(); i++) {
        int squareOwner = board->squareOwners[i];
        if (squareOwner >= 0) {
            int x,y,c;
            x = (i % board->width) * 2 + 2;
            y = (i / board->width) * 2 + 2;
            c = squareOwner == 0 ? MLImageBoxA : MLImageBoxB;
            img.setPixel(x,y, qRgb(c,c,c));
        }
    }
}

QImage MLDataGenerator::generateInputImage(aiBoard::Ptr board) {
    int imgWidth = board->width*2+3; // 1px border
    int imgHeight = board->height*2+3;

    QImage img(imgWidth, imgHeight, QImage::Format_ARGB32);

    drawBackgroundAndDots(img);
    drawLines(img, board);
    drawBoxes(img, board);

    return img;
}

QImage MLDataGenerator::generateOutputImage(aiBoard::Ptr board, KSquaresAi::Ptr ai) {
    int imgWidth = board->width*2+3; // 1px border
    int imgHeight = board->height*2+3;

    QImage img(imgWidth, imgHeight, QImage::Format_ARGB32);

    int nextLine = ai->chooseLine(board->linesAsList(), board->squareOwners, QList<Board::Move>());
    aiBoard::Ptr outputBoard = aiBoard::Ptr(new aiBoard(board->lines, board->linesSize, board->width, board->height, board->squareOwners, board->playerId, board->maxPlayerId, board->hashLines));
    outputBoard->clearAllMoves();
    outputBoard->doMove(nextLine);

    drawBackgroundAndDots(img, false);
    drawLines(img, outputBoard);

    return img;
}

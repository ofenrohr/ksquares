//
// Created by ofenrohr on 02.11.17.
//

#include "Dataset.h"

AlphaDots::Dataset::Dataset() {
    valid = false;
}

AlphaDots::Dataset::Dataset(const QImage &inputImg, const QImage &outputImg, const aiBoard::Ptr &board) :
    inputImg(inputImg),
    outputImg(outputImg),
    board(board)
{
    valid = true;
    isSeq = false;
    isSeq2 = false;
}

AlphaDots::Dataset::Dataset(const QList<QImage> &seq, const aiBoard::Ptr &brd) :
    sequence(seq),
    board(brd)
{
    valid = true;
    isSeq = true;
    isSeq2 = false;
}

AlphaDots::Dataset::Dataset(const QList<QImage> &inputSequence, const QList<QImage> &targetSequence,
                            const aiBoard::Ptr &board) :
    inputSequence(inputSequence),
    targetSequence(targetSequence),
    board(board)
{
    valid = true;
    isSeq = false;
    isSeq2 = true;
}

const QImage &AlphaDots::Dataset::getInputImg() const {
    return inputImg;
}

void AlphaDots::Dataset::setInputImg(const QImage &inputImg) {
    Dataset::inputImg = inputImg;
}

const QImage &AlphaDots::Dataset::getOutputImg() const {
    return outputImg;
}

void AlphaDots::Dataset::setOutputImg(const QImage &outputImg) {
    Dataset::outputImg = outputImg;
}

const aiBoard::Ptr &AlphaDots::Dataset::getBoard() const {
    return board;
}

void AlphaDots::Dataset::setBoard(const aiBoard::Ptr &board) {
    Dataset::board = board;
}

const bool AlphaDots::Dataset::isValid() const {
    return valid;
}

const bool AlphaDots::Dataset::isSequence() const {
    return isSeq;
}

const bool AlphaDots::Dataset::isTrainingSequence() const {
    return isSeq2;
}

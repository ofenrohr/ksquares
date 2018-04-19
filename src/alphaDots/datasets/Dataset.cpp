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
    isPV = false;
}

AlphaDots::Dataset::Dataset(const QList<QImage> &seq, const aiBoard::Ptr &brd) :
    sequence(seq),
    board(brd)
{
    valid = true;
    isSeq = true;
    isSeq2 = false;
    isPV = false;
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
    isPV = false;
}

AlphaDots::Dataset::Dataset(const QImage &inputImg, const QImage &outputImg, const double &outputVal,
                            const aiBoard::Ptr &board) :
    inputImg(inputImg),
    outputImg(outputImg),
    value(outputVal),
    board(board)
{
    valid = true;
    isSeq = false;
    isSeq2 = false;
    isPV = true;
}

const QImage &AlphaDots::Dataset::getInputImg() const {
    return inputImg;
}

const QImage &AlphaDots::Dataset::getOutputImg() const {
    return outputImg;
}

const aiBoard::Ptr &AlphaDots::Dataset::getBoard() const {
    return board;
}

const QList<QImage> &AlphaDots::Dataset::getSequence() const {
    return sequence;
}

const QList<QImage> &AlphaDots::Dataset::getInputSequence() const {
    return inputSequence;
}

const QList<QImage> &AlphaDots::Dataset::getTargetSequence() const {
    return targetSequence;
}

const double &AlphaDots::Dataset::getOutputVal() const {
    return value;
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

const bool AlphaDots::Dataset::isPolicyValue() const {
    return isPV;
}

//
// Created by ofenrohr on 02.11.17.
//

#ifndef KSQUARES_DATASET_H
#define KSQUARES_DATASET_H


#include <QtGui/QImage>
#include <aiBoard.h>

namespace AlphaDots {

    class Dataset {
    public:
        Dataset();
        Dataset(const QImage &inputImg, const QImage &outputImg, const aiBoard::Ptr &board);
        Dataset(const QList<QImage> &sequence, const aiBoard::Ptr &board);
        Dataset(const QList<QImage> &inputSequence, const QList<QImage> &targetSequence, const aiBoard::Ptr &board);
        Dataset(const QImage &inputImg, const QImage &outputImg, const double &outputVal, const aiBoard::Ptr &board);

        const QImage &getInputImg() const;
        const QImage &getOutputImg() const;
        const aiBoard::Ptr &getBoard() const;
        const QList<QImage> &getSequence() const;
        const QList<QImage> &getInputSequence() const;
        const QList<QImage> &getTargetSequence() const;
        const double &getOutputVal() const;

        const bool isValid() const;
        const bool isSequence() const;
        const bool isTrainingSequence() const;
        const bool isPolicyValue() const;

    protected:
        QImage inputImg;
        QImage outputImg;
        aiBoard::Ptr board;
        QList<QImage> sequence;
        QList<QImage> inputSequence;
        QList<QImage> targetSequence;
        double value;
        bool valid;
        bool isSeq;
        bool isSeq2;
        bool isPV; // isPolicyValue flag
    };
}


#endif //KSQUARES_DATASET_H

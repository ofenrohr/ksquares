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

        const QImage &getInputImg() const;
        void setInputImg(const QImage &inputImg);
        const QImage &getOutputImg() const;
        void setOutputImg(const QImage &outputImg);
        const aiBoard::Ptr &getBoard() const;
        void setBoard(const aiBoard::Ptr &board);
        const bool isValid() const;
        const bool isSequence() const;
        const bool isTrainingSequence() const;

    protected:
        QImage inputImg;
        QImage outputImg;
        aiBoard::Ptr board;
        QList<QImage> sequence;
        QList<QImage> inputSequence;
        QList<QImage> targetSequence;
        bool valid;
        bool isSeq;
        bool isSeq2;
    };
}


#endif //KSQUARES_DATASET_H

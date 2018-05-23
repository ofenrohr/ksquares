//
// Created by ofenrohr on 02.05.18.
//

#ifndef KSQUARES_SELFPLAY_H
#define KSQUARES_SELFPLAY_H

#include <KXmlGui/KXmlGuiWindow>
#include <alphaDots/ModelInfo.h>
#include "ui_SelfPlayForm.h"

namespace AlphaDots {
    class SelfPlay : public KXmlGuiWindow, public Ui::SelfPlayForm {
    Q_OBJECT
    public:
        SelfPlay(QString datasetDest, int threads, QString initialModel, int gamesPerIteration);

        void initObject();

        void updateInfo();

        void setupIteration();

        void finishIteration();

    public slots:
        void threadFinished(int thread);

    private:
        QWidget *m_view;

        // general configuration
        QString datasetDirectory;
        int threadCnt;
        QList<QPoint> availableBoardSizes;
        int iterationSize;

        // current state infos
        ModelInfo currentModel;
        QPoint currentBoardSize;
        int iteration;
        int gamesCompleted;
        QList<bool> threadRunning;

        // data container for one iteration
        std::vector<uint8_t> *input;
        std::vector<uint8_t> *output;
        std::vector<double> *value;
    };
}


#endif //KSQUARES_SELFPLAY_H

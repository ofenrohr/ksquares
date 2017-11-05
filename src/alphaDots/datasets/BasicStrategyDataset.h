//
// Created by ofenrohr on 05.11.17.
//

#ifndef KSQUARES_BASICSTRATEGYDATASET_H
#define KSQUARES_BASICSTRATEGYDATASET_H


#include <QtCore/QSharedPointer>
#include <alphaDots/ExternalProcess.h>
#include "DatasetGenerator.h"

namespace AlphaDots {
    /**
     * Generates examples to teach basic game strategies:
     * - take boxes that are available
     * - don''t make the third line of a box
     * Most notable change: All safe lines will be added as correct output - more than one correct line
     */
    class BasicStrategyDataset : public DatasetGenerator {
    public:
        typedef QSharedPointer<BasicStrategyDataset> Ptr;

        BasicStrategyDataset(bool gui, int width, int height);

        ~BasicStrategyDataset();

        Dataset generateDataset() override;

        void startConverter(int samples);

        void cleanup() override;

    protected:
        int width;
        int height;
        int linesSize;

        bool isGUI;
        int sampleIdx;

        ExternalProcess *converter;
    };
}


#endif //KSQUARES_BASICSTRATEGYDATASET_H

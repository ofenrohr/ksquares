//
// Created by ofenrohr on 29.10.17.
//

#ifndef KSQUARES_FIRSTTRYDATASET_H
#define KSQUARES_FIRSTTRYDATASET_H

#include "DatasetGenerator.h"


namespace AlphaDots {
    class FirstTryDataset : public DatasetGenerator {

    public:
        FirstTryDataset(int w, int h, QString outputDir);

        Dataset generateDataset() override;

        void startConverter(int examplesCnt, QString destinationDirectory) override {};

    private:
        int width;
        int height;
        QString output_dir;
    };
}


#endif //KSQUARES_FIRSTTRYDATASET_H

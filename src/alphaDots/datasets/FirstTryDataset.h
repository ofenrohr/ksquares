//
// Created by ofenrohr on 29.10.17.
//

#ifndef KSQUARES_FIRSTTRYDATASET_H
#define KSQUARES_FIRSTTRYDATASET_H

#include "DatasetGenerator.h"


class FirstTryDataset : public DatasetGenerator {

public:
    FirstTryDataset(int w, int h, QString outputDir);
    void generateDataset() override;

private:
    int width;
    int height;
    QString output_dir;
};


#endif //KSQUARES_FIRSTTRYDATASET_H

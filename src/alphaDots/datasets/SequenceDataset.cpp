//
// Created by ofenrohr on 01.12.17.
//

#include <alphaDots/MLDataGenerator.h>
#include "SequenceDataset.h"

SequenceDataset::SequenceDataset(bool gui, int width, int height) {
    isGUI = gui;
    this->width = width;
    this->height = height;
}

SequenceDataset::~SequenceDataset() {

}

Dataset SequenceDataset::generateDataset() {
    return Dataset();
}

void SequenceDataset::startConverter(int samples) {
    int widthImg = MLDataGenerator::boxesToImgSize(width);
    int heightImg = MLDataGenerator::boxesToImgSize(height);

    QStringList args;
    args << QStringLiteral("/home/ofenrohr/arbeit/master/code/alphaDots/datasetConverter/convert.py")
         << QStringLiteral("--zmq")
         << QString::number(samples)
         << QStringLiteral("--seq")
         << QStringLiteral("--output-file")
         << QStringLiteral("/run/media/ofenrohr/Data/AlphaDots/data/sequence") + QString::number(width) + QStringLiteral("x") + QString::number(height) + QStringLiteral(".npz")
         << QStringLiteral("--x-size")
         << QString::number(widthImg)
         << QStringLiteral("--y-size")
         << QString::number(heightImg);
    if (!isGUI) {
        converter = new ExternalProcess(QStringLiteral("/usr/bin/python2.7"), args);
        converter->startExternalProcess();
    }
}

void SequenceDataset::cleanup() {
    DatasetGenerator::cleanup();
}

//
// Created by ofenrohr on 27.05.18.
//

#include "ProtobufBatch.h"

AlphaDots::ProtobufBatch::ProtobufBatch(int batch_size) {
    this->batchSize = batch_size;
}

AlphaDots::PolicyValueData AlphaDots::ProtobufBatch::predict(AlphaDots::DotsAndBoxesImage input) {
    return AlphaDots::PolicyValueData();
}

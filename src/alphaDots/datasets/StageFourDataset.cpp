//
// Created by ofenrohr on 22.05.18.
//

#include <QtWidgets/QMessageBox>
#include <alphaDots/cnpy.h>
#include <QtCore/QDateTime>
#include <alphaDots/MLDataGenerator.h>
#include <alphaDots/MLImageGenerator.h>
#include <aiAlphaZeroMCTS.h>
#include <aiEasyMediumHard.h>
#include "StageFourDataset.h"


using namespace AlphaDots;

StageFourDataset::StageFourDataset(bool gui, int w, int h, QString modelName, int thread, int threads, int iteration) {
    isGUI = gui;
    width = w;
    height = h;

    sampleIdx = 0;
    threadIdx = thread;
    threadCnt = threads;
    selfPlayIteration = iteration;

    model = ProtobufConnector::getModelByName(modelName);

    // init GSLTest
    rng = gsl_rng_alloc(gsl_rng_taus);
    gsl_rng_set(rng, std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count());

    qDebug() << "StageFourDataset: ";
    qDebug() << " |-> thread id: " << threadIdx;
    qDebug() << " |-> threads: " << threadCnt;
    qDebug() << " |-> modelName: " << modelName;
    qDebug() << " |-> model: " << model.name();
    if (selfPlayIteration >= 0) {
        qDebug() << " |-> iteration: " << selfPlayIteration;
    }
}

StageFourDataset::~StageFourDataset() {
    cleanup();
}

void StageFourDataset::cleanup() {
    if (!isGUI) {
        //converter->stopExternalProcess();
    }
}

void StageFourDataset::startConverter(int samples, QString destinationDirectory) {
    sampleCnt = samples;
    destDir = destinationDirectory;

    int widthImg = MLImageGenerator::boxesToImgSize(width);
    int heightImg = MLImageGenerator::boxesToImgSize(height);
    dataSize = {
            sampleCnt,
            heightImg,
            widthImg
    };
    valueDataSize = {
            sampleCnt,
            1
    };
    input = new std::vector<uint8_t>(sampleCnt*heightImg*widthImg);
    policy = new std::vector<uint8_t>(sampleCnt*heightImg*widthImg);
    value = new std::vector<double>(sampleCnt);
}

bool StageFourDataset::stopConverter() {
    QString timeStr = QDateTime::currentDateTime().toString(QStringLiteral("-hh:mm-dd_MM_yyyy"));
    std::string filename =
            "/StageFour-" + model.name().toStdString() +
            (selfPlayIteration >= 0 ? "." + std::to_string(selfPlayIteration) : "") +
            "-" + std::to_string(sampleCnt) +
            "-" + std::to_string(width) + "x" + std::to_string(height) +
            timeStr.toStdString() +
            ".npz";
    //std::string filename = "/StageThree.npz";
    datasetPath = destDir + QString::fromStdString(filename);
    bool success = true;
    if (!cnpy::npz_save(destDir.toStdString()+filename, "input", &(*input)[0], dataSize, "w")) {
        QMessageBox::critical(nullptr, QObject::tr("Error"), QObject::tr("failed to save input data"));
        success = false;
    }
    if (!cnpy::npz_save(destDir.toStdString()+filename, "policy", &(*policy)[0], dataSize, "a")) {
        QMessageBox::critical(nullptr, QObject::tr("Error"), QObject::tr("failed to save policy data"));
        success = false;
    }
    if (!cnpy::npz_save(destDir.toStdString()+filename, "value", &(*value)[0], valueDataSize, "a")) {
        QMessageBox::critical(nullptr, QObject::tr("Error"), QObject::tr("failed to save value data"));
        success = false;
    }
    delete input;
    delete policy;
    delete value;
    return success;
}

Dataset StageFourDataset::generateDataset() {
    // create board
    aiBoard::Ptr board = aiBoard::Ptr(new aiBoard(width, height));//MLDataGenerator::generateRandomBoard(width, height, 15);

    // create ais
    KSquaresAi::Ptr alphaZeroAi = KSquaresAi::Ptr(new aiAlphaZeroMCTS(0, 1, width, height, 5000, model));// aiEasyMediumHard(0, width, height, 2));
    KSquaresAi::Ptr fastAi = KSquaresAi::Ptr(new aiEasyMediumHard(0, width, height, 2));

    // add hard ai moves
    int linesSize = aiFunctions::toLinesSize(width, height);
    double maxMoves = linesSize - 1;
    int movesLeft = gsl_ran_gaussian(rng, maxMoves/8.0) + maxMoves/2.0;
    MLDataGenerator::makeAiMoves(board, fastAi, movesLeft);

    int currentPlayer = board->playerId;

    // generate input image
    QImage inputImage = MLImageGenerator::generateInputImage(board);
    // output image is generated with AlphaZero MCTS
    int alphaZeroLine = -1;
    QImage outputImage = MLImageGenerator::generateOutputImage(board, alphaZeroAi, &alphaZeroLine);
    board->doMove(alphaZeroLine);

    // calculate value
    double val = 0;
    QList<int> extraLines;
    for (int i = 1; i < movesLeft; i++) {
        int line = fastAi->chooseLine(board->linesAsList(), board->squareOwners, QList<Board::Move_t>());
        board->doMove(line);
        extraLines.prepend(line);
    }
    for (const auto &owner: board->squareOwners) {
        val += owner == currentPlayer ? 1 : -1;
    }
    val /= board->squareOwners.size();

    // return gui dataset
    if (isGUI) {
        for (const auto &l: extraLines) {
            board->undoMove(l);
        }
        return Dataset(inputImage, outputImage, val, board);
    }

    // add to data
    int widthImg = MLImageGenerator::boxesToImgSize(width);
    int heightImg = MLImageGenerator::boxesToImgSize(height);
    if (input != nullptr && policy != nullptr && value != nullptr) {
        int sampleStart = (sampleIdx * threadCnt + threadIdx) * heightImg * widthImg;
        int valueSampleStart = sampleIdx * threadCnt + threadIdx;
        //qDebug() << "sampleStart: " << sampleStart;
        for (int y = 0; y < heightImg; y++) {
            for (int x = 0; x < widthImg; x++) {
                input->at(sampleStart + y * widthImg + x) = (uint8_t) inputImage.pixelColor(x,y).red();
                (*policy)[sampleStart + y * widthImg + x] = (uint8_t) outputImage.pixelColor(x,y).red();
                (*value)[valueSampleStart] = val;
            }
        }
    } else {
        qDebug() << "ERROR: input, policy or value is null";
    }

    //qDebug() << ".";
    sampleIdx++;

    return Dataset(inputImage, outputImage, val, board);
}
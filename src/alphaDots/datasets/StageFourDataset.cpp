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
#include <alphaDots/AlphaDotsExceptions.h>
#include "StageFourDataset.h"


using namespace AlphaDots;

double StageFourDataset::sigmaScale = 0.125; // overwritten in main.cpp
double StageFourDataset::meanScale = 0.5; // overwritten in main.cpp

StageFourDataset::StageFourDataset(bool gui, int w, int h, QString modelName, int thread, int threads, bool useMCTS,
        int aiLevel) {
    isGUI = gui;
    width = w;
    height = h;

    sampleIdx = 0;
    threadIdx = thread;
    threadCnt = threads;
    useMCTSai = useMCTS;
    fastAiLevel = aiLevel;

    model = ProtobufConnector::getInstance().getModelByName(modelName);

    // init GSLTest
    rng = gsl_rng_alloc(gsl_rng_taus);
    gsl_rng_set(rng, std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count());

    qDebug() << "StageFourDataset: ";
    qDebug() << " |-> thread id: " << threadIdx;
    qDebug() << " |-> threads: " << threadCnt;
    qDebug() << " |-> modelName: " << modelName;
    qDebug() << " |-> model: " << model.name();
    qDebug() << " |-> AI: " << aiFunctions::prettyAiLevel(useMCTSai ? KSquares::AI_MCTS_ALPHAZERO : fastAiLevel);
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
    startConverter(samples, destinationDirectory, true);
}

void StageFourDataset::startConverter(int samples, QString destinationDirectory, bool createData) {
    sampleCnt = samples;
    destDir = destinationDirectory;
    createdData = createData;

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
    if (createData) {
        input = new std::vector<uint8_t>(sampleCnt * heightImg * widthImg);
        policy = new std::vector<uint8_t>(sampleCnt * heightImg * widthImg);
        value = new std::vector<double>(sampleCnt);
    }
}

bool StageFourDataset::stopConverter() {
    QString timeStr = QDateTime::currentDateTime().toString(QStringLiteral("-hh:mm-dd_MM_yyyy"));
    QString filename =
            "/StageFour-" +
            model.name() +
            aiFunctions::prettyAiLevel(useMCTSai ? KSquares::AI_MCTS_ALPHAZERO : fastAiLevel) +
            (useMCTSai ?
                "-it" + QString::number(aiAlphaZeroMCTS::mcts_iterations) +
                "-dir" + QString::number(aiAlphaZeroMCTS::dirichlet_alpha) +
                "-c:" + QString::number(aiAlphaZeroMCTS::C_puct)
            : QString("-noMCTS") ) +
            "-" + QString::number(sampleCnt) +
            "-" + QString::number(width) + "x" + QString::number(height) +
            "-sig" + QString::number(sigmaScale) +
            "-mean" + QString::number(meanScale) +
            timeStr +
            ".npz";
    //std::string filename = "/StageThree.npz";
    datasetPath = destDir + filename;
    bool success = true;
    if (!cnpy::npz_save(destDir.toStdString()+filename.toStdString(), "input", &(*input)[0], dataSize, "w")) {
        QMessageBox::critical(nullptr, QObject::tr("Error"), QObject::tr("failed to save input data"));
        success = false;
    }
    if (!cnpy::npz_save(destDir.toStdString()+filename.toStdString(), "policy", &(*policy)[0], dataSize, "a")) {
        QMessageBox::critical(nullptr, QObject::tr("Error"), QObject::tr("failed to save policy data"));
        success = false;
    }
    if (!cnpy::npz_save(destDir.toStdString()+filename.toStdString(), "value", &(*value)[0], valueDataSize, "a")) {
        QMessageBox::critical(nullptr, QObject::tr("Error"), QObject::tr("failed to save value data"));
        success = false;
    }
    if (createdData) {
        delete input;
        delete policy;
        delete value;
    }
    return success;
}

Dataset StageFourDataset::generateDataset() {
    // create board
    aiBoard::Ptr board = aiBoard::Ptr(new aiBoard(width, height));//MLDataGenerator::generateRandomBoard(width, height, 15);

    // create fast ai
    KSquaresAi::Ptr fastAi = KSquaresAi::Ptr(new aiEasyMediumHard(0, width, height, fastAiLevel));

    // add hard ai moves
    int linesSize = aiFunctions::toLinesSize(width, height);
    double maxMoves = linesSize - 2;
    bool redo;
    int movesLeft;
    do {
        redo = false;
        movesLeft = gsl_ran_gaussian(rng, (double) maxMoves * sigmaScale) + (double) maxMoves * meanScale;
        if (movesLeft <= 0) {
            redo = true;
        }
        if (movesLeft >= maxMoves) {
            redo = true;
        }
    } while (redo);
    MLDataGenerator::makeAiMoves(board, fastAi, movesLeft);

    int currentPlayer = board->playerId;

    // create mcts ai with correct player id (important for correct value calculation in mcts)
    KSquaresAi::Ptr alphaZeroAi;
    aiAlphaZeroMCTS::Ptr realAlphaZeroAi;
    if (useMCTSai) {
        realAlphaZeroAi = aiAlphaZeroMCTS::Ptr(new aiAlphaZeroMCTS(currentPlayer, 1, width, height, 5000,
                                                                   model));// aiEasyMediumHard(0, width, height, 2));
        alphaZeroAi = realAlphaZeroAi;
    } else {
        alphaZeroAi = fastAi;
    }

    // generate input image
    QImage inputImage = MLImageGenerator::generateInputImage(board);
    // output image is generated with AlphaZero MCTS (or fast ai)
    QImage outputImage;
    int alphaZeroLine = -1;
    try {
        outputImage = MLImageGenerator::generateOutputImage(board, alphaZeroAi, &alphaZeroLine);
    } catch (InternalAiException &) {
        qDebug() << "InternalAiException";
        qDebug().noquote() << aiFunctions::boardToString(board);
        failRecursionLevel++;
        if (failRecursionLevel >= 100) {
            assert(false);
            return Dataset();
        }
        return generateDataset();
    }
    failRecursionLevel = 0;
    if (alphaZeroLine < 0) {
        qDebug() << "AI failed, dataset is tained!";
        QMessageBox::critical(nullptr, "KSquares Stage Four dataset error", "AI failed to produce valid line, creating dataset failed!");
        assert(false);
        return Dataset();
    }
    board->doMove(alphaZeroLine);

    // calculate value
    double val = 0;
    if (useMCTSai) {
        val = realAlphaZeroAi->lineValue();
    } else {
        QList<int> extraLines;
        // finish game with hard ai
        for (int i = 1; i < movesLeft; i++) {
            int line = fastAi->chooseLine(board->linesAsList(), board->squareOwners, QList<Board::Move_t>());
            board->doMove(line);
            extraLines.prepend(line);
        }
        // calculate value
        for (const auto &owner: board->squareOwners) {
            val += owner == currentPlayer ? 1 : -1;
        }
        val /= board->squareOwners.size();
        // undo extra moves
        for (const auto &l: extraLines) {
            board->undoMove(l);
        }
    }

    // return gui dataset
    if (isGUI) {
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
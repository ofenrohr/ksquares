//
// Created by ofenrohr on 11.03.18.
//

#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <qdebug.h>
#include "TestResultModel.h"

using namespace AlphaDots;

TestResultModel::TestResultModel(QObject *parent, QList<ModelInfo> *models, int gamesPerAi) :
        QAbstractTableModel(parent),
        modelList(models),
        gamesPerAiCnt(gamesPerAi)
{
    rows.reserve(modelList->size());
    histories.reserve(modelList->size());
    for (int i = 0; i < modelList->size(); i++) {
        QList<int> columns;
        for (int j = 0; j < columnCount(); j++) {
            columns.append(0);
        }
        rows.append(columns);
        histories.append(tr(""));
    }
}

TestResultModel::~TestResultModel() = default;

int TestResultModel::rowCount(const QModelIndex &parent) const {
    return modelList->size();
}

int TestResultModel::columnCount(const QModelIndex &parent) const {
    return 5;//7;
}

QVariant TestResultModel::headerData(int section, Qt::Orientation orientation, int role) const {
    QMutexLocker locker(&rowsMutex); // make it thread safe
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal) {
            switch(section) {
                case 0: return tr("Games");
                case 1: return tr("Wins vs. Easy \nin ").append(QString::number(gamesPerAiCnt)).append(tr(" games"));
                case 2: return tr("Wins vs. Medium \nin ").append(QString::number(gamesPerAiCnt)).append(tr(" games"));
                case 3: return tr("Wins vs. Hard \nin ").append(QString::number(gamesPerAiCnt)).append(tr(" games"));
                case 4: return tr("Errors");
                case 5: return tr("Ends with Double Dealing");
                case 6: return tr("Preemtive Sacrifices");
                default: return tr("UNDEFINED");
            }
        }
        if (orientation == Qt::Vertical) {
            return modelList->at(section).name();
        }
    }
    return QVariant();
}

QVariant TestResultModel::data(const QModelIndex &index, int role) const {
    QMutexLocker locker(&rowsMutex); // make it thread safe
    if (role == Qt::DisplayRole) {
        //return tr("(%1,%2)").arg(index.column()).arg(index.row());
        return rows[index.row()][index.column()];
    }
    return QVariant();
}

void TestResultModel::addResult(AITestResult result) {
    QMutexLocker locker(&rowsMutex); // make it thread safe
    results.append(result);
    int ruleBasedAi = -1;
    int modelAi = -1;
    int ruleBasedAiScore = -1;
    int modelAiScore = -1;
    if (result.setup.aiLevelP1 > result.setup.aiLevelP2) { // rule based ais are 0,1,2 (lowest levels)
        modelAi = result.setup.aiLevelP1;
        ruleBasedAi = result.setup.aiLevelP2;
        modelAiScore = result.scoreP1;
        ruleBasedAiScore = result.scoreP2;
    } else {
        modelAi = result.setup.aiLevelP2;
        ruleBasedAi = result.setup.aiLevelP1;
        modelAiScore = result.scoreP2;
        ruleBasedAiScore = result.scoreP1;
    }
    rows[modelAi-3][0]++; // inc games counter
    if (ruleBasedAiScore < modelAiScore) {
        rows[modelAi - 3][ruleBasedAi + 1]++; // inc win counter
    }
    if (result.taintedP1 || result.taintedP2) {
        rows[modelAi - 3][4]++; // inc error counter
    }
    // add line history
    histories[modelAi - 3] += tr("[");
    histories[modelAi - 3] += aiIndexToName(result.setup.aiLevelP1) + tr(" vs ") + aiIndexToName(result.setup.aiLevelP2) + tr(" on ");
    histories[modelAi - 3] += QString::number(result.setup.boardSize.x()) + tr("x") + QString::number(result.setup.boardSize.y());
    histories[modelAi - 3] += tr("|");
    for (int i = 0; i < result.moves.size(); i++) {
        if (i != 0) {
            histories[modelAi - 3] += tr("|");
        }
        histories[modelAi - 3] += QString::number(result.moves[i]);
    }
    histories[modelAi - 3] += tr("]");
    // update gui
    emit(dataChanged(createIndex(0,0),createIndex(rowCount(), columnCount())));
}

void TestResultModel::saveData(QString dest) {
    QString output;
    for (int y = 0; y < rowCount() + 1; y++) {
        for (int x = 0; x < columnCount() + 2; x++) { // +1 for column 'move history'
            QString cell;
            if (y == 0) {
                // columns that contain normal results
                // (first column is used for model names, last column for game histories)
                if (x != 0 && x < columnCount() + 1) {
                    cell = headerData(x-1, Qt::Horizontal, Qt::DisplayRole).toString();
                    output.append(tr(","));
                }
                if (x == columnCount() + 1) {
                    cell = tr("Move histories");
                    output.append(tr(","));
                }
            } else {
                if (x == 0) {
                    cell = headerData(y-1, Qt::Vertical, Qt::DisplayRole).toString();
                }
                else if (x == columnCount() + 1) {
                    cell = histories[y-1];
                    output.append(tr(","));
                } else {
                    cell = data(createIndex(y - 1, x - 1), Qt::DisplayRole).toString();
                    output.append(tr(","));
                }
            }
            output.append(cell.replace(tr("\n"), tr("")));
        }
        output.append(tr("\n"));
    }
    QFile outputFile(dest);
    if (!outputFile.open(QIODevice::ReadWrite)) {
        qDebug() << "failed to open output file!";
        return;
    }
    QTextStream outputStream(&outputFile);
    outputStream << output;

}

QString TestResultModel::aiIndexToName(int aiIndex) {
    switch (aiIndex) {
        case 0:
            return tr("Easy");
        case 1:
            return tr("Medium");
        case 2:
            return tr("Hard");
        default:
            return modelList->at(aiIndex - 3).name();
    }
}

//
// Created by ofenrohr on 11.03.18.
//

#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <qdebug.h>
#include "TestResultModel.h"

using namespace AlphaDots;

TestResultModel::TestResultModel(QObject *parent, QList<ModelInfo> *models, QList<ModelInfo> *adversaryModels, int gamesPerAi) :
        QAbstractTableModel(parent)/*,
        modelList(models),
        adverarialModels(adversaryModels),
        gamesPerAiCnt(gamesPerAi)*/
{
    reset(models, adversaryModels, gamesPerAi);
    /*
    rows.reserve(modelList->size());
    modelHistories.reserve(modelList->size());
    for (int i = 0; i < modelList->size(); i++) {
        QList<int> columns;
        for (int j = 0; j < columnCount(); j++) {
            columns.append(0);
        }
        rows.append(columns);
        modelHistories.append(tr(""));

        if (adverarialModels->contains(modelList->at(i))) {
            qDebug() << "WARNING: model list and adversary model list contain the same model! that's not supported";
        }
    }
     */
}

TestResultModel::~TestResultModel() = default;

void TestResultModel::reset(QList<ModelInfo> *models, QList<ModelInfo> *opponentModels, int gamesPerAi) {
    QMutexLocker locker(&rowsMutex); // make it thread safe
    results.clear();
    rows.clear();
    modelHistories.clear();
    modelList = models;
    adverarialModels = opponentModels;
    gamesPerAiCnt = gamesPerAi;

    rows.reserve(modelList->size());
    modelHistories.reserve(modelList->size());
    for (int i = 0; i < modelList->size(); i++) {
        QList<int> columns;
        for (int j = 0; j < columnCount(); j++) {
            columns.append(0);
        }
        rows.append(columns);
        modelHistories.append(tr(""));

        if (adverarialModels->contains(modelList->at(i))) {
            qDebug() << "WARNING: model list and adversary model list contain the same model! that's not supported";
        }
    }
    emit(headerDataChanged(Qt::Orientation::Horizontal, 0, columnCount()));
    emit(headerDataChanged(Qt::Orientation::Vertical, 0, rowCount()));
    emit(dataChanged(createIndex(0,0),createIndex(rowCount(), columnCount())));
}

int TestResultModel::rowCount(const QModelIndex &parent) const {
    return modelList->size();
}

int TestResultModel::columnCount(const QModelIndex &parent) const {
    return adverarialModels->size() + 2;//7;
}

QVariant TestResultModel::headerData(int section, Qt::Orientation orientation, int role) const {
    QMutexLocker locker(&rowsMutex); // make it thread safe
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal) {
            if (section == 0) return "Games";
            if (section == adverarialModels->count()+1) return "Errors";
            if (section > adverarialModels->count()+1) return "UNDEFINED";
            QString ret = "Wins vs " + adverarialModels->at(section-1).name() + " \nin "
                          + QString::number(gamesPerAiCnt) + " games";
            return ret;
                //case 1: return tr("Wins vs. Easy \nin ").append(QString::number(gamesPerAiCnt)).append(tr(" games"));
                //case 2: return tr("Wins vs. Medium \nin ").append(QString::number(gamesPerAiCnt)).append(tr(" games"));
                //case 3: return tr("Wins vs. Hard \nin ").append(QString::number(gamesPerAiCnt)).append(tr(" games"));
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

int TestResultModel::rawData(const int row, const int column) const {
    return rows[row][column];
}

void TestResultModel::addResult(AITestResult result) {
    QMutexLocker locker(&rowsMutex); // make it thread safe
    results.append(result);
    int columnAi = -1; // adversaryModelList (negative aiLevelP1/2)
    int rowAi = -1; // modelList (positive aiLevelP1/2)
    int columnAiScore = -1;
    int rowAiScore = -1;
    //if (result.setup.aiLevelP1 > result.setup.aiLevelP2) { // rule based ais are 0,1,2 (lowest levels)
    if (result.setup.aiLevelP1 > 0) {
        rowAi = result.setup.aiLevelP1 - 1;
        columnAi = -result.setup.aiLevelP2;
        rowAiScore = result.scoreP1;
        columnAiScore = result.scoreP2;
    } else {
        rowAi = result.setup.aiLevelP2 - 1;
        columnAi = -result.setup.aiLevelP1;
        rowAiScore = result.scoreP2;
        columnAiScore = result.scoreP1;
    }
    rows[rowAi][0]++; // inc games counter
    if (columnAiScore < rowAiScore) {
        rows[rowAi][columnAi]++; // inc win counter
    }
    if (result.taintedP1 || result.taintedP2) {
        rows[rowAi][columnCount()-2]++; // inc error counter
    }
    rows[rowAi][columnCount()-2] += result.crashesP1 + result.crashesP2;
    // add line history
    QString hist = "|";
    hist += aiIndexToName(result.setup.aiLevelP1) + " vs " + aiIndexToName(result.setup.aiLevelP2) + " | ";
    hist += QString::number(result.scoreP1) + ":" + QString::number(result.scoreP2) + " | ";
    hist += QString::number(result.setup.boardSize.x()) + "x" + QString::number(result.setup.boardSize.y());
    hist += " | ";

    for (int i = 0; i < result.autoFillMoves.size(); i++) {
        if (i != 0) {
            hist += ", ";
        }
        hist += QString::number(result.autoFillMoves[i]);
    }
    hist+= " | ";

    bool first = true;
    for (int i = 0; i < result.moves.size(); i++) {
        if (result.autoFillMoves.contains(result.moves[i])) {
            continue;
        }
        if (!first) {
            hist += ", ";
        } else {
            first = false;
        }
        hist += QString::number(result.moves[i]);
    }
    hist += "|";

    gameHistories.append(hist);
    modelHistories[rowAi] += hist;

    // update gui
    emit(dataChanged(createIndex(0,0),createIndex(rowCount(), columnCount())));
}

QString TestResultModel::dataToString(bool includeHistory) {
    QString output;
    int cols = columnCount() + (includeHistory ? 2 : 1);
    for (int y = 0; y < rowCount() + 1; y++) {
        for (int x = 0; x < cols; x++) { // +1 for column 'move history'
            // add cell input
            QString cell;
            if (y == 0) {
                // columns that contain normal results
                // (first column is used for model names, last column for game modelHistories)
                if (x == 0) {
                    cell = tr("|Model");
                }
                if (x != 0) {
                    if (includeHistory && x == cols - 1) {
                        cell = tr("Move modelHistories");
                        output.append(tr("|"));
                    } else {
                        cell = headerData(x - 1, Qt::Horizontal, Qt::DisplayRole).toString();
                        output.append(tr("|"));
                    }
                }
            } else {
                if (x == 0) {
                    cell = headerData(y-1, Qt::Vertical, Qt::DisplayRole).toString();
                }
                else if (includeHistory && x == columnCount() + 1) {
                    cell = modelHistories[y-1];
                    output.append(tr("|"));
                } else {
                    cell = data(createIndex(y - 1, x - 1), Qt::DisplayRole).toString();
                    output.append(tr("|"));
                }
            }
            // add the cell
            output.append(cell.replace(tr("\n"), tr("")));
            // add separator row after header
            if (y == 0 && x == cols - 1) {
                output.append(tr("|\n"));
                for (int i = 0; i < cols; i++) {
                    output.append(tr("|---"));
                }
            }
        }
        output.append(tr("|\n"));
    }
    return output;
}

void TestResultModel::saveData(QString dest) {
    QString output = dataToString(true);
    QFile outputFile(dest);
    if (!outputFile.open(QIODevice::ReadWrite)) {
        qDebug() << "failed to open output file!";
        return;
    }
    QTextStream outputStream(&outputFile);
    outputStream << output;

}

QString TestResultModel::aiIndexToName(int aiIndex) {
    if (aiIndex < 0) {
        return adverarialModels->at(-aiIndex - 1).name();
    } else {
        return modelList->at(aiIndex - 1).name();
    }
}

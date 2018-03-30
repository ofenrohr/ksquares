//
// Created by ofenrohr on 11.03.18.
//

#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <qdebug.h>
#include "TestResultModel.h"

using namespace AlphaDots;

TestResultModel::TestResultModel(QObject *parent, QList<ModelInfo> models) :
        QAbstractTableModel(parent),
        modelList(models)
{
    for (int i = 0; i < modelList.size(); i++) {
        QList<int> columns;
        for (int j = 0; j < columnCount(); j++) {
            columns.append(0);
        }
        rows.append(columns);
    }
}

TestResultModel::~TestResultModel() = default;

int TestResultModel::rowCount(const QModelIndex &parent) const {
    return modelList.size();
}

int TestResultModel::columnCount(const QModelIndex &parent) const {
    return 7;
}

QVariant TestResultModel::headerData(int section, Qt::Orientation orientation, int role) const {
    QMutexLocker locker(&rowsMutex); // make it thread safe
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal) {
            switch(section) {
                case 0: return QStringLiteral("Games");
                case 1: return QStringLiteral("Wins vs. Easy");
                case 2: return QStringLiteral("Wins vs. Medium");
                case 3: return QStringLiteral("Wins vs. Hard");
                case 4: return QStringLiteral("Errors");
                case 5: return QStringLiteral("Ends with Double Dealing");
                case 6: return QStringLiteral("Preemtive Sacrifices");
            }
        }
        if (orientation == Qt::Vertical) {
            return modelList[section].name();
        }
    }
    return QVariant();
}

QVariant TestResultModel::data(const QModelIndex &index, int role) const {
    QMutexLocker locker(&rowsMutex); // make it thread safe
    if (role == Qt::DisplayRole) {
        //return QStringLiteral("(%1,%2)").arg(index.column()).arg(index.row());
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
    if (result.setup.levelP1 > result.setup.levelP2) {
        modelAi = result.setup.levelP1;
        ruleBasedAi = result.setup.levelP2;
        modelAiScore = result.scoreP1;
        ruleBasedAiScore = result.scoreP2;
    } else {
        modelAi = result.setup.levelP2;
        ruleBasedAi = result.setup.levelP1;
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
    emit(dataChanged(createIndex(0,0),createIndex(rowCount(), columnCount())));
}

void TestResultModel::saveData() {
    QString output;
    for (int y = 0; y < rowCount()+1; y++) {
        for (int x = 0; x < columnCount() + 1; x++) {
            QString cell;
            if (y == 0) {
                if (x != 0) {
                    cell = headerData(x-1, Qt::Horizontal, Qt::DisplayRole).toString();
                    output.append(QStringLiteral(","));
                }
            } else {
                if (x == 0) {
                    cell = headerData(y-1, Qt::Vertical, Qt::DisplayRole).toString();
                } else {
                    cell = data(createIndex(y - 1, x - 1), Qt::DisplayRole).toString();
                    output.append(QStringLiteral(","));
                }
            }
            output.append(cell);
        }
        output.append(QStringLiteral("\n"));
    }
    QFile outputFile(QStringLiteral("ModelEvaluation.csv"));
    if (!outputFile.open(QIODevice::ReadWrite)) {
        qDebug() << "failed to open output file!";
        return;
    }
    QTextStream outputStream(&outputFile);
    outputStream << output;

}

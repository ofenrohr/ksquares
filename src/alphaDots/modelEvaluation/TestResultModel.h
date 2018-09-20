//
// Created by ofenrohr on 11.03.18.
//

#ifndef KSQUARES_TESTRESULTMODEL_H
#define KSQUARES_TESTRESULTMODEL_H


#include <QtCore/QAbstractTableModel>
#include <alphaDots/ModelInfo.h>
#include <QtCore/QMutex>
#include "AITestResult.h"

namespace AlphaDots {
    // http://doc.qt.io/qt-5/modelview.html
    class TestResultModel : public QAbstractTableModel {
    Q_OBJECT
    public:
        TestResultModel(QObject *parent, QList<ModelInfo> *models, QList<ModelInfo> *adversaryModels, int gamesPerAi);

        ~TestResultModel();

        /**
         * Number of rows in result table
         * @param parent
         * @return
         */
        int rowCount(const QModelIndex &parent = QModelIndex()) const override;
        /**
         * Number of columns in result table
         * @param parent
         * @return
         */
        int columnCount(const QModelIndex &parent = QModelIndex()) const override;
        /**
         * Data for cells in result table
         * @param index
         * @param role
         * @return
         */
        QVariant data(const QModelIndex &index, int role) const override;
        /**
         * Captions for rows and columns
         * @param section
         * @param orientation
         * @param role
         * @return
         */
        QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

        /**
         * Add a game result to the table data.
         * @param result
         */
        void addResult(AITestResult result);

        /**
         * Exports the data as markdown table.
         * @param includeMoveHistories include the move histories in the table?
         * @return QString of the data in markdown format
         */
        QString dataToString(bool includeMoveHistories);

        /**
         * Save data
         * @param dest destination file path
         */
        void saveData(QString dest);

        /**
         * Returns the name of the ai/model. 0,1,2 = Easy,Medium,Hard, 3,...,n = Model name
         * @param aiIndex
         * @return name of ai/model
         */
        QString aiIndexToName(int aiIndex);

        /**
         * Returns the line histories of all games
         * @return line histories of all games.
         */
        QList<QString> getHistories() { return gameHistories; }

    private:
        QList<ModelInfo> *modelList;
        QList<ModelInfo> *adverarialModels;

        QList<AITestResult> results;
        QList<QList<int>> rows;
        QList<QString> modelHistories;
        QList<QString> gameHistories;
        int gamesPerAiCnt;

        mutable QMutex rowsMutex;
    };
}


#endif //KSQUARES_TESTRESULTMODEL_H

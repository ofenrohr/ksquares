//
// Created by ofenrohr on 11.03.18.
//

#ifndef KSQUARES_TESTRESULTMODEL_H
#define KSQUARES_TESTRESULTMODEL_H


#include <QtCore/QAbstractTableModel>
#include <alphaDots/ModelInfo.h>
#include "AITestResult.h"

namespace AlphaDots {
    // http://doc.qt.io/qt-5/modelview.html
    class TestResultModel : public QAbstractTableModel {
    Q_OBJECT
    public:
        TestResultModel(QObject *parent, QList<ModelInfo> models);

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
    private:
        QList<ModelInfo> modelList;

        QList<AITestResult> results;
        QList<QList<int>> rows;
    };
}


#endif //KSQUARES_TESTRESULTMODEL_H

//
// Created by ofenrohr on 24.09.18.
//

#include "ReportLogger.h"

#include <QDebug>
#include <QMutexLocker>
#include <QStringBuilder>

using namespace AlphaDots;

ReportLogger::ReportLogger(QString &filepath) {
    reportFilePath = filepath;
}

void ReportLogger::log(QString message) {
    QMutexLocker locker(&logMutex);
    QFile reportFile(reportFilePath);
    if (!reportFile.open(QIODevice::WriteOnly | QIODevice::Append)) {
        qDebug() << "ERROR: failed to open (create) report file " << reportFilePath;
        QTextStream report(stdout, QIODevice::WriteOnly);
        report << message;
    } else {
        QTextStream report(&reportFile);
        report << message;
    }
}

void ReportLogger::logTable(QList<QList<QString>> table) {
    // each column is at least minWidth chars wide
    int minWidth = 5;

    int rowCnt = table.size();
    int colCnt = 0;
    QList<int> colWidth;
    for (const auto &row : table) {
        colCnt = std::max(colCnt, row.size());
    }
    for (int c = 0; c < colCnt; c++) {
        colWidth.append(5);
    }
    int r = 0;
    for (const auto &row : table) {
        int c = 0;
        for (const auto &col : row) {
            for (const auto &colRow : col.split("\n")) {
                colWidth[c] = std::max(colWidth[c], colRow.length());
            }
            c++;
        }
        r++;
    }
    QString separatorRow = "+";
    for (int c = 0; c < colCnt; c++) {
        separatorRow += QString("-").repeated(colWidth[c]) + "+";
    }
    log("\n\n" + separatorRow + "\n");
    for (const auto &row : table) {
        int maxColRowCnt = 1;
        for (const auto &col : row) {
            maxColRowCnt = std::max(maxColRowCnt, col.split("\n").length());
        }
        QList<QList<QString>> subtable;
        for (int r = 0; r < maxColRowCnt; r++) {
            QList<QString> subrow;
            for (int c = 0; c < colCnt; c++) {
                subrow.append("");
            }
            subtable.append(subrow);
        }
        for (int c = 0; c < colCnt; c++) {
            const auto subrows = row[c].split("\n");
            for (int r = 0; r < subrows.size(); r++) {
                subtable[r][c] = subrows[r];
            }
        }
        for (const auto &subrow : subtable) {
            QString rowStr = "|";
            for (int c = 0; c < colCnt; c++) {
                rowStr += QString("%1|").arg(subrow[c], -colWidth[c]);
            }
            log(rowStr+"\n");
        }

        log(separatorRow+"\n");
    }
    log("\n\n");
}
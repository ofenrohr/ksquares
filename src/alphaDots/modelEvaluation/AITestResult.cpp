//
// Created by ofenrohr on 11.03.18.
//

#include "AITestResult.h"

QVariant AITestResult::toQVariant()
{
    QVariantMap map;
    map[QStringLiteral("setup")] = setup.toQVariant();
    QVariantList moveList;
    for (int i = 0; i < moves.size(); i++)
        moveList << moves[i];
    map[QStringLiteral("moves")] = moveList;
    QVariantList timeP1List;
    for (int i = 0; i < timeP1.size(); i++)
        timeP1List << timeP1[i];
    map[QStringLiteral("timeP1")] = timeP1List;
    QVariantList timeP2List;
    for (int i = 0; i < timeP2.size(); i++)
        timeP2List << timeP2[i];
    map[QStringLiteral("timeP2")] = timeP2List;
    map[QStringLiteral("taintedP1")] = taintedP1;
    map[QStringLiteral("taintedP2")] = taintedP2;
    map[QStringLiteral("scoreP1")] = scoreP1;
    map[QStringLiteral("scoreP2")] = scoreP2;
    map[QStringLiteral("crashesP1")] = crashesP1;
    map[QStringLiteral("crashesP2")] = crashesP2;
    return map;
}

void AITestResult::fromQVariant(QVariant var)
{
    QVariantMap map = var.toMap();
    setup.fromQVariant(map[QStringLiteral("setup")]);
    moves.clear();
    QVariantList moveList = map[QStringLiteral("moves")].toList();
    for (int i = 0; i < moveList.size(); i++)
        moves.append(moveList[i].toInt());
    timeP1.clear();
    QVariantList timeP1List = map[QStringLiteral("timeP1")].toList();
    for (int i = 0; i < timeP1List.size(); i++)
        timeP1.append(timeP1List[i].toInt());
    timeP2.clear();
    QVariantList timeP2List = map[QStringLiteral("timeP2")].toList();
    for (int i = 0; i < timeP2List.size(); i++)
        timeP2.append(timeP2List[i].toInt());
    taintedP1 = map[QStringLiteral("taintedP1")].toBool();
    taintedP2 = map[QStringLiteral("taintedP2")].toBool();
    scoreP1 = map[QStringLiteral("scoreP1")].toInt();
    scoreP2 = map[QStringLiteral("scoreP2")].toInt();
    if (map.contains(QStringLiteral("crashesP1")) && map.contains(QStringLiteral("crashesP2")))
    {
        crashesP1 = map[QStringLiteral("crashesP1")].toInt();
        crashesP2 = map[QStringLiteral("crashesP2")].toInt();
    }
    else
    {
        crashesP1 = 0;
        crashesP2 = 0;
    }
}

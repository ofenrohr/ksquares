//
// Created by ofenrohr on 11.03.18.
//

#include "AITestSetup.h"


QVariant AITestSetup::toQVariant()
{
    QVariantMap map;
    map[QStringLiteral("aiLevelP1")] = aiLevelP1;
    map[QStringLiteral("aiLevelP2")] = aiLevelP2;
    map[QStringLiteral("timeout")] = timeout;
    QVariantMap boardSizeMap;
    boardSizeMap[QStringLiteral("width")] = boardSize.x();
    boardSizeMap[QStringLiteral("height")] = boardSize.y();
    map[QStringLiteral("boardSize")] = boardSizeMap;
    return map;
}

void AITestSetup::fromQVariant(QVariant var)
{
    QVariantMap map = var.toMap();
    aiLevelP1 = map[QStringLiteral("aiLevelP1")].toInt();
    aiLevelP2 = map[QStringLiteral("aiLevelP2")].toInt();
    timeout = map[QStringLiteral("timeout")].toInt();
    boardSize = QPoint();
    boardSize.setX(map[QStringLiteral("boardSize")].toMap()[QStringLiteral("width")].toInt());
    boardSize.setY(map[QStringLiteral("boardSize")].toMap()[QStringLiteral("height")].toInt());
}
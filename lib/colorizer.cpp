/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#include "colorizer.h"
#include <QMap>
#include <QRandomGenerator>

QColor Colorizer::color(const QString &key, COLOR_TYPE type)
{
    static QRandomGenerator sFixedSeedGenerator{1}; // used fixed seed to ensure that colors for same units never change
    static QMap<QString, std::pair<QColor, QColor>> sUnitToColorMap;
    if (!sUnitToColorMap.contains(key)) {
        int hue = sFixedSeedGenerator.bounded(255);
        QColor foreground = QColor::fromHsl(hue, 220, 150);
        QColor background = QColor::fromHsl(hue, 200, 220);
        sUnitToColorMap[key] = std::make_pair(foreground, background);
    }
    if (type == COLOR_TYPE::FOREGROUND) {
        return sUnitToColorMap.value(key).first;
    } else {
        return sUnitToColorMap.value(key).second;
    }
}

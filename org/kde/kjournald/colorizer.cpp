/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#include "colorizer.h"
#include <QMap>
#include <QRandomGenerator>

struct LineColor {
    QColor foreground;
    QColor background;
};

QColor Colorizer::color(const QString &key, COLOR_TYPE type)
{
    QColor color;
    static QRandomGenerator sFixedSeedGenerator{1}; // used fixed seed to ensure that colors for same units never change
    static QMap<QString, LineColor> sUnitToColorMap;

    auto needle = sUnitToColorMap.constFind(key);
    if (needle != sUnitToColorMap.cend()) {
        color = type == COLOR_TYPE::FOREGROUND ? needle->foreground : needle->background;
    } else {
        int hue = sFixedSeedGenerator.bounded(255);
        QColor foreground = QColor::fromHsl(hue, 220, 150);
        QColor background = QColor::fromHsl(hue, 200, 220);
        sUnitToColorMap[key] = {foreground, background};
        color = type == COLOR_TYPE::FOREGROUND ? foreground : background;
    }
    return color;
}

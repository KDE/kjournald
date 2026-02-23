/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#include "colorizer.h"
#include <QMap>
#include <string_view>

struct LineColor {
    QColor foreground;
    QColor background;
};

QColor Colorizer::color(const QString &key, COLOR_TYPE type)
{
    QColor color;
    static QMap<QString, LineColor> sUnitToColorMap;

    auto needle = sUnitToColorMap.constFind(key);
    if (needle != sUnitToColorMap.cend()) {
        color = type == COLOR_TYPE::FOREGROUND ? needle->foreground : needle->background;
    } else {
        // uniformly project value into size_t area, then map to [0,255]
        quint32 hue = std::hash<std::string_view>{}(key.toStdString().c_str()) % 256;
        QColor foreground = QColor::fromHsl(hue, 220, 150);
        QColor background = QColor::fromHsl(hue, 200, 220);
        sUnitToColorMap[key] = {foreground, background};
        color = type == COLOR_TYPE::FOREGROUND ? foreground : background;
    }
    return color;
}

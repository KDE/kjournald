/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#ifndef COLORIZER_H
#define COLORIZER_H

#include "kjournald_export.h"
#include <QColor>
#include <QString>

class KJOURNALD_EXPORT Colorizer
{
public:
    enum class COLOR_TYPE {
        FOREGROUND,
        BACKGROUND,
    };

    static QColor color(const QString &key, COLOR_TYPE = COLOR_TYPE::FOREGROUND);
};

#endif

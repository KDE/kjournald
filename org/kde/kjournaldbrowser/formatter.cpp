/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2025 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#include "formatter.h"
#include <QDateTime>
#include <QString>

Formatter::Formatter(QObject *parent)
    : QObject(parent)
{
}

QString Formatter::formatTime(const QDateTime &datetime, bool utc) const
{
    if (utc) {
        return datetime.toUTC().time().toString(QLatin1String("HH:mm:ss.zzz"));
    } else {
        return datetime.time().toString(QLatin1String("HH:mm:ss.zzz"));
    }
}

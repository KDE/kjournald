/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2025 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#ifndef FORMATTER_H
#define FORMATTER_H

#include <QObject>
#include <QQmlEngine>

class Formatter : public QObject
{
    Q_OBJECT

    QML_ELEMENT
    QML_SINGLETON
public:
    explicit Formatter(QObject *parent = nullptr);

    /**
     * @brief Format time into string
     * @param datetime the datetime object
     * @param utc if set to true, the string will be UTC time, otherwise according to the current local
     * @return formatted string
     */
    Q_INVOKABLE QString formatTime(const QDateTime &datetime, bool utc) const;
};

#endif // FORMATTER_H

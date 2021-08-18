/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#ifndef BOOT_MODEL_PRIVATE_H
#define BOOT_MODEL_PRIVATE_H

#include "journaldhelper.h"

class BootModelPrivate
{
public:
    enum class TIME_FORMAT {
        UTC,
        LOCALTIME,
    };

    using BootInfo = JournaldHelper::BootInfo;

    explicit BootModelPrivate(std::unique_ptr<IJournal> journal);

    static QString prettyPrintBoot(const BootInfo &bootInfo, TIME_FORMAT format);

    QVector<BootInfo> mBootInfo;
    std::unique_ptr<IJournal> mJournal;
};

QString BootModelPrivate::prettyPrintBoot(const BootInfo &bootInfo, TIME_FORMAT format)
{
    const QString id = bootInfo.mBootId.left(10);
    QString sinceTime;
    QString sinceDate;
    QString untilTime;
    QString untilDate;

    if (format == TIME_FORMAT::UTC) {
        sinceTime = bootInfo.mSince.toUTC().toString("hh:mm");
        sinceDate = bootInfo.mSince.toUTC().toString("yyyy-MM-dd");
        untilTime = bootInfo.mUntil.toUTC().toString("hh:mm");
        untilDate = bootInfo.mUntil.toUTC().toString("yyyy-MM-dd");
    } else {
        sinceTime = bootInfo.mSince.toString("hh:mm");
        sinceDate = bootInfo.mSince.toString("yyyy-MM-dd");
        untilTime = bootInfo.mUntil.toString("hh:mm");
        untilDate = bootInfo.mUntil.toString("yyyy-MM-dd");
    }
    if (sinceDate == untilDate) {
        return QString("%1 %2-%3 [%4...]").arg(sinceDate, sinceTime, untilTime, id);
    } else {
        return QString("%1 %2-%3 %4 [%5...]").arg(sinceDate, sinceTime, untilDate, untilTime, id);
    }
}

#endif // JOURNAL_H

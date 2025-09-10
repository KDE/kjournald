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
    static QString prettyPrintBoot(const BootInfo &bootInfo, TIME_FORMAT format);
    void sort(Qt::SortOrder order);

    QVector<BootInfo> mBootInfo;
    IJournalProvider *mJournalProvider{nullptr};
    std::unique_ptr<SdJournal> mJournal;
};

QString BootModelPrivate::prettyPrintBoot(const BootInfo &bootInfo, TIME_FORMAT format)
{
    const QString id = bootInfo.mBootId.left(10);
    QString sinceTime;
    QString sinceDate;
    QString untilTime;
    QString untilDate;

    if (format == TIME_FORMAT::UTC) {
        sinceTime = bootInfo.mSince.toUTC().toString(QLatin1String("hh:mm"));
        sinceDate = bootInfo.mSince.toUTC().toString(QLatin1String("yyyy-MM-dd"));
        untilTime = bootInfo.mUntil.toUTC().toString(QLatin1String("hh:mm"));
        untilDate = bootInfo.mUntil.toUTC().toString(QLatin1String("yyyy-MM-dd"));
    } else {
        sinceTime = bootInfo.mSince.toString(QLatin1String("hh:mm"));
        sinceDate = bootInfo.mSince.toString(QLatin1String("yyyy-MM-dd"));
        untilTime = bootInfo.mUntil.toString(QLatin1String("hh:mm"));
        untilDate = bootInfo.mUntil.toString(QLatin1String("yyyy-MM-dd"));
    }
    if (sinceDate == untilDate) {
        return QString(QLatin1String("%1 %2-%3 [%4...]")).arg(sinceDate, sinceTime, untilTime, id);
    } else {
        return QString(QLatin1String("%1 %2-%3 %4 [%5...]")).arg(sinceDate, sinceTime, untilDate, untilTime, id);
    }
}

void BootModelPrivate::sort(Qt::SortOrder order)
{
    std::sort(std::begin(mBootInfo), std::end(mBootInfo), [order](const BootInfo &left, const BootInfo &right) {
        if (order == Qt::AscendingOrder) {
            return left.mSince <= right.mSince;
        } else {
            return left.mSince > right.mSince;
        }
    });
}

#endif // JOURNAL_H

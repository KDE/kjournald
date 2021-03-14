/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#include "journaldhelper.h"
#include <QDebug>
#include <systemd/sd-journal.h>

QVector<QString> JournaldHelper::queryUnique(const Journal &journal, Field field)
{
    QVector<QString> dataList;
    const void *data;
    size_t length;
    int result;

    std::string fieldString;
    switch (field) {
    case Field::MESSAGE:
        fieldString = "MESSAGE";
        break;
    case Field::_BOOT_ID:
        fieldString = "_BOOT_ID";
        break;
    case Field::_SYSTEMD_CGROUP:
        fieldString = "_SYSTEMD_CGROUP";
        break;
    case Field::_SYSTEMD_OWNER_UID:
        fieldString = "_SYSTEMD_OWNER_UID";
        break;
    case Field::_SYSTEMD_SESSION:
        fieldString = "_SYSTEMD_SESSION";
        break;
    case Field::_SYSTEMD_SLICE:
        fieldString = "_SYSTEMD_SLICE";
        break;
    case Field::_SYSTEMD_UNIT:
        fieldString = "_SYSTEMD_UNIT";
        break;
    case Field::_SYSTEMD_USER_SLICE:
        fieldString = "_SYSTEMD_USER_SLICE";
        break;
    case Field::_SYSTEMD_USER_UNIT:
        fieldString = "_SYSTEMD_USER_UNIT";
        break;
    }

    result = sd_journal_query_unique(journal.sdJournal(), fieldString.c_str());
    if (result < 0) {
        qCritical() << "Failed to query journal:" << strerror(-result);
        return dataList;
    }
    const int fieldLength = fieldString.length() + 1;
    SD_JOURNAL_FOREACH_UNIQUE(journal.sdJournal(), data, length)
    {
        QString dataStr = static_cast<const char *>(data);
        dataList << dataStr.remove(0, fieldLength);
    }
    return dataList;
}

QVector<JournaldHelper::BootInfo> JournaldHelper::queryOrderedBootIds(const Journal &journal)
{
    QVector<JournaldHelper::BootInfo> boots;

    QVector<QString> bootIds = JournaldHelper::queryUnique(journal, Field::_BOOT_ID);

    sd_journal *sdJournal = journal.sdJournal();
    for (const QString &id : bootIds) {
        int result = -1;
        uint64_t time;

        sd_journal_flush_matches(sdJournal);
        QString filterExpression = "_BOOT_ID=" + id;
        result = sd_journal_add_match(sdJournal, filterExpression.toStdString().c_str(), 0);
        if (result < 0) {
            qCritical() << "Failed add filter:" << strerror(-result);
            continue;
        }

        QDateTime since;
        result = sd_journal_seek_head(sdJournal);
        if (result < 0) {
            qCritical() << "Failed to seek head:" << strerror(-result);
            continue;
        }
        result = sd_journal_next(sdJournal);
        if (result < 0) {
            qCritical() << "Failed to obtain first entry:" << strerror(-result);
            continue;
        }
        result = sd_journal_get_realtime_usec(sdJournal, &time);
        if (result == 0) {
            since.setMSecsSinceEpoch(time / 1000);
        }

        QDateTime until;
        result = sd_journal_seek_tail(sdJournal);
        if (result < 0) {
            qCritical() << "Failed to seek tail:" << strerror(-result);
            continue;
        }
        result = sd_journal_next(sdJournal);
        if (result < 0) {
            qCritical() << "Failed to obtain first entry:" << strerror(-result);
        }
        result = sd_journal_get_realtime_usec(sdJournal, &time);
        if (result == 0) {
            until.setMSecsSinceEpoch(time / 1000);
        }

        boots << BootInfo{id, since, until};
    }

    std::sort(boots.begin(), boots.end(), [](const JournaldHelper::BootInfo &lhs, const JournaldHelper::BootInfo &rhs) {
        return lhs.mSince < rhs.mUntil;
    });

    return boots;
}

/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#include "journaldhelper.h"
#include <systemd/sd-journal.h>
#include <QDebug>

QVector<QString> JournaldHelper::queryUnique(const Journal &journal, Field field)
{
    QVector<QString> dataList;
    const void *data;
    size_t length;
    int result;

    std::string fieldString;
    switch(field) {
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
    SD_JOURNAL_FOREACH_UNIQUE(journal.sdJournal(), data, length) {
        QString dataStr = static_cast<const char*>(data);
        dataList << dataStr.remove(0, fieldLength);
    }
    return dataList;
}

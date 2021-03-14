/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#ifndef JOURNALDHELPER_H
#define JOURNALDHELPER_H

#include "journal.h"
#include <QDateTime>
#include <QVector>
#include "kjournald_export.h"

class KJOURNALD_EXPORT JournaldHelper
{
public:
    struct BootInfo {
        QString mBootId;
        QDateTime mSince;
        QDateTime mUntil;
    };

    enum class Field {
        // user fields
        MESSAGE,
        MESSAGE_ID,
        PRIORITY,
        CODE_FILE,
        CODE_LINE,
        CODE_FUNC,
        // trusted fields
        _BOOT_ID,
        _SYSTEMD_CGROUP,
        _SYSTEMD_SLICE,
        _SYSTEMD_UNIT,
        _SYSTEMD_USER_UNIT,
        _SYSTEMD_USER_SLICE,
        _SYSTEMD_SESSION,
        _SYSTEMD_OWNER_UID
    };

    static QVector<QString> queryUnique(const Journal &journal, Field field);

    /**
     * @brief Query boot information for @p journal
     *
     * @return ordered list of boots (first is earliest boot in time)
     */
    static QVector<BootInfo> queryOrderedBootIds(const Journal &journal);
};

#endif // JOURNALDHELPER_H

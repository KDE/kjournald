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
#include <QObject>

class KJOURNALD_EXPORT JournaldHelper
{
    Q_GADGET
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
        BOOT_ID,
        SYSTEMD_CGROUP,
        SYSTEMD_SLICE,
        SYSTEMD_UNIT,
        SYSTEMD_USER_UNIT,
        SYSTEMD_USER_SLICE,
        SYSTEMD_SESSION,
        SYSTEMD_OWNER_UID
    };
    Q_ENUM(Field)

    static QVector<QString> queryUnique(const Journal &journal, Field field);

    /**
     * @brief Query boot information for @p journal
     *
     * @return ordered list of boots (first is earliest boot in time)
     */
    static QVector<BootInfo> queryOrderedBootIds(const Journal &journal);

    static QString mapField(Field field);
};

#endif // JOURNALDHELPER_H

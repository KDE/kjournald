/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#ifndef JOURNALDHELPER_H
#define JOURNALDHELPER_H

#include "journal.h"
#include <QVector>

class JournaldHelper
{
public:
    enum class Field {
        // user fields
        MESSAGE,
        MESSAGE_ID,
        PRIORITY,
        CODE_FILE, CODE_LINE, CODE_FUNC,
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

    JournaldHelper();
    ~JournaldHelper();

    static QVector<QString> queryUnique(const Journal &journal, Field field);
};

#endif // JOURNALDHELPER_H

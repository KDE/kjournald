/*
    SPDX-License-Identifier: LGPL-2.1-or-later
    SPDX-FileCopyrightText: Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#ifndef JOURNALDHELPER_H
#define JOURNALDHELPER_H

#include <systemd/sd-journal.h>
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

    QVector<QString> queryUnique(Field field) const;

private:
    sd_journal *mJournal{ nullptr };
};

#endif // JOURNALDHELPER_H

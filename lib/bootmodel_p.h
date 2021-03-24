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
    using BootInfo = JournaldHelper::BootInfo;

    explicit BootModelPrivate(std::unique_ptr<IJournal> journal);

    QVector<BootInfo> mBootInfo;
    std::unique_ptr<IJournal> mJournal;
};

#endif // JOURNAL_H

/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2026 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#pragma once

#include <QObject>

class TestJournaldHelper : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void queryUniquePerBoot();
};

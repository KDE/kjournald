/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#ifndef TEST_UNIQUEQUERY_H
#define TEST_UNIQUEQUERY_H

#include <QObject>

class TestUniqueQuery : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void journalAccess();
    void boots();
    void systemdUnits();
    void systemdUnitsPerBoot();
};
#endif

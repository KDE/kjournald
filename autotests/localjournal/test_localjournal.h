/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#ifndef TEST_LOCALJOURNAL_H
#define TEST_LOCALJOURNAL_H

#include <QObject>

class TestLocalJournal : public QObject
{
    Q_OBJECT

public:
    TestLocalJournal();

private Q_SLOTS:
    void journalAccess();

private:
    const QStringList mBoots;
};
#endif

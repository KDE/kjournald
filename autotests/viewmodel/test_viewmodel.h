/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#ifndef TEST_VIEWMODEL_H
#define TEST_VIEWMODEL_H

#include <QObject>

class TestViewModel : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void journalAccess();
    void rowAccess();
};
#endif

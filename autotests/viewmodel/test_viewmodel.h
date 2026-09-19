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

public:
    TestViewModel();

private Q_SLOTS:
    void journalAccess();
    void rowAccess();
    void bootFilter();
    void systemUnitFilter();
    void userUnitFilter();
    void showKernelMessages();
    void closestIndexForDateComputation();
    /**
     * Check that exactly the full size of the journal is read and not more
     */
    void readFullJournal();
    /**
     * Reset model by changing the boot ID and and test that cursor for head/tail are updated accordingly
     */
    void resetModelHeadAndTailCursorTest();
    /**
     * Search mechanism with automatic fetching
     */
    void stringSearch();

private:
    const QStringList mBoots;
};
#endif

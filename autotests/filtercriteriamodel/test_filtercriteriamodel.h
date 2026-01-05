/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#ifndef TEST_FILTERCRITERIAMODEL_H
#define TEST_FILTERCRITERIAMODEL_H

#include <QObject>

class TestFilterCriteriaModel : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    /**
     * @brief Test basic assumptions about this model when loading a journal
     */
    void basicTreeModelStructure();

    // check for filter options availability
    void standaloneTestSystemdUnitSelectionOptionsUngrouped();
    void standaloneTestSystemdUnitSelectionOptionsGrouped();
    void standaloneTestExeSelectionOptions();
    void standaloneTestPrioritySelectionOptions();

private:
    const QStringList mBoots{"68f2e61d061247d8a8ba0b8d53a97a52", "27acae2fe35a40ac93f9c7732c0b8e59", "2dbe99dd855049af8f2865c5da2b8fda"};
};

#endif

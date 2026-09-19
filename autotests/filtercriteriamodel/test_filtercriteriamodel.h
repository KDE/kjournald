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

public:
    TestFilterCriteriaModel();

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
    const QStringList mBoots;
};

#endif

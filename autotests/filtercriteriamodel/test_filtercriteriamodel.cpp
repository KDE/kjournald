/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#include "test_filtercriteriamodel.h"
#include "../containertesthelper.h"
#include "../testdatalocation.h"
#include "filtercriteriamodel.h"
#include <QAbstractItemModelTester>
#include <QDebug>
#include <QDir>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QTest>
#include <QVector>

// note: this test request several data from a real example journald database
//       you can check them by using "journalctl -D journal" and requesting the values
//       that are checked here

void TestFilterCriteriaModel::basicTreeModelStructure()
{
    FilterCriteriaModel model;
    QAbstractItemModelTester tester(&model, QAbstractItemModelTester::FailureReportingMode::Fatal);

    // test failure handling for invalid journal: categories shall also be available there
    QTemporaryFile invalidJournal; // file is surely invalid
    QCOMPARE(model.setJournaldPath(invalidJournal.fileName()), false);
    QVERIFY(model.rowCount() > 0);

    // use extracted journal
    QCOMPARE(model.setJournaldPath(JOURNAL_LOCATION), true);
    QVERIFY(model.rowCount() > 0);

    // check for all expected categories
    MODEL_CONTAINS(model, FilterCriteriaModel::Roles::CATEGORY, FilterCriteriaModel::Category::SYSTEMD_UNIT);
    MODEL_CONTAINS(model, FilterCriteriaModel::Roles::CATEGORY, FilterCriteriaModel::Category::EXE);
    MODEL_CONTAINS(model, FilterCriteriaModel::Roles::CATEGORY, FilterCriteriaModel::Category::PRIORITY);
}

void TestFilterCriteriaModel::standaloneTestSystemdUnitSelectionOptions()
{
    FilterCriteriaModel model;
    QAbstractItemModelTester tester(&model, QAbstractItemModelTester::FailureReportingMode::Fatal);

    // use extracted journal
    QCOMPARE(model.setJournaldPath(JOURNAL_LOCATION), true);
    QVERIFY(model.rowCount() > 0);
    QVERIFY(model.entries(FilterCriteriaModel::Category::SYSTEMD_UNIT).count() > 0);

    {
        const auto container = model.entries(FilterCriteriaModel::Category::SYSTEMD_UNIT);
        QVERIFY(std::any_of(container.cbegin(), container.cend(), [=](std::pair<QString, bool> value) {
            return value.first == "user@1000.service"; // arbitrary service from test journal
        }));
    }

    { // QAbstractItemModel interface acccess
        QModelIndex categoryIndex;
        for (int i = 0; i < model.rowCount(); ++i) {
            if (model.data(model.index(i, 0), FilterCriteriaModel::Roles::CATEGORY) == FilterCriteriaModel::Category::SYSTEMD_UNIT) {
                categoryIndex = model.index(i, 0);
                break;
            }
        }
        QVERIFY(categoryIndex.isValid());
        QVERIFY(model.hasChildren(categoryIndex));
        // value from first position
        QCOMPARE(model.data(model.index(0, 0, categoryIndex), FilterCriteriaModel::Roles::TEXT).toString(), "busybox-klogd.service");
    }
}

void TestFilterCriteriaModel::standaloneTestExeSelectionOptions()
{
    FilterCriteriaModel model;
    QAbstractItemModelTester tester(&model, QAbstractItemModelTester::FailureReportingMode::Fatal);

    // use extracted journal
    QCOMPARE(model.setJournaldPath(JOURNAL_LOCATION), true);
    QVERIFY(model.rowCount() > 0);
    QVERIFY(model.entries(FilterCriteriaModel::Category::EXE).count() > 0);

    {
        const auto container = model.entries(FilterCriteriaModel::Category::EXE);
        QVERIFY(std::any_of(container.cbegin(), container.cend(), [=](std::pair<QString, bool> value) {
            return value.first == "/lib/systemd/systemd"; // arbitrary service from test journal
        }));
    }

    { // QAbstractItemModel interface acccess
        QModelIndex categoryIndex;
        for (int i = 0; i < model.rowCount(); ++i) {
            if (model.data(model.index(i, 0), FilterCriteriaModel::Roles::CATEGORY) == FilterCriteriaModel::Category::EXE) {
                categoryIndex = model.index(i, 0);
                break;
            }
        }
        QVERIFY(categoryIndex.isValid());
        QVERIFY(model.hasChildren(categoryIndex));
        // value from first position
        QCOMPARE(model.data(model.index(0, 0, categoryIndex), FilterCriteriaModel::Roles::TEXT).toString(), "/bin/bash.bash");
        // all elements are disabled initially
        QCOMPARE(model.data(model.index(0, 0, categoryIndex), FilterCriteriaModel::Roles::SELECTED).toBool(), false);
    }
}

void TestFilterCriteriaModel::standaloneTestPrioritySelectionOptions()
{
    FilterCriteriaModel model;
    QAbstractItemModelTester tester(&model, QAbstractItemModelTester::FailureReportingMode::Fatal);

    // use extracted journal
    QCOMPARE(model.setJournaldPath(JOURNAL_LOCATION), true);
    QVERIFY(model.rowCount() > 0);
    QVERIFY(model.entries(FilterCriteriaModel::Category::PRIORITY).count() > 0);

    { // direct access
        const auto container = model.entries(FilterCriteriaModel::Category::PRIORITY);
        QVERIFY(std::any_of(container.cbegin(), container.cend(), [=](std::pair<QString, bool> value) {
            return value.first == QString::number(2); // arbitrary priority from test journal
        }));
    }

    { // QAbstractItemModel interface acccess
        QModelIndex categoryIndex;
        for (int i = 0; i < model.rowCount(); ++i) {
            if (model.data(model.index(i, 0), FilterCriteriaModel::Roles::CATEGORY) == FilterCriteriaModel::Category::PRIORITY) {
                categoryIndex = model.index(i, 0);
                break;
            }
        }
        QVERIFY(categoryIndex.isValid());
        QVERIFY(model.hasChildren(categoryIndex));
        // at first position expect priority '0' / emergency level
        QCOMPARE(model.data(model.index(0, 0, categoryIndex), FilterCriteriaModel::Roles::DATA).toString(), "0");
    }

    { // QAbstractItemData::setData operations
        QModelIndex categoryIndex;
        for (int i = 0; i < model.rowCount(); ++i) {
            if (model.data(model.index(i, 0), FilterCriteriaModel::Roles::CATEGORY) == FilterCriteriaModel::Category::PRIORITY) {
                categoryIndex = model.index(i, 0);
                break;
            }
        }
        QVERIFY(categoryIndex.isValid());
        QVERIFY(model.hasChildren(categoryIndex));
        // already set, default beavhior is to return false
        //        QVERIFY(model.setData(model.index(1, 0, categoryIndex), false, FilterCriteriaModel::Roles::SELECTED) == false);

        // select priority entry for level 1, check for result
        model.setData(model.index(1, 0, categoryIndex), true, FilterCriteriaModel::Roles::SELECTED);
        // setData shall always return false if no data is changed
        QVERIFY(model.setData(model.index(1, 0, categoryIndex), true, FilterCriteriaModel::Roles::SELECTED) == false);
        QCOMPARE(model.priorityFilter(), 1);
        QVERIFY(model.data(model.index(0, 0, categoryIndex), FilterCriteriaModel::Roles::SELECTED) == false);
        QVERIFY(model.data(model.index(1, 0, categoryIndex), FilterCriteriaModel::Roles::SELECTED) == true);

        // select priority entry for level 0, then again for 1 to check resetting behavior of level 1
        QVERIFY(model.setData(model.index(0, 0, categoryIndex), true, FilterCriteriaModel::Roles::SELECTED) == true);
        QCOMPARE(model.priorityFilter(), 0);
        QVERIFY(model.data(model.index(0, 0, categoryIndex), FilterCriteriaModel::Roles::SELECTED) == true);
        QVERIFY(model.data(model.index(1, 0, categoryIndex), FilterCriteriaModel::Roles::SELECTED) == false);

        QVERIFY(model.setData(model.index(1, 0, categoryIndex), true, FilterCriteriaModel::Roles::SELECTED) == true);
        QCOMPARE(model.priorityFilter(), 1);
        QVERIFY(model.data(model.index(0, 0, categoryIndex), FilterCriteriaModel::Roles::SELECTED) == false);
        QVERIFY(model.data(model.index(1, 0, categoryIndex), FilterCriteriaModel::Roles::SELECTED) == true);
    }
}

QTEST_GUILESS_MAIN(TestFilterCriteriaModel);

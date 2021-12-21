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
        QCOMPARE(model.data(model.index(0, 0, categoryIndex), FilterCriteriaModel::Roles::TEXT).toString(), "ModemManager.service");
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
            return value.first == "/usr/bin/login"; // arbitrary service from test journal
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
        QCOMPARE(model.data(model.index(0, 0, categoryIndex), FilterCriteriaModel::Roles::TEXT).toString(), "/opt/Element/element-desktop");
        // all elements are selected initially
        QCOMPARE(model.data(model.index(0, 0, categoryIndex), FilterCriteriaModel::Roles::SELECTED).toBool(), true);
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
            return value.first == QString::number(5); // arbitrary service from test journal
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
        QCOMPARE(model.data(model.index(0, 0, categoryIndex), FilterCriteriaModel::Roles::TEXT).toString(), "0");
    }
}

QTEST_GUILESS_MAIN(TestFilterCriteriaModel);

/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#include "containertesthelpertest.h"
#include "../containertesthelper.h"
#include <QSet>
#include <QTest>
#include <algorithm>

void ContainerTestHelperTest::testContainerTestChecks()
{
    QVector<int> a = {1, 1, 2, 3, 5, 8, 13, 21};
    QSet<int> b = {1, 4, 9, 16, 25};
    QSet<int> c = {4, 9};

    // ContainerEq(container)
    // compare container a with b
    QVERIFY(!std::equal(a.cbegin(), a.cend(), b.cbegin()));
    //    CONTAINER_EQUAL(a, b);

    // Contains(e)
    // check if container contains 2
    QVERIFY(std::any_of(a.cbegin(), a.cend(), [=](int value) {
        return value == 2;
    }));
    //    CONTAINER_CONTAINS(a, 7);

    // Contains(e).Times(n)
    // check if container contains "1" two times
    QCOMPARE(std::accumulate(a.cbegin(),
                             a.cend(),
                             0,
                             [=](int acc, int value) {
                                 return acc += (value == 1 ? 1 : 0);
                             }),
             2);

    // IsSubsetOf(a_container)
    // check that a is not a subset of b
    QVERIFY(!std::all_of(a.cbegin(), a.cend(), [=](int value) {
        return b.contains(value);
    }));
    // check that c is subset of b
    CONTAINER_IS_SUBSET_OF(c, b);
}

QTEST_GUILESS_MAIN(ContainerTestHelperTest)

#include "moc_containertesthelpertest.cpp"

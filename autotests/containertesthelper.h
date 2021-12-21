/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#ifndef CONTAINER_TEST_H
#define CONTAINER_TEST_H

#include <algorithm>
#include <QTest>
#include <qtestcase.h>

// TODO currently only the first mismatching element is returned: document or change to print all

#define CONTAINER_EQUAL(actual, expected) \
do {\
    if (!QTest::compare_helper(std::size(actual) >= std::size(expected), "Container elements differ", \
                          QTest::toString(std::size(expected)), QTest::toString(std::size(actual)), "second container size", "first container size", __FILE__, __LINE__)) \
        return; \
    auto mismatch = std::mismatch(actual.cbegin(), actual.cend(), expected.cbegin()); \
    if (!QTest::compare_helper(mismatch.first == actual.cend(), "Container elements differ", \
                          QTest::toString(*mismatch.second), QTest::toString(*mismatch.first), "element of second container", "element of first container", __FILE__, __LINE__)) \
        return; \
} while (false)

/**
 * @param container a container poviding cbegin and cend; TODO std::cbegin
 */
#define CONTAINER_CONTAINS(container, expected) \
do {\
    const bool found = std::any_of(container.cbegin(), container.cend(), [=](int value) {\
        return value == expected;\
    }); \
    if (!QTest::qVerify(found, "Container does not contain element", #expected, __FILE__, __LINE__))\
        return;\
} while (false)

#define CONTAINER_IS_SUBSET_OF(subset, superset) \
do {\
    const bool found = std::all_of(subset.cbegin(), subset.cend(), [=](auto value) { \
        return superset.contains(value); \
    }); \
    if (!QTest::qVerify(found, "Container does not contain subset", #subset, __FILE__, __LINE__))\
        return;\
} while (false)

/**
 * Iterate over all elements of a list model @p model for certain @p role and check if it contains @p expected
 * @param container a container poviding cbegin and cend; TODO std::cbegin
 */
// TODO constexpr test of derived from model
#define MODEL_CONTAINS(model, role, expected) \
do {\
    bool found{ false }; \
    for (int i = 0; i < model.rowCount(); ++i) { \
        if (model.data(model.index(i, 0), role) == expected) { \
            found = true; \
            return; \
        } \
    } \
    if (!QTest::qVerify(found, "Model does not contain element", #expected, __FILE__, __LINE__))\
        return;\
} while (false)

#endif

# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>

ecm_add_test(
    containertesthelpertest.cpp
    LINK_LIBRARIES Qt::Core Qt::Quick Qt::Test
    TEST_NAME test_containertesthelper
)

# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>

ecm_add_test(
    containertesthelpertest.cpp
    LINK_LIBRARIES Qt5::Core Qt5::Quick Qt5::Test
    TEST_NAME test_containertesthelper
)

# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: Andreas Cord-Landwehr <cordlandwehr@kde.org>

ecm_add_test(
    test_journaldhelper.cpp
    test_journaldhelper.h
LINK_LIBRARIES Qt::Test kjournald
TEST_NAME test_journaldhelper
)

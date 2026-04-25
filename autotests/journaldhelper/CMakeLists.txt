# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: Andreas Cord-Landwehr <cordlandwehr@kde.org>

ecm_add_test(
    test_journaldhelper.cpp
    test_journaldhelper.h
LINK_LIBRARIES Qt::Test kjournald
TEST_NAME test_journaldhelper
)

qt_add_resources(test_journaldhelper "testdata"
    PREFIX "/"
    FILES cleanup_string_rawinput.txt cleanup_string_result.txt
)
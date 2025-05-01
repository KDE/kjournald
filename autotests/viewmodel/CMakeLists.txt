# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: Andreas Cord-Landwehr <cordlandwehr@kde.org>

ecm_add_test(
    test_viewmodel.cpp
    LINK_LIBRARIES Qt::Core Qt::Quick Qt::Test kjournald
    TEST_NAME test_viewmodel
)

target_include_directories(test_viewmodel PRIVATE
    ${CMAKE_SOURCE_DIR}/org/kde/kjournald
)

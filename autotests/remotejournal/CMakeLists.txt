# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: Andreas Cord-Landwehr <cordlandwehr@kde.org>

ecm_add_test(
    test_remotejournal.cpp
    LINK_LIBRARIES Qt5::Core Qt5::Quick Qt5::Test kjournald
    TEST_NAME test_remotejournal
)

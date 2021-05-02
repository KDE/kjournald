# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: Andreas Cord-Landwehr <cordlandwehr@kde.org>

configure_file(testdatalocation.h.inc testdatalocation.h)
configure_file(journalexportformat_example.export journalexportformat_example.export COPYONLY)
configure_file(journalexportformat_binary_example.export journalexportformat_binary_example.export COPYONLY)

add_custom_target(extract_testdata
    ALL
    COMMAND ${CMAKE_COMMAND} -E tar "xzf" ${CMAKE_CURRENT_SOURCE_DIR}/journal.tar.gz
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

add_subdirectory(localjournal)
add_subdirectory(uniquequery)
add_subdirectory(viewmodel)
add_subdirectory(remotejournal)

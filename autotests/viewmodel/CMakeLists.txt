# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: Andreas Cord-Landwehr <cordlandwehr@kde.org>

include_directories(
    ${CMAKE_SOURCE_DIR}/lib
    ${CMAKE_BINARY_DIR}/lib
)

set(test_SRCS
    test_viewmodel.cpp
)
add_executable(test_viewmodel ${test_SRCS})
target_link_libraries(test_viewmodel
PRIVATE
    Qt5::Core
    Qt5::Quick
    Qt5::Test
    kjournald
)
add_test(viewmodel test_viewmodel)

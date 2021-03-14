# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: Andreas Cord-Landwehr <cordlandwehr@kde.org>

include_directories(
    ${CMAKE_SOURCE_DIR}/lib
    ${CMAKE_BINARY_DIR}/lib
)

add_executable(journald-browser
    main.cpp
    clipboardproxy.cpp
    qml.qrc
)

target_compile_definitions(journald-browser
    PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)

target_link_libraries(journald-browser
PUBLIC
    Qt5::Core
    Qt5::Quick
    kjournald
)
install(TARGETS journald-browser DESTINATION bin)

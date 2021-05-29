# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: Andreas Cord-Landwehr <cordlandwehr@kde.org>

include_directories(
    ${CMAKE_SOURCE_DIR}/lib
    ${CMAKE_BINARY_DIR}/lib
)

add_executable(journald-browser
    main.cpp
    clipboardproxy.cpp
    sessionconfig.cpp
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
install(PROGRAMS org.kde.journald-browser.desktop  DESTINATION ${KDE_INSTALL_APPDIR})
install(FILES org.kde.journald-browser.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})

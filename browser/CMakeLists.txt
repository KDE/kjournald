# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: Andreas Cord-Landwehr <cordlandwehr@kde.org>

include_directories(
    ${CMAKE_SOURCE_DIR}/lib
    ${CMAKE_BINARY_DIR}/lib
    ${CMAKE_BINARY_DIR}
)

add_executable(kjournaldbrowser
    main.cpp
    sessionconfig.cpp
    sessionconfig.h
)

target_compile_definitions(kjournaldbrowser
    PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)

target_link_libraries(kjournaldbrowser
LINK_PUBLIC
    Qt::Core
    Qt::Quick
    Qt::QuickControls2
    Qt::Widgets # because of QApplication and QQC2 style integration
    KF6::I18n
    KF6::CoreAddons
    KF6::Crash
    kjournald
LINK_PRIVATE
    kjournaldbrowser_backing
    kjournaldbrowser_backingplugin
)
install(TARGETS kjournaldbrowser DESTINATION bin)
install(PROGRAMS org.kde.kjournaldbrowser.desktop  DESTINATION ${KDE_INSTALL_APPDIR})
install(FILES org.kde.kjournaldbrowser.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})

# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: Andreas Cord-Landwehr <cordlandwehr@kde.org>

add_library(kjournald SHARED)

ecm_qt_declare_logging_category(
    kjournald
    HEADER kjournaldlib_log_general.h
    IDENTIFIER "KJOURNALDLIB_GENERAL"
    CATEGORY_NAME kjournald.lib.general
    DESCRIPTION "KJournald General Debug Logs"
    EXPORT kjournald
)
ecm_qt_declare_logging_category(
    kjournald
    HEADER kjournaldlib_log_filtertrace.h
    IDENTIFIER "KJOURNALDLIB_FILTERTRACE"
    CATEGORY_NAME kjournald.lib.general
    DESCRIPTION "KJournald Trace Logs for Filter Operations"
    EXPORT kjournald
)
ecm_qt_install_logging_categories(
    EXPORT kjournald
    FILE kjournald.categories
    DESTINATION "${KDE_INSTALL_LOGGINGCATEGORIESDIR}"
)

generate_export_header(kjournald BASE_NAME KJournald)
target_sources(kjournald PRIVATE
    ${KJOURNALDLIB_SRCS}
    bootmodel.cpp
    bootmodel.h
    bootmodel_p.h
    colorizer.cpp
    colorizer.h
    fieldfilterproxymodel.cpp
    fieldfilterproxymodel.h
    ijournal.h
    localjournal.cpp
    localjournal.h
    localjournal_p.h
    filtercriteriamodel.cpp
    filtercriteriamodel.h
    filtercriteriamodel_p.h
    journaldexportreader.cpp
    journaldexportreader.h
    journaldhelper.cpp
    journaldhelper.h
    journaldviewmodel.cpp
    journaldviewmodel.h
    journaldviewmodel_p.h
    journalduniquequerymodel.cpp
    journalduniquequerymodel.h
    journalduniquequerymodel_p.h
    memory.h
    systemdjournalremote.cpp
    systemdjournalremote.h
    systemdjournalremote_p.h
)
target_link_libraries(kjournald
PRIVATE
    KF6::I18n
    Qt6::Core
    Qt6::Quick
    PkgConfig::SYSTEMD
)
set_property(TARGET kjournald APPEND PROPERTY COMPILE_DEFINITIONS QT_NO_CAST_FROM_ASCII QT_NO_CAST_TO_ASCII)
target_include_directories(kjournald PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/>
    $<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR}>
)
set_target_properties(kjournald PROPERTIES
    VERSION ${KJOURNALD_VERSION}
    SOVERSION ${KJOURNALD_SOVERSION}
    EXPORT_NAME KJournald
)
install(TARGETS kjournald
    EXPORT KJournaldTargets
    DESTINATION ${KDE_INSTALL_LIBDIR})

if(INSTALL_EXPERIMENTAL_HEADERS)
    install(FILES
        bootmodel.h
        ijournal.h
        localjournal.h
        journaldhelper.h
        journaldviewmodel.h
        journalduniquequerymodel.h
        systemdjournalremote.h
        ${CMAKE_CURRENT_BINARY_DIR}/kjournald_export.h
        DESTINATION ${KDE_INSTALL_INCLUDEDIR}
    )
endif()

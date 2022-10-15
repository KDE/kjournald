# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: Andreas Cord-Landwehr <cordlandwehr@kde.org>

add_library(kjournald SHARED)
generate_export_header(kjournald BASE_NAME KJournald)
target_sources(kjournald PRIVATE
    bootmodel.cpp
    bootmodel.h
    bootmodel_p.h
    colorizer.cpp
    colorizer.h
    loggingcategories.cpp
    loggingcategories.h
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
    KF5::I18n
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Quick
    PkgConfig::SYSTEMD
)
set_property(TARGET kjournald APPEND PROPERTY COMPILE_DEFINITIONS QT_NO_CAST_FROM_ASCII QT_NO_CAST_TO_ASCII)
target_include_directories(kjournald PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/>
    $<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR}>
)
set_target_properties(kjournald PROPERTIES
    VERSION ${KJOURNALD_VERSION_STRING}
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

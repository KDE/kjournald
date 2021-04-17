# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: Andreas Cord-Landwehr <cordlandwehr@kde.org>

set(kjournald_SRCS
    bootmodel.cpp
    loggingcategories.cpp
    fieldfilterproxymodel.cpp
    ijournal.h
    localjournal.cpp
    journaldexportreader.cpp
    journaldhelper.cpp
    journaldviewmodel.cpp
    journalduniquequerymodel.cpp
)

set(kjournald_HDRS
    bootmodel.h
    ijournal.h
    localjournal.h
    journaldhelper.h
    journaldviewmodel.h
    journalduniquequerymodel.h
)

add_library(kjournald SHARED
    ${kjournald_SRCS}
)
generate_export_header(kjournald BASE_NAME KJournald)

target_link_libraries(kjournald
PRIVATE
    Qt5::Core
    Qt5::Quick
    ${SYSTEMD_LIBRARIES}
)
target_include_directories(kjournald PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/>
      $<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR}>
)

install(TARGETS kjournald DESTINATION ${KDE_INSTALL_LIBDIR})

install(FILES
    ${kjournald_HDRS}
    ${CMAKE_CURRENT_BINARY_DIR}/kjournald_export.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}
)

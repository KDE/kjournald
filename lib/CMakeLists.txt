# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: Andreas Cord-Landwehr <cordlandwehr@kde.org>

set(kjournald_SRCS
    bootmodel.cpp
    loggingcategories.cpp
    fieldfilterproxymodel.cpp
    journal.cpp
    journaldhelper.cpp
    journaldviewmodel.cpp
    journalduniquequerymodel.cpp
)

set(kjournald_HDRS
    bootmodel.h
    fieldfilterproxymodel.h
    journal.h
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
install(TARGETS kjournald DESTINATION lib)
install(FILES ${kjournald_HDRS}
    ${CMAKE_CURRENT_BINARY_DIR}/kjournald_export.h
    DESTINATION lib/kjournald
)

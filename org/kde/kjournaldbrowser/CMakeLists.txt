# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: Andreas Cord-Landwehr <cordlandwehr@kde.org>

add_library(kjournaldbrowser_backing STATIC)

ecm_qt_declare_logging_category(
    kjournaldbrowser_backing
    HEADER kjournaldbrowser_log.h
    IDENTIFIER "KJOURNALD_APP"
    CATEGORY_NAME kjournald.app
    DESCRIPTION "KJournald Application Debug Logs"
    EXPORT kjournald
)
# ecm_qt_install_logging_categories(
#     EXPORT kjournaldbrowser_backing
#     FILE kjournald.categories
#     DESTINATION "${KDE_INSTALL_LOGGINGCATEGORIESDIR}"
# )

qt6_add_qml_module(
    kjournaldbrowser_backing
    URI org.kde.kjournaldbrowser
    IMPORTS
        org.kde.kirigami
        org.kde.kjournald
    DEPENDENCIES
        QtQuick
        org.kde.ki18n
        org.kde.kirigamiaddons.
        org.kde.kirigamiaddons.statefulapp
        org.kde.kjournald
    SOURCES
        browserapplication.cpp
        browserapplication.h
        clipboardproxy.cpp
        clipboardproxy.h
        databaseprovider.cpp
        databaseprovider.h
        formatter.cpp
        formatter.h
        textsearch.cpp
        textsearch.h
    QML_FILES
        ColoredCheckbox.qml
        FilterCriteriaView.qml
        GlobalMenu.qml
        JournalFileSelectionDialog.qml
        JournalFolderSelectionDialog.qml
        LogLine.qml
        LogView.qml
        Main.qml
        RemoteJournalConfigDialog.qml
        TopMenuBar.qml
)

target_link_libraries(kjournaldbrowser_backing
    PUBLIC kjournald Qt6::Core Qt6::Gui KF6::Kirigami KirigamiAddonsStatefulApp
)

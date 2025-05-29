# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: Andreas Cord-Landwehr <cordlandwehr@kde.org>

add_library(kjournaldbrowser_backing STATIC)

qt6_add_qml_module(
    kjournaldbrowser_backing
    URI org.kde.kjournaldbrowser
    IMPORTS
        org.kde.kirigami
        org.kde.kjournald
    DEPENDENCIES
        QtQuick
        org.kde.kjournald
        org.kde.kirigamiaddons.
        org.kde.kirigamiaddons.statefulapp
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
        Main.qml
        LogLine.qml
        GlobalMenu.qml
        RemoteJournalConfigDialog.qml
        TopMenuBar.qml
        LogView.qml
        FilterCriteriaView.qml
        ColoredCheckbox.qml
)

target_link_libraries(kjournaldbrowser_backing
    PUBLIC kjournald Qt6::Core Qt6::Gui KF6::Kirigami KirigamiAddonsStatefulApp
)

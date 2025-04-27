add_library(kjournaldbrowser_backing STATIC)

qt6_add_qml_module(
    kjournaldbrowser_backing
    URI "org.kde.kjournaldbrowser"
    IMPORTS
        org.kde.kirigami
    SOURCES
        clipboardproxy.cpp
        clipboardproxy.h
        flattenedfiltercriteriaproxymodel.cpp
        flattenedfiltercriteriaproxymodel.h
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
    PUBLIC kjournald Qt6::Core Qt6::Gui
)

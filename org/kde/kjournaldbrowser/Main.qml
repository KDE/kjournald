/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Dialogs as Dialogs
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.statefulapp as StatefulApp
import org.kde.kjournald
import org.kde.kjournaldbrowser
import org.kde.ki18n

StatefulApp.StatefulWindow {
    id: root

    width: 1000
    height: 640
    visible: true
    windowName: 'kjournaldbrowser'
    application: BrowserApplication

    required property FilterCriteriaModel filterModel
    required property url initialJournalPath

    Component.onCompleted: {
        if (root.initialJournalPath !== "") {
            console.log(`set initial journald path to ` + root.initialJournalPath)
            DatabaseProvider.setJournalPath(root.initialJournalPath)
        }
    }

    Binding {
        root.filterModel.journalProvider: DatabaseProvider.journalProvider
    }

    Binding {
        root.filterModel.enableSystemdUnitTemplateGrouping: BrowserApplication.serviceGrouping === BrowserApplication.ServiceGrouping.GROUP_SERVICE_TEMPLATES
    }

    menuBar: TopMenuBar {
        visible: (Kirigami.Settings.hasPlatformMenuBar === false || Kirigami.Settings.hasPlatformMenuBar === undefined) && !Kirigami.Settings.isMobile
        onCopyViewToClipboard: logView.copyTextFromView()
        fileDialog: journalFileSelectionDialog
        folderDialog: journalFolderSelectionDialog
    }

    Loader {
        id: gobalMenuLoader
        active: Kirigami.Settings.hasPlatformMenuBar === true && !Kirigami.Settings.isMobile
        sourceComponent: GlobalMenu {
            fileDialog: journalFileSelectionDialog
            folderDialog: journalFolderSelectionDialog
        }
    }
    Connections {
        target: gobalMenuLoader.item
        function onCopyViewToClipboard() { logView.copyTextFromView() }
    }

    header: ToolBar {
        RowLayout {
            anchors {
                fill: parent
                leftMargin: 10
            }
            // forward page key events to listview
            Keys.forwardTo: [logView]
            focus: true

            Label {
                text: KI18n.i18nc("@label", "Boot:")
            }
            ComboBox {
                id: bootIdComboBox
                implicitWidth: Math.max(300, implicitContentWidth)
                model: bootModel
                valueRole: "bootid"
                textRole: BrowserApplication.timeDisplay
                          === BrowserApplication.UTC ? "displayshort_utc" : "displayshort_localtime"
                delegate: ItemDelegate {
                    id: bootIdDelegate
                    required property int index
                    required property string displayshort_utc
                    required property string displayshort_localtime
                    width: parent.width
                    text: BrowserApplication.timeDisplay
                          === BrowserApplication.UTC ? bootIdDelegate.displayshort_utc : bootIdDelegate.displayshort_localtime
                    font.weight: bootIdDelegate.index === bootIdComboBox.currentIndex ? Font.Bold : Font.Normal
                }
            }
            ToolSeparator {}

            Kirigami.SearchField {
                id: highlightTextField
                Layout.fillWidth: true
                text: ""
                property ListModel recentSearchesModel: ListModel {
                    // TODO replace with persistent model over application starts
                }
                onTextChanged: TextSearch.needle = text
                onTextEdited: {
                    recentSearchesListView.currentIndex = -1
                    if (!popup.opened && text !== "") {
                        popup.open()
                    }
                }
                Keys.onPressed: (event)=> {
                    console.log("handle in search field")
                    if (event.key === Qt.Key_Down) {
                        if (!popup.opened) {
                            recentSearchesListView.currentIndex = 0
                            popup.open()
                        } else {
                            recentSearchesListView.incrementCurrentIndex()
                        }
                    }
                    if (event.key === Qt.Key_Up) {
                        recentSearchesListView.decrementCurrentIndex()
                    }
                    if (event.key === Qt.Key_Return) {
                        if (recentSearchesListView.currentIndex === -1) {
                            recentSearchesModel.insert(0, {text: text})
                            if (recentSearchesModel.rowCount() > 20) {
                                recentSearchesModel.remove(20)
                            }
                        } else {
                            recentSearchesModel.move(recentSearchesListView.currentIndex, 0, 1)
                            highlightTextField.text = recentSearchesModel.get(recentSearchesListView.currentIndex).text
                        }
                        popup.close()
                    }
                    if (event.key === Qt.Key_Escape) {
                        popup.close()
                    }
                }

                Popup {
                    id: popup
                    y: highlightTextField.height
                    width: highlightTextField.width
                    height: Math.min(recentSearchesListView.contentHeight + 24, 400)
                    modal: false
                    focus: false
                    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent
                    ListView {
                        id: recentSearchesListView
                        anchors.fill: parent
                        model: highlightTextField.recentSearchesModel
                        highlightFollowsCurrentItem: true
                        delegate: ItemDelegate {
                            id: recentSearchDelegate
                            required text
                            required property int index
                            width: recentSearchesListView.width
                            highlighted: ListView.isCurrentItem
                            onClicked: {
                                highlightTextField.recentSearchesModel.move(index, 0, 1)
                                highlightTextField.text = text
                                popup.close()
                            }

                            contentItem: Text {
                                rightPadding: recentSearchDelegate.spacing
                                text: recentSearchDelegate.text
                                font: recentSearchDelegate.font
                                elide: Text.ElideRight
                                visible: recentSearchDelegate.text
                                horizontalAlignment: Text.AlignLeft
                                verticalAlignment: Text.AlignVCenter
                            }
                        }
                        ScrollIndicator.vertical: ScrollIndicator {
                            id: recentSeearchScrollIndicator
                        }
                    }
                }
            }

            ToolButton {
                id: caseSensitiveOptionButton
                icon.name: checked ? "format-text-capitalize" : "format-text-lowercase"
                checkable: true
                ToolTip.text: KI18n.i18nc("@info:tooltip", "Switch between case sensitive and case insensitive matching")
                ToolTip.visible: hovered
                onCheckedChanged: TextSearch.caseSensitive = caseSensitiveOptionButton.checked
            }
            ToolButton {
                enabled: TextSearch.needle.length > 2
                icon.name: "go-up-search"
                onClicked: {
                    logView.scrollToSearchResult(TextSearch.needle, JournaldViewModel.BACKWARD, TextSearch.caseSensitive)
                }
            }
            ToolButton {
                enabled: TextSearch.needle.length > 2
                icon.name: "go-down-search"
                onClicked: {
                    logView.scrollToSearchResult(TextSearch.needle, JournaldViewModel.FORWARD, TextSearch.caseSensitive)
                }
            }

            ToolSeparator {}

            ToolButton {
                icon.name: "go-top"
                onClicked: logView.scrollToBeginning()
            }
            ToolButton {
                icon.name: "go-bottom"
                onClicked: logView.scrollToEnd()
            }

            ToolSeparator {}
            ToolButton {
                icon.name: BrowserApplication.viewMode === BrowserApplication.BROWSE ? "transform-browse" : "edit-select-text"
                checked: BrowserApplication.viewMode === BrowserApplication.BROWSE
                onClicked: {
                    if (BrowserApplication.viewMode === BrowserApplication.BROWSE) {
                        BrowserApplication.viewMode = BrowserApplication.SELECT
                    } else {
                        BrowserApplication.viewMode = BrowserApplication.BROWSE
                    }
                }
                ToolTip.text: BrowserApplication.viewMode === BrowserApplication.BROWSE ?
                                 KI18n.i18nc("@info:tooltip", "Switch to browsing mode") :
                                 KI18n.i18nc("@info:tooltip", "Switch to selection mode")
                ToolTip.visible: hovered
            }

            Item {
                Layout.fillWidth: true
            }
        }
    }

    Dialogs.FolderDialog {
        id: journalFolderSelectionDialog
        title: KI18n.i18nc("@title", "Select journal folder")
        currentFolder: DatabaseProvider.journalPath
        onAccepted: {
            DatabaseProvider.setJournalPath(journalFolderSelectionDialog.selectedFolder)
        }
    }

    Dialogs.FileDialog {
        id: journalFileSelectionDialog
        title: KI18n.i18nc("@title", "Select journal file")
        nameFilters: [KI18n.i18nc("@item", "Journal files (*.journal)"), KI18n.i18nc("@item", "All files (*)")]
        onAccepted: {
            DatabaseProvider.setJournalPath(journalFileSelectionDialog.selectedFile)
        }
    }

    RemoteJournalConfigDialog {
        id: remoteJournalDialog
        onAccepted: {
            DatabaseProvider.setRemoteJournalUrl(remoteJournalDialog.url, remoteJournalDialog.port)
        }
    }

    FlattenedFilterCriteriaProxyModel {
        id: flatFilterSelection
        sourceModel: root.filterModel
    }

    SplitView {
        id: splitView
        anchors.fill: parent

        // forward page key events to listview
        Keys.forwardTo: [logView]
        focus: true

        ScrollView {
            id: scrollView
            SplitView.fillHeight: true
            SplitView.preferredWidth: 250
            Kirigami.Theme.colorSet: Kirigami.Theme.View
            Kirigami.Theme.inherit: false
            ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

            FilterCriteriaView {
                model: flatFilterSelection
            }

            data: Kirigami.Separator {
                height: splitView.height
                anchors.right: scrollView.right
            }
        }

        Item {
            height: parent.height
            SplitView.fillWidth: true
            SplitView.fillHeight: true
            LogView {
                id: logView
                anchors.fill: parent
                journalModel: journalModel
                displayRoleRight: BrowserApplication.filterCriterium === BrowserApplication.SYSTEMD_UNIT ?
                                      JournaldViewModel.SYSTEMD_UNIT : JournaldViewModel.EXE
                snapToFollowMode: true
                textSelectionMode: BrowserApplication.viewMode === BrowserApplication.SELECT
                visible: count > 0
                onTextCopied: text => {
                    ClipboardProxy.setText(text)
                    console.log("view content copied")
                }
            }

            Kirigami.PlaceholderMessage {
                anchors.centerIn: parent
                width: parent.width - (Kirigami.Units.largeSpacing * 4)

                visible: !journalModel.available

                icon.name: "data-error"
                text: KI18n.i18nc("@title", "Unable to load journal database from selected location: ") + DatabaseProvider.localJournalPath
                explanation: KI18n.i18nc("@info", "Check sub-directories of selected directory to contain files ending with '.journal'.")
            }

            Kirigami.PlaceholderMessage {
                anchors.centerIn: parent
                width: parent.width - (Kirigami.Units.largeSpacing * 4)
                visible: journalModel.available && logView.count === 0
                text: KI18n.i18nc("@info:tooltip", "No log entries apply to selected filters.")
            }
        }
    }

    BootModel {
        id: bootModel
        journalProvider: DatabaseProvider.journalProvider
    }

    JournaldViewModel {
        id: journalModel
        journalProvider: DatabaseProvider.journalProvider
        filter.units: root.filterModel.systemdUnitFilter
        filter.exes: root.filterModel.exeFilter
        filter.boots: bootIdComboBox.currentValue
        filter.priority: root.filterModel.priorityFilter
        filter.kernel: root.filterModel.kernelFilter
    }
}

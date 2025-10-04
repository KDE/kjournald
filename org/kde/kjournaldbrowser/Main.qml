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

StatefulApp.StatefulWindow {
    id: root

    width: 1000
    height: 640
    visible: true
    windowName: 'kjournaldbrowser'
    application: BrowserApplication

    required property FilterCriteriaModel filterModel
    required property string initialJournalPath

    Component.onCompleted: {
        if (root.initialJournalPath !== "") {
            console.log(`set initial journald path to ` + root.initialJournalPath)
            DatabaseProvider.setLocalJournalPath(root.initialJournalPath)
        }
    }

    Binding {
        root.filterModel.journalPath: DatabaseProvider.mode === DatabaseProvider.SYSTEM ? undefined : DatabaseProvider.journalPath
    }

    menuBar: TopMenuBar {
        visible: (Kirigami.Settings.hasPlatformMenuBar === false || Kirigami.Settings.hasPlatformMenuBar === undefined) && !Kirigami.Settings.isMobile
        onCopyViewToClipboard: logView.copyTextFromView()
    }

    Loader {
        active: Kirigami.Settings.hasPlatformMenuBar === true && !Kirigami.Settings.isMobile
        source: Qt.resolvedUrl("qrc:/GlobalMenu.qml")
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
                text: i18n("Boot:")
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
                ToolTip.text: i18nc("@info:tooltip", "Switch between case sensitive and case insensitive matching")
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
                                 i18nc("@info:tooltip", "Switch to browsing mode") :
                                 i18nc("@info:tooltip", "Switch to selection mode")
                ToolTip.visible: hovered
            }

            Item {
                Layout.fillWidth: true
            }
        }
    }

    Dialogs.FolderDialog {
        id: folderDialog
        title: i18n("Select journal folder")
        onAccepted: {
            DatabaseProvider.setLocalJournalPath(folderDialog.currentFolder)
        }
    }

    Dialogs.FileDialog {
        id: fileDialog
        title: i18n("Select journal file")
        nameFilters: [i18n("Journal files (*.journal)"), i18n("All files (*)")]
        onAccepted: {
            DatabaseProvider.setLocalJournalPath(fileDialog.currentFile)
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
            // Once the minimal KF5 version gets increased
            // it would be great to use Kirigami.PlaceholderMessage instead
            ColumnLayout {
                visible: logView.count === 0
                anchors.centerIn: parent
                Kirigami.Heading {
                    Layout.fillWidth: true

                    text: i18n("No log entries apply ")
                    level: 2
                    opacity: 0.5

                    horizontalAlignment: Qt.AlignHCenter
                    wrapMode: Text.WordWrap
                }
            }
        }
    }

    BootModel {
        id: bootModel
        journalPath: DatabaseProvider.mode === DatabaseProvider.SYSTEM ? undefined : DatabaseProvider.journalPath
    }

    JournaldViewModel {
        id: journalModel
        journalPath: DatabaseProvider.mode === DatabaseProvider.SYSTEM ? undefined : DatabaseProvider.journalPath
        filter.units: root.filterModel.systemdUnitFilter
        filter.exes: root.filterModel.exeFilter
        filter.boots: bootIdComboBox.currentValue
        filter.priority: root.filterModel.priorityFilter
        filter.kernel: root.filterModel.kernelFilter
    }
}

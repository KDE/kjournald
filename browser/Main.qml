/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.15
import org.kde.kirigami 2.6 as Kirigami
import kjournald 1.0

Kirigami.AbstractApplicationWindow {
    width: 1000
    height: 640
    visible: true

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
                model: g_bootModel
                valueRole: "bootid"
                textRole: g_config.timeDisplay
                          === SessionConfig.UTC ? "displayshort_utc" : "displayshort_localtime"
                delegate: ItemDelegate {
                    text: g_config.timeDisplay
                          === SessionConfig.UTC ? model.displayshort_utc : model.displayshort_localtime
                    font.weight: model.current === true ? Font.Bold : Font.Normal
                }
            }
            ToolSeparator {}

            Label {
                text: i18n("Highlight:")
            }
            TextField {
                id: hightlightTextField
                text: ""
            }
            ToolButton {
                enabled: hightlightTextField.length > 2
                icon.name: "go-up-search"
                onClicked: {
                    logView.scrollToSearchResult(hightlightTextField.text, JournaldViewModel.BACKWARD)
                }
            }
            ToolButton {
                enabled: hightlightTextField.length > 2
                icon.name: "go-down-search"
                onClicked: {
                    logView.scrollToSearchResult(hightlightTextField.text, JournaldViewModel.FORWARD)
                }
            }

            ToolSeparator {}
            ToolButton {
                text: i18n("Browse")
                icon.name: "transform-browse"
                checkable: true
                checked: g_config.viewMode === SessionConfig.BROWSE
                onClicked: {
                    g_config.viewMode = SessionConfig.BROWSE
                }
            }
            ToolButton {
                text: i18n("Select")
                icon.name: "edit-select-text"
                checkable: true
                checked: g_config.viewMode === SessionConfig.SELECT
                onClicked: {
                    g_config.viewMode = SessionConfig.SELECT
                }
            }

            Item {
                Layout.fillWidth: true
            }
        }
    }

    FileDialog {
        id: folderDialog
        title: i18n("Select journal folder")
        selectFolder: true
        onAccepted: {
            g_config.localJournalPath = folderDialog.fileUrl
            g_config.sessionMode = SessionConfig.LOCALFOLDER
        }
    }

    FileDialog {
        id: fileDialog
        title: i18n("Select journal file")
        nameFilters: [i18n("Journal files (*.journal)"), i18n("All files (*)")]
        onAccepted: {
            g_config.localJournalPath = fileDialog.fileUrl
            g_config.sessionMode = SessionConfig.LOCALFOLDER
        }
    }

    Dialog {
        id: aboutDialog
        visible: false
        modality: Qt.NonModal
        standardButtons: StandardButton.Close
        width: 600
        height: 450

        ColumnLayout {
            anchors.fill: parent
            Kirigami.AboutPage {
                AboutProxy {
                    id: aboutProxy
                }
                Layout.fillWidth: true
                Layout.fillHeight: true
                aboutData: aboutProxy.aboutData
            }
        }
    }

    RemoteJournalConfigDialog {
        id: remoteJournalDialog
        onAccepted: {
            console.log("set remote journal to: " + url + ":" + port)
            g_config.remoteJournalPort = remoteJournalDialog.port
            g_config.remoteJournalUrl = remoteJournalDialog.url
            g_config.sessionMode = SessionConfig.REMOTE
        }
    }

    FlattenedFilterCriteriaProxyModel {
        id: flatFilterSelection
        sourceModel: g_filterModel
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
                journalModel: g_journalModel
                displayRoleRight: g_config.filterCriterium === SessionConfig.SYSTEMD_UNIT ?
                                      JournaldViewModel.SYSTEMD_UNIT : JournaldViewModel.EXE
                snapToFollowMode: true
                textSelectionMode: g_config.viewMode === SessionConfig.SELECT
                visible: count > 0
                onTextCopied: {
                    Clipboard.setText(text)
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

    JournaldViewModel {
        id: g_journalModel
        journalPath: g_config.sessionMode === SessionConfig.LOCALFOLDER
                     || g_config.sessionMode
                     === SessionConfig.REMOTE ? g_config.localJournalPath : undefined
        systemdUnitFilter: g_filterModel.systemdUnitFilter
        exeFilter: g_filterModel.exeFilter
        bootFilter: bootIdComboBox.currentValue
        priorityFilter: g_filterModel.priorityFilter
        kernelFilter: g_filterModel.kernelFilter
    }
}

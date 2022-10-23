/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import QtQuick.Dialogs 1.3 as Dialogs
import QtQuick.Layouts 1.15
import org.kde.kirigami 2.19 as Kirigami
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

    Pane {
        Loader {
            id: aboutDialog
            anchors.centerIn: parent
            parent: Overlay.overlay
            width: 500
            height: 500

            function open() {
                if (item) {
                    item.open()
                } else {
                    active = true
                }
            }
            onLoaded: item.open()
            active: false
            asynchronous: true
            sourceComponent: Dialog {
                width: aboutDialog.width
                height: aboutDialog.height
                visible: false
                title: i18n("About")
                footer: DialogButtonBox {
                    Button {
                        icon.name: "tools-report-bug"
                        text: i18n("Launch Bug Report Wizard")
                        DialogButtonBox.buttonRole: DialogButtonBox.RejectRole
                        onClicked:{
                            const elements = aboutProxy.aboutData.productName.split('/');
                            let url = `https://bugs.kde.org/enter_bug.cgi?format=guided&product=${elements[0]}&version=${aboutProxy.aboutData.version}`;
                            if (elements.length === 2) {
                                url += "&component=" + elements[1]
                            }
                            console.log("url: " + url)
                            Qt.openUrlExternally(url)
                        }
                    }
                    Button {
                        icon.name: "document-close"
                        text: i18n("Close")
                        DialogButtonBox.buttonRole: DialogButtonBox.RejectRole
                    }
                }

                AboutProxy {
                    id: aboutProxy
                }
                Kirigami.AboutItem {
                    anchors.fill: parent
                    aboutData: aboutProxy.aboutData
                }
            }
        }
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
                textRole: SessionConfigProxy.timeDisplay
                          === SessionConfig.UTC ? "displayshort_utc" : "displayshort_localtime"
                delegate: ItemDelegate {
                    text: SessionConfigProxy.timeDisplay
                          === SessionConfig.UTC ? model.displayshort_utc : model.displayshort_localtime
                    font.weight: model.index === bootIdComboBox.currentIndex ? Font.Bold : Font.Normal
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
                checked: SessionConfigProxy.viewMode === SessionConfig.BROWSE
                onClicked: {
                    SessionConfigProxy.viewMode = SessionConfig.BROWSE
                }
            }
            ToolButton {
                text: i18n("Select")
                icon.name: "edit-select-text"
                checkable: true
                checked: SessionConfigProxy.viewMode === SessionConfig.SELECT
                onClicked: {
                    SessionConfigProxy.viewMode = SessionConfig.SELECT
                }
            }

            Item {
                Layout.fillWidth: true
            }
        }
    }

    Dialogs.FileDialog {
        id: folderDialog
        title: i18n("Select journal folder")
        selectFolder: true
        onAccepted: {
            SessionConfigProxy.localJournalPath = folderDialog.fileUrl
            SessionConfigProxy.sessionMode = SessionConfig.LOCALFOLDER
        }
    }

    Dialogs.FileDialog {
        id: fileDialog
        title: i18n("Select journal file")
        nameFilters: [i18n("Journal files (*.journal)"), i18n("All files (*)")]
        onAccepted: {
            SessionConfigProxy.localJournalPath = fileDialog.fileUrl
            SessionConfigProxy.sessionMode = SessionConfig.LOCALFOLDER
        }
    }

    RemoteJournalConfigDialog {
        id: remoteJournalDialog
        onAccepted: {
            console.log("set remote journal to: " + url + ":" + port)
            SessionConfigProxy.remoteJournalPort = remoteJournalDialog.port
            SessionConfigProxy.remoteJournalUrl = remoteJournalDialog.url
            SessionConfigProxy.sessionMode = SessionConfig.REMOTE
        }
    }

    FlattenedFilterCriteriaProxyModel {
        id: flatFilterSelection
        sourceModel: FilterCriteriaModelProxy
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
                displayRoleRight: SessionConfigProxy.filterCriterium === SessionConfig.SYSTEMD_UNIT ?
                                      JournaldViewModel.SYSTEMD_UNIT : JournaldViewModel.EXE
                snapToFollowMode: true
                textSelectionMode: SessionConfigProxy.viewMode === SessionConfig.SELECT
                visible: count > 0
                onTextCopied: {
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
        journalPath: SessionConfigProxy.sessionMode === SessionConfig.LOCALFOLDER
                     || SessionConfigProxy.sessionMode
                     === SessionConfig.REMOTE ? SessionConfigProxy.localJournalPath : undefined
    }

    JournaldViewModel {
        id: g_journalModel
        journalPath: SessionConfigProxy.sessionMode === SessionConfig.LOCALFOLDER
                     || SessionConfigProxy.sessionMode
                     === SessionConfig.REMOTE ? SessionConfigProxy.localJournalPath : undefined
        systemdUnitFilter: FilterCriteriaModelProxy.systemdUnitFilter
        exeFilter: FilterCriteriaModelProxy.exeFilter
        bootFilter: bootIdComboBox.currentValue
        priorityFilter: FilterCriteriaModelProxy.priorityFilter
        kernelFilter: FilterCriteriaModelProxy.kernelFilter
    }
}

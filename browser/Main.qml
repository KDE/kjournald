/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Dialogs as Dialogs
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import kjournald
import org.kde.kjournald

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
                            const elements = AboutData.productName.split('/');
                            let url = `https://bugs.kde.org/enter_bug.cgi?format=guided&product=${elements[0]}&version=${AboutData.version}`;
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

                Kirigami.AboutItem {
                    anchors.fill: parent
                    aboutData: AboutData
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
                    width: parent.width
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
                icon.name: SessionConfigProxy.viewMode === SessionConfig.BROWSE ? "transform-browse" : "edit-select-text"
                checked: SessionConfigProxy.viewMode === SessionConfig.BROWSE
                onClicked: {
                    if (SessionConfigProxy.viewMode === SessionConfig.BROWSE) {
                        SessionConfigProxy.viewMode = SessionConfig.SELECT
                    } else {
                        SessionConfigProxy.viewMode = SessionConfig.BROWSE
                    }
                }
                ToolTip.text: SessionConfigProxy.viewMode === SessionConfig.BROWSE ?
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
        filter.units: FilterCriteriaModelProxy.systemdUnitFilter
        filter.exes: FilterCriteriaModelProxy.exeFilter
        filter.boots: bootIdComboBox.currentValue
        filter.priority: FilterCriteriaModelProxy.priorityFilter
        filter.kernel: FilterCriteriaModelProxy.kernelFilter
    }
}

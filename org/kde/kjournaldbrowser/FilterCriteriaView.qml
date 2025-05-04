/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.kjournald
import Qt.labs.qmlmodels

ListView {
    id: root
    ButtonGroup { id: priorityGroup }
    activeFocusOnTab: true
    reuseItems: true
    delegate: DelegateChooser {
        role: "type"
        DelegateChoice {
            roleValue: FlattenedFilterCriteriaProxyModel.FIRST_LEVEL
            delegate: ItemDelegate {
                id: expandDelegate
                required property bool expanded
                required property bool selected
                required text
                required property var model

                width: ListView.view.width
                onClicked: model.expanded = !expandDelegate.expanded
                Accessible.name: expandDelegate.text
                Accessible.description: expandDelegate.expanded ? i18n("Expanded") : i18n("Collapsed")
                contentItem: ColumnLayout {
                    RowLayout {
                        Label {
                            text: expandDelegate.text
                            textFormat: Text.PlainText
                            elide: Text.ElideRight
                            Layout.fillWidth: true
                        }
                        Kirigami.Icon {
                            id: collapseIcon
                            implicitWidth: Kirigami.Units.iconSizes.small
                            implicitHeight: Kirigami.Units.iconSizes.small
                            source: expandDelegate.expanded ? "collapse" : "expand"
                        }
                    }
                    ItemDelegate { // clear button for checkbox selection
                        visible: expandDelegate.expanded && expandDelegate.selected
                        leftPadding: 10
                        onClicked: expandDelegate.model.selected = false
                        activeFocusOnTab: true
                        Accessible.name: clearLabel.text
                        Layout.fillWidth: true
                        contentItem: RowLayout {
                            Label {
                                id: clearLabel
                                text: i18n("Clear")
                                Layout.fillWidth: true
                            }
                            Kirigami.Icon {
                                Layout.preferredWidth: Kirigami.Units.iconSizes.small
                                Layout.preferredHeight: Kirigami.Units.iconSizes.small
                                source: "edit-clear"
                            }
                        }
                    }
                }
            }
        }
        DelegateChoice {
            roleValue: FlattenedFilterCriteriaProxyModel.CHECKBOX
            delegate: ItemDelegate {
                id: checkboxDelegate
                required property bool selected
                required property color color
                required text
                required property string longtext
                required property var model

                width: ListView.view.width
                leftPadding: 20
                onClicked: model.selected = !checkboxDelegate.selected

                contentItem: Row {
                    spacing: 0.5 * Kirigami.Units.gridUnit
                    Label {
                        text: checkboxDelegate.text
                        textFormat: Text.PlainText
                        elide: Text.ElideRight
                        width: parent.width - checkbox.width - 0.5 * Kirigami.Units.gridUnit
                    }
                    ColoredCheckbox {
                        id: checkbox

                        // hard overwrite of internal calculations for speedup, check with profiler when modify
                        implicitWidth: Kirigami.Units.iconSizes.small
                        implicitHeight: Kirigami.Units.iconSizes.small
                        baselineOffset: 0
                        contentItem: null

                        color: checkboxDelegate.color
                        checked: checkboxDelegate.selected
                        onToggled: {
                            if (checkboxDelegate.selected !== checked) {
                                checkboxDelegate.model.selected = checked
                            }
                        }
                    }
                }
                ToolTip.delay: 1000
                ToolTip.timeout: 5000
                ToolTip.visible: hovered
                ToolTip.text: checkboxDelegate.longtext
            }
        }
        DelegateChoice {
            roleValue: FlattenedFilterCriteriaProxyModel.RADIOBUTTON
            delegate: ItemDelegate {
                id: radioDelegate
                required property bool selected
                required text
                required property string longtext
                required property var model

                width: ListView.view.width
                leftPadding: 20
                onClicked: radioDelegate.model.selected = true
                contentItem: RowLayout {
                    Label {
                        text: radioDelegate.text
                        textFormat: Text.PlainText
                        elide: Text.ElideRight
                        Layout.fillWidth: true
                    }

                    RadioButton {
                        id: radiobox
                        autoExclusive: true
                        checked: radioDelegate.selected
                        spacing: 0
                        onToggled: {
                            if (radioDelegate.selected !== checked) {
                                radioDelegate.model.selected = checked
                            }
                        }
                        ButtonGroup.group: priorityGroup
                    }
                }
                ToolTip.delay: 1000
                ToolTip.timeout: 5000
                ToolTip.visible: hovered
                ToolTip.text: radioDelegate.longtext
            }
        }
    }
}

/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import QtQml 2.15
import org.kde.kirigami 2.2 as Kirigami
import kjournald 1.0
import Qt.labs.qmlmodels 1.0

ListView {
    id: root
    ButtonGroup { id: priorityGroup }
    model: flatFilterSelection
    activeFocusOnTab: true
    reuseItems: true
    delegate: DelegateChooser {
        role: "type"
        DelegateChoice {
            roleValue: FlattenedFilterCriteriaProxyModel.FIRST_LEVEL
            delegate: Kirigami.AbstractListItem {
                onClicked: model.expanded = !model.expanded
                Accessible.name: model.text
                Accessible.description: model.expanded ? i18n("Expanded") : i18n("Collapsed")
                contentItem: ColumnLayout {
                    RowLayout {
                        Label {
                            text: model.text
                            textFormat: Text.PlainText
                            elide: Text.ElideRight
                            Layout.fillWidth: true
                        }
                        Kirigami.Icon {
                            id: collapseIcon
                            implicitWidth: Kirigami.Units.iconSizes.small
                            implicitHeight: Kirigami.Units.iconSizes.small
                            source: model.expanded ? "collapse" : "expand"
                        }
                    }
                    Kirigami.AbstractListItem { // clear button for checkbox selection
                        visible: model.expanded && model.selected
                        leftPadding: 20
                        onClicked: model.selected = false
                        activeFocusOnTab: true
                        Accessible.name: clearLabel.text
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
            delegate: Kirigami.AbstractListItem {
                id: checkboxDelegate
                width: ListView.view.width
                text: model ? model.text : ""
                leftPadding: 20
                onClicked: model.selected = !model.selected

                contentItem: Row {
                    spacing: 0.5 * Kirigami.Units.gridUnit
                    Label {
                        text: model ? model.text : ""
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

                        color: model ? model.color : Kirigami.Theme.textColor
                        checked: model ? model.selected : false
                        onToggled: model.selected = checked
                    }
                }
                ToolTip.delay: 1000
                ToolTip.timeout: 5000
                ToolTip.visible: hovered
                ToolTip.text: model !== null ? model.longtext : ""
            }
        }
        DelegateChoice {
            roleValue: FlattenedFilterCriteriaProxyModel.RADIOBUTTON
            delegate: Kirigami.AbstractListItem {
                id: radioDelegate
                width: ListView.view.width
                text: model ? model.text : ""
                leftPadding: 20
                onClicked: {
                    model.selected = true
                }
                contentItem: RowLayout {
                    Label {
                        text: model ? model.text : ""
                        textFormat: Text.PlainText
                        elide: Text.ElideRight
                        Layout.fillWidth: true
                    }

                    RadioButton {
                        id: radiobox
                        autoExclusive: true
                        checked: model ? model.selected : false
                        spacing: 0
                        onToggled: {
                            if (model.selected !== checked) {
                                model.selected = checked
                            }
                        }
                        ButtonGroup.group: priorityGroup
                    }
                }
                ToolTip.delay: 1000
                ToolTip.timeout: 5000
                ToolTip.visible: hovered
                ToolTip.text: model !== null ? model.longtext : ""
            }
        }
    }
}

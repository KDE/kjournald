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

Column {
    id: root
    ButtonGroup{ id: priorityGroup }
    Repeater {
        id: rootElements
        model: flatFilterSelection

        Component {
            id: firstLevelComponent
            Column {
                property var model
                Kirigami.AbstractListItem {
                    onClicked: model.expanded = !model.expanded
                    contentItem: RowLayout {
                        Label {
                            text: model.text
                            textFormat: Text.PlainText
                            elide: Text.ElideRight
                            Layout.fillWidth: true
                        }
                        Kirigami.Icon {
                            Layout.preferredWidth: 16
                            Layout.preferredHeight: 16
                            source: model.expanded ? "collapse" : "expand"
                        }
                    }
                }
                Kirigami.AbstractListItem { // clear button for checkbox selection
                    visible: model.expanded && model.selected
                    leftPadding: 20
                    onClicked: model.selected = false
                    contentItem: RowLayout {
                        Label {
                            text: i18n("Clear")
                            Layout.fillWidth: true
                        }
                        Kirigami.Icon {
                            Layout.preferredWidth: 16
                            Layout.preferredHeight: 16
                            source: "edit-clear"
                        }
                    }
                }
            }
        }
        Component {
            id: secondLevelCheckboxComponent
            Kirigami.AbstractListItem {
                id: control
                property var model
                readonly property bool selected: model.selected
                onSelectedChanged: checkbox.checked = control.selected
                text: model ? model.text : ""
                leftPadding: 20
                onClicked: model.selected = !model.selected

                contentItem: RowLayout {
                    Label {
                        text: model ? model.text : ""
                        textFormat: Text.PlainText
                        elide: Text.ElideRight
                        Layout.fillWidth: true
                    }
                    ColoredCheckbox {
                        id: checkbox
                        color: model ? model.color : Kirigami.Theme.textColor
                        checked: model ? model.selected : false
                        onCheckedChanged: if (model.selected !== checked) model.selected = checked
                    }
                }
                ToolTip.delay: 1000
                ToolTip.timeout: 5000
                ToolTip.visible: hovered
                ToolTip.text: model !== null ? model.longtext : ""
            }
        }
        Component {
            id: secondLevelRadiobuttonComponent
            Kirigami.AbstractListItem {
                id: control
                property var model
                readonly property bool selected: model.selected
                onSelectedChanged: radiobox.checked = control.selected
                text: model ? model.text : ""
                leftPadding: 20
                onClicked: model.selected = !model.selected

                contentItem: RowLayout {
                    Label {
                        text: model ? model.text : ""
                        textFormat: Text.PlainText
                        elide: Text.ElideRight
                        Layout.fillWidth: true
                    }

                    RadioButton {
                        id: radiobox
                        checked: model ? model.selected : false
                        spacing: 0
                        onCheckedChanged: if (model.selected !== checked) model.selected = checked
                        ButtonGroup.group: priorityGroup
                    }

                }
                ToolTip.delay: 1000
                ToolTip.timeout: 5000
                ToolTip.visible: hovered
                ToolTip.text: model !== null ? model.longtext : ""
            }
        }
        Loader {
            width: root.width
            sourceComponent: {
                if (model.indentation === 0) {
                    return firstLevelComponent;
                }
                if (model.type === FlattenedFilterCriteriaProxyModel.CHECKBOX) {
                    return secondLevelCheckboxComponent
                }
                if (model.type === FlattenedFilterCriteriaProxyModel.RADIOBUTTON) {
                    return secondLevelRadiobuttonComponent
                }
                console.warn("fallback due to unknown type: " + model.type)
                return firstLevelComponent;
            }
            onLoaded: item.model = model // pass model reference to item
        }
    }
}

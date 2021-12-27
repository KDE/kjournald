/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQml 2.15
import kjournald 1.0

Column {
    ButtonGroup{ id: priorityGroup }
    Repeater {
        id: rootElements
        model: flatFilterSelection

        Component {
            id: firstLevelComponent
            Column {
                property var model
                ItemDelegate {
                    text:  model.text
                    onClicked: model.expanded = !model.expanded
                }
                ItemDelegate { // clear button for checkbox selection
                    visible: model.expanded && model.selected
                    leftPadding: 20
                    icon.name: "arrow-left"
                    text: i18n("Clear")
                    onClicked: model.selected = false
                }
            }
        }
        Component {
            id: secondLevelCheckboxComponent
            CheckBox {
                property var model
                height: 20 + children.height
                text: model ? model.text : ""
                leftPadding: 20
                checked: model ? model.selected : false
                onCheckedChanged: if (model.selected !== checked) model.selected = checked

                ToolTip.delay: 1000
                ToolTip.timeout: 5000
                ToolTip.visible: hovered
                ToolTip.text: model !== null ? model.longtext : ""
            }
        }
        Component {
            id: secondLevelCheckboxColorCodeComponent
            CheckBox {
                id: control
                property var model
                height: 20 + children.height
                text: model ? model.text : ""
                leftPadding: 20
                checked: model ? model.selected : false
                onCheckedChanged: if (model.selected !== checked) model.selected = checked

                ToolTip.delay: 1000
                ToolTip.timeout: 5000
                ToolTip.visible: hovered
                ToolTip.text: model !== null ? model.longtext : ""

                contentItem: Row {
                    leftPadding: control.indicator && !control.mirrored ? control.indicator.width + control.spacing : 0
                    rightPadding: control.indicator && control.mirrored ? control.indicator.width + control.spacing : 0
                    spacing: 6

                    Rectangle {
                        width: 24
                        height: 24
                        radius: 4
                        color: model ? model.color : "#FF0000"
                    }

                    Text {
                        text: control.text
                        font: control.font
                        color: control.palette.windowText
                    }
                }
            }
        }
        Component {
            id: secondLevelRadiobuttonComponent
            RadioButton {
                property var model
                height: 20 + children.height
                text: model ? model.text : ""
                leftPadding: 20
                onCheckedChanged: model.selected = checked
                onModelChanged: if (model) { checked = model.selected }
                ButtonGroup.group: priorityGroup

                ToolTip.delay: 1000
                ToolTip.timeout: 5000
                ToolTip.visible: hovered
                ToolTip.text: model !== null ? model.longtext : ""
            }
        }
        Loader {
            sourceComponent: {
                if (model.indentation === 0) {
                    return firstLevelComponent;
                }
                if (model.type === FlattenedFilterCriteriaProxyModel.CHECKBOX) {
                    return secondLevelCheckboxComponent
                }
                if (model.type === FlattenedFilterCriteriaProxyModel.CHECKBOX_COLORED) {
                    return secondLevelCheckboxColorCodeComponent
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

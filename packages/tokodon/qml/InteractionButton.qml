// SPDX-FileCopyrightText: 2022 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2

/**
 * @brief A specialized button used for the "Favorite", "Boost", etc buttons on a status
 */
QQC2.ToolButton {
    id: control

    required property string iconName
    required property string tooltip

    property string interactedIconName
    property bool interactable: true
    property bool interacted: false
    property color interactionColor

    display: QQC2.AbstractButton.IconOnly
    activeFocusOnTab: interactable

    QQC2.ToolTip.text: control.tooltip
    QQC2.ToolTip.visible: hovered && QQC2.ToolTip.text !== ""
    QQC2.ToolTip.delay: 600

    Accessible.name: tooltip
    Accessible.description: text

    contentItem: RowLayout {
        spacing: 6

        QQC2.Label {
            id: icon

            Layout.preferredWidth: 22
            Layout.preferredHeight: 22
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter

            text: control.interacted ? control.interactedIconName : control.iconName

            color: (control.interactable && parent.activeFocus) ? "#7767dc" : (control.interacted ? control.interactionColor : "#64748b")
        }

        QQC2.Label {
            id: label

            text: control.text
            verticalAlignment: Text.AlignVCenter
            visible: control.text
            color: "#64748b"

            Layout.rightMargin: 6
        }
    }
}

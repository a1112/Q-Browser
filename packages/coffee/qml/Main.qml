// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
import QtQuick
import QtQuick.Controls
//! [Set application window size]
Rectangle {
    id: root
    property var runtime: typeof Runtime !== "undefined" ? Runtime : null
    width: 1000
    height: 600
    color: Colors.currentTheme.background
    Component.onCompleted: if (runtime) runtime.setPageMetadata("咖啡工坊", "ready")
    Button { text: "← 示例中心"; x: 12; y: 8; onClicked: if (root.runtime) root.runtime.navigate("/__demo_gallery") }

//! [Set application window size]
    ApplicationFlow {
        anchors.fill: parent
        anchors.topMargin: 54
    }

    Binding {
        target: Config
        property: "mode"
        value: root.height > root.width ? "portrait" : "landscape"
    }
}

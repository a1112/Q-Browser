// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
import QtQuick
import QtQuick.Controls

ReadyForm {
    id: readyForm

    required property ApplicationFlow appFlow
    required property CoffeeConfig coffeeConfig

    property alias timer: timer
    foamAmount: coffeeConfig.foamAmount
    milkAmount: coffeeConfig.milkAmount
    coffeeAmount: coffeeConfig.coffeeAmount
    sugarAmount: coffeeConfig.sugarAmount
    state: Config.mode

    Timer {
        id: timer
        interval: 3000
        running: false
        onTriggered: {
            readyForm.appFlow.onReturnToStart()
        }
    }
    Button {
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottomMargin: 16
        text: "再做一杯"
        onClicked: readyForm.appFlow.onReturnToStart()
    }
    grid.states: [
        State {
            name: "small"
            when: ((Screen.height * Screen.devicePixelRatio)
                   + (Screen.width * Screen.devicePixelRatio)) < 2000
            PropertyChanges {
                target: grid
                rowSpacing: 10
            }
        }
    ]
    caption.states: [
        State {
            name: "small"
            when: ((Screen.height * Screen.devicePixelRatio)
                   + (Screen.width * Screen.devicePixelRatio)) < 2000
            PropertyChanges {
                target: caption
                font.pixelSize: 16
            }
        }
    ]
}

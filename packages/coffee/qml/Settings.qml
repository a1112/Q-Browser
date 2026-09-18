// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
import QtQuick

SettingsForm {
    id: settingsForm

    required property ApplicationFlow appFlow
    required property CoffeeConfig coffeeConfig

    foamAmount: coffeeConfig.foamAmount
    milkAmount: coffeeConfig.milkAmount
    coffeeAmount: coffeeConfig.coffeeAmount
    state: Config.mode

    rectangle.states: [
        State {
            name: "smallerFont"
            when: ((Screen.height * Screen.devicePixelRatio)
                   + (Screen.width * Screen.devicePixelRatio)) < 2000
            PropertyChanges {
                target: rectangle
                textPixelSize: 14
            }
        }
    ]
    sugarSlider.states: State {
        name: "pressed"
        when: settingsForm.sugarSlider.pressed
        PropertyChanges {
            target: handle
            scale: 1.1
        }
    }
    handle.states: [
        State {
            name: "small"
            when: ((Screen.height * Screen.devicePixelRatio)
                   + (Screen.width
                      * Screen.devicePixelRatio)) < 2000
            PropertyChanges {
                target: handle
                width: 10
            }
        }
    ]
    box.states: [
        State {
            name: "small"
            when: ((Screen.height * Screen.devicePixelRatio)
                   + (Screen.width
                      * Screen.devicePixelRatio)) < 2000
            PropertyChanges {
                target: box
                implicitWidth: sugarText.width + 4
                implicitHeight: sugarText.height + 2
            }
        }
    ]
    sugarText.states: [
        State {
            name: "small"
            when: ((Screen.height * Screen.devicePixelRatio) + (Screen.width * Screen.devicePixelRatio)) < 2000
            PropertyChanges {
                target: sugarText
                font.pixelSize: 8
            }
        }
    ]

    confirmButton.onClicked: appFlow.confirmButton()

    //! [synchronizer]
    Binding { target: settingsForm.sugarSlider; property: "value"; value: settingsForm.coffeeConfig.sugarAmount }
    Connections { target: settingsForm.sugarSlider; function onMoved() { settingsForm.coffeeConfig.sugarAmount = settingsForm.sugarSlider.value } }
    //! [synchronizer]
    Binding { target: settingsForm.foamSlider; property: "value"; value: settingsForm.coffeeConfig.foamAmount }
    Connections { target: settingsForm.foamSlider; function onMoved() { settingsForm.coffeeConfig.foamAmount = settingsForm.foamSlider.value } }
    Binding { target: settingsForm.milkSlider; property: "value"; value: settingsForm.coffeeConfig.milkAmount }
    Connections { target: settingsForm.milkSlider; function onMoved() { settingsForm.coffeeConfig.milkAmount = settingsForm.milkSlider.value } }
    Binding { target: settingsForm.coffeeSlider; property: "value"; value: settingsForm.coffeeConfig.coffeeAmount }
    Connections { target: settingsForm.coffeeSlider; function onMoved() { settingsForm.coffeeConfig.coffeeAmount = settingsForm.coffeeSlider.value } }
}

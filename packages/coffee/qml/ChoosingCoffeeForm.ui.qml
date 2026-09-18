// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
import QtQuick
import QtQuick.Layouts

Item {
    // Height, width and any other size related properties containing odd looking float or other dividers
    // that do not seem to have any logical origin are just arbitrary and based on original design
    // and/or personal preference on what looks nice.
    id: root
    property alias cappuccinoButton: cappuccino.button
    property alias latteButton: latte.button
    property alias espressoButton: espresso.button
    property alias macchiatoButton: macchiato.button
    property alias cards: cards
    property alias cappuccino: cappuccino
    property alias macchiato: macchiato
    property alias espresso: espresso
    property alias latte: latte

    states: [
        State {
            name: "portrait"
            PropertyChanges {
                target: cards
                flow: GridLayout.TopToBottom
                rows: 2
            }
        },
        State {
            name: "landscape"
            PropertyChanges {
                target: cards
                flow: GridLayout.LeftToRight
                columns: 4
            }
        }
    ]
    //! [Coffees]
    GridLayout {
        id: cards
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        rowSpacing: 20
        columnSpacing: 20
        CoffeeCard {
            id: cappuccino
            coffeeName: "卡布奇诺"
            ingredients: "牛奶 · 浓缩咖啡 · 奶泡"
            time: 5
        }
        CoffeeCard {
            id: latte
            coffeeName: "拿铁"
            ingredients: "浓缩咖啡 · 奶泡"
            time: 6
        }
        CoffeeCard {
            id: espresso
            coffeeName: "浓缩咖啡"
            ingredients: "牛奶 · 浓缩咖啡"
            time: 4
        }
        CoffeeCard {
            id: macchiato
            coffeeName: "玛奇朵"
            ingredients: "奶泡 · 浓缩咖啡"
            time: 8
        }
    }
    //! [Coffees]
}

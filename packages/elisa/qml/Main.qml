// SPDX-FileCopyrightText: 2026 Q-Browser contributors
// SPDX-License-Identifier: LGPL-3.0-or-later
// Local application shell around the adapted Elisa TrackBrowserDelegate.
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root
    width: 1100; height: 720; color: "#f5f7fa"
    property var runtime: typeof Runtime !== "undefined" ? Runtime : null
    property var tracks: []
    property var queue: []
    property int current: -1
    property var playback: ({state: "stopped", position: 0, duration: 0, volume: .35})
    property string error: ""
    property var requests: ({})
    property bool showQueue: false
    property var commandQueue: []
    property bool requestPending: false
    function call(operation, payload) {
        if (!runtime) { error = "需要已验证的包运行环境"; return }
        if (requestPending) {
            if (operation !== "status") {
                let pending = commandQueue.filter(c => c.operation !== operation || (operation !== "seek" && operation !== "setVolume"))
                pending.push({operation: operation, payload: payload || {}})
                commandQueue = pending
            }
            return
        }
        requestPending = true
        let id = runtime.invoke("audio", operation, payload || {})
        requests[id] = operation
    }
    function play(index) {
        if (index < 0 || index >= tracks.length) return
        current = index
        call("play", {trackId: tracks[index].id})
    }
    function step(delta) {
        let list = queue.length ? queue : tracks.map((t, i) => i)
        if (!list.length) return
        let at = list.indexOf(current)
        play(list[(at + delta + list.length) % list.length])
    }
    function clock(ms) { let s = Math.floor((ms || 0) / 1000); return Math.floor(s / 60) + ":" + (s % 60 < 10 ? "0" : "") + s % 60 }
    Component.onCompleted: {
        if (runtime) runtime.setPageMetadata("Elisa 音乐", "ready")
        call("catalog")
    }
    Connections {
        target: root.runtime
        function onCapabilityFinished(id, response) {
            let operation = root.requests[id]
            if (!operation) return
            delete root.requests[id]
            root.requestPending = false
            if (root.commandQueue.length) {
                let pending = root.commandQueue.slice()
                let command = pending.shift()
                root.commandQueue = pending
                Qt.callLater(function() { root.call(command.operation, command.payload) })
            }
            if (!response.ok) { root.error = response.error ? response.error.message : "音频请求失败"; return }
            if (operation !== "status") root.error = ""
            let data = response.result || response.data || {}
            if (operation === "catalog") root.tracks = data.tracks || []
            else { root.playback = data; if (data.error) root.error = data.error }
        }
    }
    Timer { interval: 500; repeat: true; running: root.runtime !== null && root.runtime.active; onTriggered: root.call("status") }
    ColumnLayout {
        anchors.fill: parent; spacing: 0
        Rectangle {
            Layout.fillWidth: true; Layout.preferredHeight: 62; color: "white"
            RowLayout {
                anchors.fill: parent; anchors.margins: 14
                Button { text: "← 示例中心"; onClicked: if (root.runtime) root.runtime.navigate("/__demo_gallery") }
                Label { text: "♫  Elisa"; font.pixelSize: 24; font.bold: true; color: "#207baf" }
                Item { Layout.fillWidth: true }
                Label { text: "KDE 26.08.1 · 精简移植"; color: "#6b7688" }
            }
        }
        RowLayout {
            Layout.fillWidth: true; Layout.fillHeight: true; spacing: 0
            Rectangle {
                Layout.preferredWidth: 170; Layout.fillHeight: true; color: "#eaf0f6"
                ColumnLayout {
                    anchors.fill: parent; anchors.margins: 16; spacing: 12
                    Label { text: "音乐资料库"; font.bold: true; color: "#607087" }
                    Button { text: "▦  专辑"; Layout.fillWidth: true; onClicked: root.showQueue = false }
                    Button { text: "☷  播放队列 (" + root.queue.length + ")"; Layout.fillWidth: true; onClicked: root.showQueue = true }
                    Item { Layout.fillHeight: true }
                    Label { text: "3 段原创 WAV\nHost 真实播放\n不自动开始播放"; color: "#607087"; lineHeight: 1.5 }
                }
            }
            ColumnLayout {
                Layout.fillWidth: true; Layout.fillHeight: true; Layout.margins: 24; spacing: 16
                Label { text: root.showQueue ? "播放队列" : "你的音乐，此刻相伴。"; font.pixelSize: 28; font.bold: true }
                TextField { id: search; objectName: "elisa-search"; Layout.fillWidth: true; placeholderText: "搜索歌曲、专辑或艺术家" }
                RowLayout {
                    visible: !root.showQueue; Layout.fillWidth: true; spacing: 14
                    Repeater {
                        model: root.tracks
                        delegate: Rectangle {
                            required property var modelData; required property int index
                            Layout.fillWidth: true; Layout.preferredHeight: 140; radius: 12
                            color: ["#b9deed", "#c9cdea", "#243550"][index % 3]
                            Column { anchors.centerIn: parent; spacing: 9
                                Label { anchors.horizontalCenter: parent.horizontalCenter; text: "♫"; font.pixelSize: 42; color: index === 2 ? "white" : "#2d4e68" }
                                Label { text: modelData.album; font.pixelSize: 18; color: index === 2 ? "white" : "#2d4e68" }
                            }
                            TapHandler { onTapped: search.text = modelData.album }
                        }
                    }
                }
                ListView {
                    id: songList; Layout.fillWidth: true; Layout.fillHeight: true; clip: true; spacing: 4
                    model: root.showQueue ? root.queue : root.tracks.map((t, i) => i)
                    delegate: TrackBrowserDelegate {
                        required property int modelData
                        width: ListView.view.width
                        title: root.tracks[modelData].title; artist: root.tracks[modelData].artist
                        album: root.tracks[modelData].album; duration: "0:24"; trackNumber: modelData + 1
                        isSelected: root.current === modelData
                        visible: (title + artist + album).indexOf(search.text) >= 0
                        height: visible ? 66 : 0
                        onReplaceAndPlay: root.play(modelData)
                        onEnqueue: { let q = root.queue.slice(); q.push(modelData); root.queue = q }
                    }
                }
                Button { visible: root.showQueue; text: "清空队列"; onClicked: root.queue = [] }
                Label { text: root.error; visible: text.length > 0; color: "#b42332"; wrapMode: Text.Wrap; Layout.fillWidth: true }
            }
        }
        Rectangle {
            Layout.fillWidth: true; Layout.preferredHeight: 112; color: "white"
            ColumnLayout {
                anchors.fill: parent; anchors.margins: 14; spacing: 4
                RowLayout {
                    Layout.fillWidth: true
                    Label { text: root.current >= 0 ? root.tracks[root.current].title : "选择一首歌开始播放"; Layout.preferredWidth: 190; font.bold: true }
                    Button { text: "上一首"; onClicked: root.step(-1) }
                    Button { objectName: "audio-play"; text: root.playback.state === "playing" ? "暂停" : "播放"; onClicked: root.playback.state === "playing" ? root.call("pause") : root.play(root.current < 0 ? 0 : root.current) }
                    Button { text: "下一首"; onClicked: root.step(1) }
                    Button { text: "停止"; onClicked: root.call("stop") }
                    Item { Layout.fillWidth: true }
                    Label { text: "音量" }
                    Slider { from: 0; to: 1; value: .35; Layout.preferredWidth: 120; onMoved: root.call("setVolume", {volume: value}) }
                }
                RowLayout {
                    Label { text: root.clock(root.playback.position) }
                    Slider { Layout.fillWidth: true; from: 0; to: Math.max(1, root.playback.duration || 0); value: root.playback.position || 0; onMoved: root.call("seek", {position: Math.round(value)}) }
                    Label { text: root.clock(root.playback.duration) }
                }
            }
        }
    }
}

// SPDX-FileCopyrightText: 2026 Q-Browser contributors
// SPDX-License-Identifier: GPL-3.0-only
// Offline shell using the adapted upstream PostDelegate/InteractionButton.
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root
    width: 1100; height: 720; color: "#f6f5fa"
    property var runtime: typeof Runtime !== "undefined" ? Runtime : null
    property int selected: -1
    property bool bookmarks: false
    function toggle(index, key) { posts.setProperty(index, key, !posts.get(index)[key]) }
    Component.onCompleted: if (runtime) runtime.setPageMetadata("Tokodon 社区", "ready")
    ListModel {
        id: posts
        objectName: "posts"
        ListElement { author: "林间来信"; handle: "@forest@local.demo"; body: "早安！今天的第一杯咖啡，配上刚刚写好的 QML 动画。\n\n你正在做什么有趣的小项目？ #桌面开发 #开源"; time: "5 分钟前"; liked: false; saved: false; likes: 24; replies: ""; hue: "#63a78d" }
        ListElement { author: "桌面设计笔记"; handle: "@design@local.demo"; body: "让信息保持清晰，让交互自然发生。\n试试点赞、收藏，或者打开详情留下回复。所有内容只保存在这个标签的当前会话。"; time: "18 分钟前"; liked: false; saved: false; likes: 42; replies: ""; hue: "#8b78ca" }
        ListElement { author: "周末观察员"; handle: "@weekend@local.demo"; body: "窗外正在下雨，适合听音乐、读书，以及整理那些尚未完成的想法。\n\n#日常 #慢生活"; time: "1 小时前"; liked: false; saved: false; likes: 16; replies: ""; hue: "#619abd" }
    }
    ColumnLayout {
        anchors.fill: parent; spacing: 0
        Rectangle {
            Layout.fillWidth: true; Layout.preferredHeight: 62; color: "white"
            RowLayout { anchors.fill: parent; anchors.margins: 14
                Button { text: "← 示例中心"; onClicked: if (root.runtime) root.runtime.navigate("/__demo_gallery") }
                Label { text: "◉  Tokodon"; font.pixelSize: 24; font.bold: true; color: "#7060bd" }
                Item { Layout.fillWidth: true }
                Label { text: "KDE 26.08.1 · 离线体验"; color: "#7a728d" }
            }
        }
        RowLayout {
            Layout.fillWidth: true; Layout.fillHeight: true; spacing: 0
            Rectangle {
                Layout.preferredWidth: 180; Layout.fillHeight: true; color: "#eeebf5"
                ColumnLayout { anchors.fill: parent; anchors.margins: 18; spacing: 12
                    Label { text: "我的社区"; font.bold: true; color: "#7a728d" }
                    Button { text: "⌂  主页"; Layout.fillWidth: true; onClicked: {root.bookmarks = false; root.selected = -1} }
                    Button { text: "☆  收藏"; Layout.fillWidth: true; onClicked: {root.bookmarks = true; root.selected = -1} }
                    Button { text: "＋  发布动态"; Layout.fillWidth: true; onClicked: composer.open() }
                    Item { Layout.fillHeight: true }
                    Label { text: "本地演示账号\n不连接 Mastodon\n关闭后清空互动"; color: "#7a728d"; lineHeight: 1.5 }
                }
            }
            ColumnLayout {
                Layout.fillWidth: true; Layout.fillHeight: true; Layout.margins: 24; spacing: 14
                RowLayout {
                    Label { text: root.selected >= 0 ? "帖子详情" : root.bookmarks ? "我的收藏" : "此刻，社区正在发生。"; font.pixelSize: 26; font.bold: true }
                    Item { Layout.fillWidth: true }
                    Button { visible: root.selected >= 0; text: "返回信息流"; onClicked: root.selected = -1 }
                }
                TextField { id: search; Layout.fillWidth: true; placeholderText: "搜索作者、动态或话题" }
                ListView {
                    id: feed; Layout.fillWidth: true; Layout.fillHeight: true; clip: true; spacing: 14; model: posts
                    delegate: Rectangle {
                        id: post
                        required property int index
                        required property string author; required property string handle
                        required property string body; required property string time
                        required property bool liked; required property bool saved
                        required property int likes; required property string replies; required property string hue
                        width: ListView.view.width; radius: 12; color: "white"
                        visible: (root.selected < 0 || root.selected === index) && (!root.bookmarks || saved) && (author + body).indexOf(search.text) >= 0
                        height: visible ? content.implicitHeight + 36 : 0
                        ColumnLayout {
                            id: content; anchors.left: parent.left; anchors.right: parent.right; anchors.top: parent.top; anchors.margins: 18; spacing: 12
                            RowLayout {
                                Rectangle { width: 42; height: 42; radius: 21; color: post.hue
                                    Label { anchors.centerIn: parent; text: post.author.charAt(0); color: "white"; font.pixelSize: 20 } }
                                ColumnLayout { spacing: 3; Label { text: post.author; font.bold: true } Label { text: post.handle; color: "#8590a2"; font.pixelSize: 12 } }
                                Item { Layout.fillWidth: true }
                                Label { text: post.time; color: "#8590a2" }
                            }
                            Label { text: post.body; textFormat: Text.PlainText; Layout.fillWidth: true; wrapMode: Text.Wrap; font.pixelSize: 16; lineHeight: 1.35 }
                            Label { visible: root.selected === post.index && text.length > 0; text: post.replies; textFormat: Text.PlainText; Layout.fillWidth: true; wrapMode: Text.Wrap; color: "#7060bd" }
                            RowLayout {
                                InteractionButton { iconName: "♡"; interactedIconName: "♥"; tooltip: "点赞"; interactionColor: "#db5d82"; interacted: post.liked; text: String(post.likes + (post.liked ? 1 : 0)); onClicked: root.toggle(post.index, "liked") }
                                InteractionButton { iconName: "☆"; interactedIconName: "★"; tooltip: "收藏"; interactionColor: "#b99043"; interacted: post.saved; onClicked: root.toggle(post.index, "saved") }
                                InteractionButton { iconName: "↩"; tooltip: "查看详情与回复"; onClicked: root.selected = post.index }
                                Item { Layout.fillWidth: true }
                                Button { text: "详情"; onClicked: root.selected = post.index }
                            }
                            RowLayout {
                                visible: root.selected === post.index
                                TextField { id: reply; Layout.fillWidth: true; placeholderText: "写下你的回复…"; maximumLength: 500 }
                                Button { text: "回复"; enabled: reply.text.trim().length > 0; onClicked: { posts.setProperty(post.index, "replies", post.replies + "\n我：" + reply.text.trim()); reply.clear() } }
                            }
                        }
                    }
                    ScrollBar.vertical: ScrollBar {}
                }
                Label { text: "本地模拟数据 · 不向外部服务发送内容"; color: "#8590a2" }
            }
        }
    }
    Popup {
        id: composer; anchors.centerIn: parent; width: Math.min(root.width - 40, 560); height: 340; modal: true; padding: 24
        ColumnLayout { anchors.fill: parent; spacing: 14
            Label { text: "分享一个新想法"; font.pixelSize: 22; font.bold: true }
            TextArea { id: draft; Layout.fillWidth: true; Layout.fillHeight: true; placeholderText: "有什么新鲜事？（最多 500 字）"; wrapMode: TextEdit.Wrap; selectByMouse: true }
            Label { text: draft.text.length + " / 500" }
            RowLayout { Button { text: "取消"; onClicked: composer.close() } Item { Layout.fillWidth: true }
                Button { text: "发布到本地信息流"; enabled: draft.text.trim().length > 0 && draft.text.length <= 500
                    onClicked: { posts.insert(0, {author: "我", handle: "@me@local.demo", body: draft.text.trim(), time: "刚刚", liked: false, saved: false, likes: 0, replies: "", hue: "#7767dc"}); draft.clear(); root.selected = -1; root.bookmarks = false; search.clear(); composer.close(); feed.positionViewAtBeginning() } }
            }
        }
    }
}

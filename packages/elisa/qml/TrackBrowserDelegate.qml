/*
   SPDX-FileCopyrightText: 2016 (c) Matthieu Gallien <matthieu_gallien@yahoo.fr>
   SPDX-FileCopyrightText: 2017 (c) Alexander Stippich <a.stippich@gmx.net>
   SPDX-FileCopyrightText: 2021 (c) Devin Lin <espidev@gmail.com>

   SPDX-License-Identifier: LGPL-3.0-or-later
 */

// Q-Browser: native KDE services/actions replaced with local signals and Controls.
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
ItemDelegate {
    id: mediaTrack
    property url trackUrl
    property var dataType
    property string title
    property string artist
    property string album
    property string albumArtist
    property string duration
    property url imageUrl
    property int trackNumber
    property int discNumber
    property int rating
    property bool hideDiscNumber
    property bool isSelected
    property bool isAlternateColor
    property bool detailedView: true
    property bool editingRating: false
    readonly property bool isFavorite: rating === 10

    signal enqueue()
    signal playNext()
    signal replaceAndPlay(var url)
    signal callOpenMetaDataView(var url, var entryType)
    signal trackRatingChanged(var url, var rating)


    Accessible.role: Accessible.ListItem
    Accessible.name: title
    Accessible.description: title
    Keys.onReturnPressed: enqueue()
    Keys.onEnterPressed: enqueue()
    highlighted: isSelected
    onClicked: replaceAndPlay(trackUrl)
    contentItem: RowLayout {
        Label { text: mediaTrack.trackNumber; Layout.preferredWidth: 24; color: "#6c7d90" }
        ColumnLayout {
            Layout.fillWidth: true
            Label { text: mediaTrack.title; font.bold: true }
            Label { text: mediaTrack.artist + " · " + mediaTrack.album; color: "#6c7d90"; font.pixelSize: 12 }
        }
        Label { text: mediaTrack.duration }
        ToolButton { text: "+ 队列"; onClicked: mediaTrack.enqueue() }
    }
}

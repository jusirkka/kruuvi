/* -*- coding: utf-8-unix -*-
 *
 * File: ./monitorapplet/contents/ui/DimensionalValue.qml
 *
 * Copyright (C) 2022 Jukka Sirkka
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
import QtQuick 2.6
import org.kde.plasma.components 3.0 as Components

Item {

  id: item

  property real value
  property int fontSize
  property string unit

  implicitHeight: left.implicitHeight
  implicitWidth: left.implicitWidth + right.implicitWidth

  Components.Label {
    id: left
    text: "" + (!isNaN(item.value) ? Math.round(item.value) : "-")
    font.pixelSize: item.fontSize
    elide: Text.ElideRight
  }

  Components.Label {
    id: right
    anchors.left: left.right
    anchors.baseline: left.verticalCenter
    anchors.baselineOffset: - item.fontSize * .1
    text: item.unit
    font.pixelSize: item.fontSize * .4
    elide: Text.ElideRight
  }
}

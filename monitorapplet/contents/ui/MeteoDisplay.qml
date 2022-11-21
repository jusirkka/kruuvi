/* -*- coding: utf-8-unix -*-
 *
 * File: ./monitorapplet/contents/ui/MeteoDisplay.qml
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

import QtQuick 2.2
import QtQuick.Layouts 1.1
import org.kde.plasma.core 2.0 as Core
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.components 3.0 as Components

Item {
  id: display
  property string name
  property string address
  property real temperature
  property real humidity
  property real pressure

  property real myWidth
  property real myHeight
  property real myHSpacing: myWidth * .05
  property real myVSpacing: myHeight * .05
  readonly property real fontFactor: .85

  implicitWidth: myWidth
  implicitHeight: myHeight

  MouseArea {
    anchors.fill: parent
    acceptedButtons: Qt.LeftButton
    property bool popped
    onPressed: {
      popped = plasmoid.expanded && (canvasAddress == address)
    }
    onClicked: {
      canvasAddress = address
      plasmoid.expanded = !popped
    }
    Component.onCompleted: {
      popped = false
    }
  }

  ColumnLayout {
    spacing: myVSpacing
    anchors.centerIn: parent

    Components.Label {
      text: display.name
      elide: Text.ElideMiddle
      Layout.preferredHeight: (myHeight - myVSpacing) * .25
      font.pixelSize: Layout.preferredHeight * fontFactor
      Layout.preferredWidth: myWidth
    }

    Rectangle {

      implicitWidth: myWidth
      implicitHeight: (myHeight - myVSpacing) * .75
      radius: 10
      color: "transparent"
      border.color: Core.ColorScope.textColor
      border.width: 2

      RowLayout {

        spacing: myHSpacing

        DimensionalValue {
          id: temperature
          value: display.temperature
          fontSize: Layout.preferredHeight * fontFactor
          unit: "°C"
          Layout.preferredHeight: (myHeight - myVSpacing) * .75
          Layout.preferredWidth: (myWidth - myHSpacing) * .5
        }

        ColumnLayout {
          DimensionalValue {
            value: display.pressure
            fontSize: Layout.preferredHeight * fontFactor
            unit: "hPa"
            Layout.preferredHeight: (myHeight - 2 * myVSpacing) * .75 * .5
            Layout.preferredWidth: (myWidth - myHSpacing) * .5
          }
          DimensionalValue {
            value: display.humidity
            fontSize: Layout.preferredHeight * fontFactor
            unit: "%"
            Layout.preferredHeight: (myHeight - 2 * myVSpacing) * .75 * .5
            Layout.preferredWidth: (myWidth - myHSpacing) * .5
          }
        }
      }
    }
  }
}

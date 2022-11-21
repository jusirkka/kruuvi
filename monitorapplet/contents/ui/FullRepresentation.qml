/* -*- coding: utf-8-unix -*-
 *
 * File: ./monitorapplet/contents/ui/FullRepresentation.qml
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
import QtQuick 2.15
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.components 3.0 as Components
import QtQuick.Controls 2.15
import "utils.js" as Utils

Item {
  id: meteogram

  property int imageWidth: 828 * units.devicePixelRatio
  property int imageHeight: 302 * units.devicePixelRatio

  property int headingHeight: theme.defaultFont.pixelSize * 2

  property bool expanded: plasmoid.expanded

  width: imageWidth
  height: heading.height + imageHeight

  function durationHelper(chkcd, func) {
    if (chkcd) {
      canvas.timeUtils = func(canvas.xmax)
      canvas.requestPaint()
      enddate.text = canvas.timeUtils.endDate()
      forward.enabled = false
    }
  }

  onExpandedChanged: {
    if (expanded) {
      canvas.requestPaint()
    }
  }

  Row {
    id: heading
    spacing: units.mediumSpacing
    Components.RadioButton {
      text: "24 Hours"
      checked: true
      onCheckedChanged: durationHelper(checked, Utils.hours24)
    }
    Components.RadioButton {
      text: "3 Days"
      onCheckedChanged: durationHelper(checked, Utils.days3)
    }
    Components.RadioButton {
      text: "Week"
      onCheckedChanged: durationHelper(checked, Utils.week)
    }
    Components.RadioButton {
      text: "2 Weeks"
      onCheckedChanged: durationHelper(checked, Utils.week2)
    }
    Components.RadioButton {
      text: "Month"
      onCheckedChanged: durationHelper(checked, Utils.month)
    }
  }

  Row {
    id: controls
    anchors.right: parent.right
    spacing: units.mediumSpacing


    Components.Button {
      id: rewind
      icon.name: 'arrow-left'
      height: enddate.height
      width: height
      onClicked: {
        canvas.timeUtils.rewind()
        canvas.requestPaint()
        enddate.text = canvas.timeUtils.endDate()
        forward.enabled = true
      }
    }

    Components.Button {
      id: forward
      icon.name: 'arrow-right'
      height: enddate.height
      width: height
      enabled: false
      onClicked: {
        canvas.timeUtils.forward()
        canvas.requestPaint()
        enddate.text = canvas.timeUtils.endDate()
        enabled = canvas.timeUtils.canForward()
      }
    }

    Components.Label {
      id: enddate
      text: canvas.timeUtils.endDate()
    }
  }

  MeteoCanvas {
    id: canvas
    anchors.top: heading.bottom
    width: imageWidth
    height: imageHeight
  }
}

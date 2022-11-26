/* -*- coding: utf-8-unix -*-
 *
 * File: ./monitorapplet/contents/config/ConfigGeneral.qml
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
import QtQuick.Controls 1.3
import QtQuick.Layouts 1.1
import QtQuick.Dialogs 1.2
import kvanttiapina.kruuvi.private 1.0 as KRuuvi
import org.kde.plasma.core 2.0 as Core

Item {

  width: childrenRect.width
  height: childrenRect.height


  property string cfg_devicesJson

  ListModel {
    id: devicesModel
  }

  KRuuvi.DBReader {
    id: reader
  }

  Component.onCompleted: {
    const devices = JSON.parse(plasmoid.configuration.devicesJson)
    const addresses = reader.addresses()
    addresses.forEach(function (addr) {
      var location = "Unset"
      var enabled = false
      const d0 = devices.find(d => d.address === addr)
      if (!!d0) {
        location = d0.location
        enabled = d0.enabled
      }
      devicesModel.append
          ({
             address: addr,
             location: location,
             enabled: enabled
           })
    })
  }

  function devicesModelChanged() {
    var newDevicesArray = []
    for (var i = 0; i < devicesModel.count; i++) {
      const d = devicesModel.get(i)
      newDevicesArray.push
          ({
             address: d.address,
             location: d.location,
             enabled: d.enabled
           })
    }
    cfg_devicesJson = JSON.stringify(newDevicesArray)
    // console.log(cfg_devicesJson)
  }

  Dialog {
    id: locationDialog
    title: i18n('Change Location')

    standardButtons: StandardButton.Ok | StandardButton.Cancel

    onAccepted: {
      devicesModel.setProperty(locationDialog.tableIndex, 'location', locationField.text)
      devicesModelChanged()
      locationDialog.close()
    }

    property int tableIndex: 0

    TextField {
      id: locationField
      placeholderText: i18n('Enter location')
      width: parent.width
    }
  }


  Label {
    id: headerLabel
    text: i18n('Devices')
    font.bold: true
  }

  TableView {
    id: devicesTable
    width: parent.width
    anchors.top: headerLabel.bottom
    anchors.topMargin: Core.Units.mediumSpacing
    model: devicesModel

    TableViewColumn {
      id: addressCol
      role: 'address'
      title: i18n('Address')
      width: parent.width * 0.3

      delegate: Label {
        text: styleData.value
        anchors.left: parent ? parent.left : undefined
        anchors.leftMargin: 5
        anchors.right: parent ? parent.right : undefined
        anchors.rightMargin: 5
      }
    }

    TableViewColumn {
      role: 'location'
      title: i18n('Location')
      width: parent.width * 0.4

      delegate: MouseArea {

        anchors.fill: parent

        Label {
          id: locationText
          text: styleData.value
          height: parent.height
          anchors.left: parent.left
          anchors.leftMargin: 5
          anchors.right: parent.right
          anchors.rightMargin: 5
        }
        cursorShape: Qt.PointingHandCursor

        onClicked: {
          locationDialog.open()
          locationDialog.tableIndex = styleData.row
          locationField.text = locationText.text
          locationField.focus = true
          locationField.selectAll()
        }
      }
    }

    TableViewColumn {
      id: enabledCol
      role: 'enabled'
      title: i18n('Active')
      width: parent.width * 0.1

      delegate: CheckBox {
        checked: styleData.value
        onCheckedChanged: {
          devicesModel.setProperty(styleData.row, 'enabled', checked)
          devicesModelChanged()
        }
      }
    }
  }
}

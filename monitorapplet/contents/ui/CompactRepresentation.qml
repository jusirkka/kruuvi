/* -*- coding: utf-8-unix -*-
 *
 * File: ./monitorapplet/contents/ui/CompactRepresentation.qml
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
import QtQuick.Controls 2.12 as Ctrl
import org.kde.plasma.core 2.0 as Core
import org.kde.plasma.extras 2.0 as Extras
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.components 3.0 as Components
import QtQuick.Layouts 1.2

ListView {
  id: listView

  anchors.fill: parent

  spacing: 3

  property bool vertical: (plasmoid.formFactor === Core.Types.Vertical)
  property string devicesJson: plasmoid.configuration.devicesJson
  // property string devicesJson: ''
  //  Component.onCompleted: {
  //    devicesJson = '[{"address":"D2:38:63:2A:6F:E1","location":"Asunto","enabled":true},{"address":"E4:80:DB:3E:67:4C","location":"Parveke","enabled":true}]'
  //  }

  onDevicesJsonChanged: {
    const devices = JSON.parse(devicesJson)
    if (devices.length === 0) return
    const lastValues = JSON.parse(plasmoid.configuration.lastValuesJson)
    // const lastValues = JSON.parse('[{"name":"Asunto","address":"D2:38:63:2A:6F:E1","temperature":8.27,"humidity":92.96000000000001,"pressure":1008.78},{"name":"Parveke","address":"E4:80:DB:3E:67:4C","temperature":8.27,"humidity":92.96000000000001,"pressure":1008.78}]')

    deviceModel.clear()

    var srcs = []

    devices.forEach(function (item) {
      if (!item.enabled) return
      var modelItem = {
        name: item.location,
        address: item.address,
        temperature: NaN,
        humidity: NaN,
        pressure: NaN
      }
      const d0 = lastValues.find(v => v.name === item.location)
      if (!!d0) {
        modelItem.temperature = d0.temperature
        modelItem.humidity = d0.humidity
        modelItem.pressure = d0.pressure
      }
      deviceModel.append(modelItem)
      srcs.push(item.address)
    })

    ruuvi.connectedSources = srcs
    ruuvi.interval = 10000 // ensure that all sources are updated
  }


  orientation: (vertical ? Qt.Vertical : Qt.Horizontal)

  Layout.minimumWidth: Math.max(count * meteoMetrics.width, holderMetrics.width)

  Core.DataSource {
    id: ruuvi
    engine: "ruuvimonitor"
    onNewData: {
      var lastValuesArray = []
      for (var i = 0; i < deviceModel.count; i++) {
        const d = deviceModel.get(i)
        if (d.address === sourceName) {
          deviceModel.setProperty(i, "temperature", data.temperature)
          deviceModel.setProperty(i, "humidity", data.humidity)
          deviceModel.setProperty(i, "pressure", data.pressure)
        }
        lastValuesArray.push
            ({
               name: d.name,
               address: d.address,
               temperature: d.temperature,
               humidity: d.humidity,
               pressure: d.pressure
             })
      }
      plasmoid.configuration.lastValuesJson = JSON.stringify(lastValuesArray)
      interval = 0
    }
  }

  model: ListModel {
    id: deviceModel
  }

  delegate: MeteoDisplay {
    name: model.name
    address: model.address
    temperature: model.temperature
    humidity: model.humidity
    pressure: model.pressure
    myWidth: listView.width / listView.count - (1 + listView.count) * listView.spacing
    myHeight: listView.height
  }

  TextMetrics {
    id: holderMetrics
    text: holder.text + "xxx"
  }

  TextMetrics {
    id: meteoMetrics
    text: "Livingroom" + "xxx"
  }

  Extras.PlaceholderMessage {
    id: holder
    text: "No Sensors"
    visible: listView.count == 0
  }
}

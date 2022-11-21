/* -*- coding: utf-8-unix -*-
 *
 * File: ./monitorapplet/contents/ui/MeteoCanvas.qml
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

import "utils.js" as Utils
import kvanttiapina.kruuvi.private 1.0 as KRuuvi
import org.kde.plasma.core 2.0 as Core

Canvas {
  id: canvas
  renderTarget: Canvas.Image
  antialiasing: true

  property string address: canvasAddress
  property var timeUtils: ({})

  readonly property real chartTopMargin: leftMetrics.height + units.smallSpacing * 2
  readonly property real chartBottomMargin: chartTopMargin
  readonly property real chartLeftMargin: leftMetrics.width + units.smallSpacing + units.mediumSpacing
  readonly property real chartRightMargin: rightMetrics.width + units.smallSpacing + units.mediumSpacing

  readonly property real chartHeight: height - chartTopMargin - chartBottomMargin
  readonly property real chartWidth: width - chartLeftMargin - chartRightMargin

  readonly property int xmax: 28
  readonly property int ymax: 13

  property real tmin: 0
  property real tdelta: 20

  readonly property real hmin: 20
  readonly property real hdelta: ymax * 10

  readonly property real pmin: hmin + 900
  readonly property real pdelta: hdelta

  readonly property color tcolor: Qt.rgba(1, 0.3, 0.3, 1)
  readonly property color hcolor: Qt.rgba(0.3, 1, 0.3, 1)
  readonly property color pcolor: Qt.rgba(0.3, 0.3, 1, 1)

  function valueT(y) {
    return chartTopMargin + (1 - (y - tmin) / tdelta) * chartHeight
  }

  function valueH(y) {
    return chartTopMargin + (1 - (y - hmin) / hdelta) * chartHeight
  }

  function valueP(y) {
    return chartTopMargin + (1 - (y - pmin) / pdelta) * chartHeight
  }

  Component.onCompleted: {
    timeUtils = Utils.hours24(xmax)
  }

  KRuuvi.DBReader {
    id: db
  }


  TextMetrics {
    id: leftMetrics
    text: "°C"
    font.pointSize: 8
  }

  TextMetrics {
    id: rightMetrics
    text: "hPa / %"
    font: leftMetrics.font
  }

  onAddressChanged: {
    requestPaint()
  }

  function drawGrid(ctx) {
    ctx.save();

    ctx.fillStyle = Core.Theme.backgroundColor
    ctx.fillRect(0, 0, canvas.width, canvas.height);

    ctx.fillStyle = "#272822";
    ctx.fillRect(chartLeftMargin, chartTopMargin, chartWidth, chartHeight);

    ctx.strokeStyle = "#423a2f";
    ctx.beginPath();
    for (var i = 0; i <= ymax; i++) {
      ctx.moveTo(chartLeftMargin, chartTopMargin + i * chartHeight / ymax)
      ctx.lineTo(chartLeftMargin + chartWidth, chartTopMargin + i * chartHeight / ymax)
    }

    for (i = 0; i <= xmax; i++) {
      ctx.moveTo(chartLeftMargin + i * chartWidth / xmax, chartTopMargin)
      ctx.lineTo(chartLeftMargin + i * chartWidth / xmax, chartTopMargin + chartHeight)
    }
    ctx.stroke();

    ctx.restore();
  }

  function drawUnits(ctx) {
    ctx.save();

    ctx.textAlign = "end"

    ctx.textBaseline = "alphabetic"
    ctx.fillStyle = "#ffffff"
    ctx.font = leftMetrics.font.pointSize + "pt " + leftMetrics.font.family

    ctx.fillText(leftMetrics.text, chartLeftMargin - units.smallSpacing, chartTopMargin - units.mediumSpacing)

    ctx.textBaseline = "middle"
    ctx.fillStyle = tcolor

    let limits = db.temperatureLimits(address, timeUtils.startInstance(), timeUtils.duration())
    var nmin = Math.floor(limits[0] - .5)
    var nmax = Math.ceil(limits[1] + .5)
    var M = nmax - nmin + 1
    var D = Math.ceil(M / ymax)

    tmin = Math.ceil((nmin + nmax - ymax * D) / 2)
    tdelta = ymax * D

    for (var i = 0; i <= ymax; i++) {
      var x = tmin + (ymax - i) * D
      ctx.fillText("" + x, chartLeftMargin - units.smallSpacing, chartTopMargin + i * chartHeight / ymax)
    }

    ctx.textBaseline = "alphabetic"
    ctx.fillStyle = "#ffffff"
    ctx.font = rightMetrics.font.pointSize + "pt " + rightMetrics.font.family

    ctx.fillText(rightMetrics.text, canvas.width - units.mediumSpacing, chartTopMargin - units.mediumSpacing)

    ctx.textBaseline = "middle"
    ctx.fillStyle = pcolor

    for (i = 0; i <= ymax; i++) {
      var p = hmin + (ymax - i) * 10
      if (p >= 100) {
        p += 900
      } else {
        ctx.fillStyle = hcolor
      }
      ctx.fillText("" + p, canvas.width - units.mediumSpacing, chartTopMargin + i * chartHeight / ymax)
    }

    ctx.textBaseline = "alphabetic"
    ctx.fillStyle = "#ffffff"
    ctx.textAlign = "center"

    for (i = 0; i < xmax / 2; i++) {
      let label = timeUtils.label(i)
      ctx.fillText(label, chartLeftMargin + (2 * i + .5) * chartWidth / xmax, canvas.height - units.mediumSpacing)
      if (label === "00") {
        ctx.beginPath();
        ctx.strokeStyle = "#524a3f";
        ctx.lineWidth = 1
        ctx.moveTo(chartLeftMargin + 2 * i * chartWidth / xmax, chartTopMargin)
        ctx.lineTo(chartLeftMargin + 2 * i * chartWidth / xmax, chartTopMargin + chartHeight)
        ctx.stroke();
      }
    }

    ctx.restore()
  }

  function drawValues(ctx, func, fetcher, col) {
    ctx.save();

    ctx.strokeStyle = col;
    ctx.beginPath();

    let nump = Math.ceil(timeUtils.duration() * timeUtils.sampleFrequency())
    var needToMove = true
    let values = fetcher(address, timeUtils.startInstance(), timeUtils.duration(), nump)

    for (var i = 0; i < nump; i++) {
      if (isNaN(values[i])) {
        // console.log("undefined", i)
        needToMove = true
        continue
      }
      let s = timeUtils.startInstance() + timeUtils.duration() * i / nump
      let x = chartLeftMargin + timeUtils.normalize(s) * chartWidth
      if (needToMove) {
        ctx.moveTo(x, func(values[i]))
        needToMove = false
      } else {
        ctx.lineTo(x, func(values[i]))
      }
    }

    ctx.stroke();

    ctx.restore();
  }

  onPaint: {
    var ctx = canvas.getContext("2d");
    ctx.lineWidth = 1;
    drawGrid(ctx);
    drawUnits(ctx)
    drawValues(ctx, valueT, db.temperature, tcolor)
    drawValues(ctx, valueP, db.pressure, pcolor)
    drawValues(ctx, valueH, db.humidity, hcolor)
  }
}


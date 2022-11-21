/* -*- coding: utf-8-unix -*-
 *
 * File: ./monitorapplet/contents/ui/utils.js
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


function create(o) {
  function F() {}
  F.prototype = o;
  return new F();
}


var timeUtilsProto = {}

timeUtilsProto.startInstance = function () {
  return this._s0
}

timeUtilsProto.duration = function () {
  return this._s1 - this._s0
}

timeUtilsProto.canForward = function () {
  return this._s1 < (Date.now() / 1000)
}

timeUtilsProto.forward = function () {
  if (this.canForward()) {
    let d = this.duration()
    this._s0 += d
    this._s1 += d
  }
}

timeUtilsProto.rewind = function () {
  let d = this.duration()
  this._s0 -= d
  this._s1 -= d
}

timeUtilsProto.sampleFrequency = function () {
  return this._sampleFrequency
}

timeUtilsProto.normalize = function (s) {
  return (s - this._s0) / this.duration()
}

timeUtilsProto._init = function (cnt, step) {
  let h = (new Date()).getHours() % (2 * step)
  let s = Date.now() / 1000

  // console.log("hours =", h)

  this._s1 = (Math.floor(s / 3600) + 2 * step - h) * 3600

  this._s0 = this._s1 - cnt * step * 3600
  this._doubleStep = 2 * step * 3600


}

timeUtilsProto._label24 = function (i) {
  let h = (new Date(1000 * (this._s0 + i * this._doubleStep))).getHours()
  let pad = h < 10 ? "0" : ""

  return pad + h
}

timeUtilsProto._labelDate = function (i) {
  let h = (new Date(1000 * (this._s0 + i * this._doubleStep))).getDate()

  return "" + h
}

timeUtilsProto._labelWeekDay = function (i) {
  let d = new Date(1000 * (this._s0 + i * this._doubleStep))
  return d.toLocaleString(Qt.locale(), 'ddd')
}

timeUtilsProto.endDate = function () {
  let d = new Date(1000 * (this._s1 - 1))
  return d.toLocaleString(Qt.locale(), 'yyyy-MM-dd')
}

function hours24(cnt) {
  var obj = create(timeUtilsProto)

  obj._init(cnt, 1)

  obj.label = obj._label24
  obj._sampleFrequency = 4.5 / 3600
  return obj
}


function days3(cnt) {
  var obj = create(timeUtilsProto)

  obj._init(cnt, 3)

  obj.label = obj._label24
  obj._sampleFrequency = 4.5 / 3600
  return obj
}

function week(cnt) {
  var obj = create(timeUtilsProto)

  obj._init(cnt, 6)

  obj.label = obj._label24
  obj._sampleFrequency = 4.5 / 3600
  return obj
}

function week2(cnt) {
  var obj = create(timeUtilsProto)

  obj._init(cnt, 12)

  obj.label = obj._labelWeekDay
  obj._sampleFrequency = 4.5 / 3600
  return obj
}

function month(cnt) {
  var obj = create(timeUtilsProto)

  obj._init(cnt, 24)

  obj.label = obj._labelDate
  obj._sampleFrequency = 4.5 / 3600
  return obj
}

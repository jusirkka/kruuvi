/* -*- coding: utf-8-unix -*-
 *
 * File: ./monitorapplet/contents/config/config.qml
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
import org.kde.plasma.configuration 2.0

ConfigModel {
  ConfigCategory {
    name: i18n('General')
    icon: Qt.resolvedUrl('../images/ruuvi.png').replace('file://', '')
    source: '../config/ConfigGeneral.qml'
  }
}

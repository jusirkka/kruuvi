/* -*- coding: utf-8-unix -*-
 *
 * File: ./src/plasmoidplugin.cpp
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
#include "plasmoidplugin.h"
#include "dbreader.h"

#include <QtQml>

void PlasmoidPlugin::registerTypes(const char* uri) {
  Q_ASSERT(uri == QLatin1String("kvanttiapina.kruuvi.private"));
  qmlRegisterType<DBReader>(uri, 1, 0, "DBReader");
}

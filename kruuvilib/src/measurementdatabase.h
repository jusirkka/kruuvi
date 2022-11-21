/* -*- coding: utf-8-unix -*-
 *
 * File: ./kruuvilib/src/measurementdatabase.h
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
#pragma once

#include "sqlitedatabase.h"

struct Measurement {
  Measurement(quint32 stamp, float v)
    : ts(stamp)
    , value(v) {}
  quint32 ts;
  float value;
};

using MeasurementVector = QVector<Measurement>;


class MeasurementDatabase: public SQLiteDatabase {
public:

  static void createTables();

  MeasurementDatabase(const QString& connName);
  ~MeasurementDatabase() = default;

  quint32 locationId(const QString& addr);
  quint32 timestamp(quint32 locId, const QString& table);
  void insertMeasurements(quint32 locId, const QString& table, const MeasurementVector& measurements);
  QStringList addresses();

  MeasurementVector measurements(quint32 locId, const QString& table, quint32 start, quint32 end);

private:


};


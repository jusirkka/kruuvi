/* -*- coding: utf-8-unix -*-
 *
 * File: ./src/dbreader.cpp
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
#include "dbreader.h"
#include "measurementdatabase.h"
#include <QVariant>
#include <QDebug>

DBReader::DBReader(QObject* parent)
  : QObject(parent) {}

DBReader::~DBReader() {}

QVariantList DBReader::addresses() {
  MeasurementDatabase db("DBReader::addresses");

  const auto as = db.addresses();
  QVariantList aps;
  for (const auto& a: as) {
    aps << a;
  }
  return aps;
}


QVariantList DBReader::temperature(const QString& addr, quint32 start, quint32 duration, quint16 samples) {
  return fetchData(addr, start, start + duration, samples, "temperature");
}

QVariantList DBReader::humidity(const QString& addr, quint32 start, quint32 duration, quint16 samples) {
  return fetchData(addr, start, start + duration, samples, "humidity");
}

QVariantList DBReader::pressure(const QString& addr, quint32 start, quint32 duration, quint16 samples) {
  return fetchData(addr, start, start + duration, samples, "pressure");
}


QVariantList DBReader::fetchData(const QString& addr, quint32 start, quint32 end, quint16 samples, const QString& table) {
  MeasurementDatabase db("DBReader::fetch");

  MeasurementVector values = db.measurements(db.locationId(addr), table, start - 3600, end + 3600);
  QVariantList results;

  // qDebug() << "fetched" << values.size() << "values";

  const double D = end - start;

  if (!values.isEmpty()) {
    double s = start + results.size() * D / samples;
    while (s < values.first().ts && results.size() < samples) {
      results << undefined;
      s = start + results.size() * D / samples;
    }

    quint16 i = 0;
    while (results.size() < samples) {
      const double s = start + results.size() * D / samples;
      while (i < values.size() && values[i].ts <= s) i++;
      if (i >= values.size()) break;
      const double ds = values[i].ts - values[i - 1].ts;
      if (ds > largeGap) {
        results << undefined;
      } else {
        const double s0 = values[i - 1].ts;
        const double s1 = values[i].ts;
        const double v0 = values[i - 1].value;
        const double v1 = values[i].value;

        results << (s - s0) / ds * v1 + (s1 - s) / ds * v0;
      }
      i--;
    }
  }

  while (results.size() < samples) {
    results << undefined;
  }

  return results;
}


QVariantList DBReader::temperatureLimits(const QString& addr, quint32 start, quint32 duration) {
  MeasurementDatabase db("DBReader::limits");

  MeasurementVector values = db.measurements(db.locationId(addr), "temperature", start, start + duration);

  // qDebug() << "fetched" << values.size() << "values";

  double tmin = 100;
  double tmax = -100;

  for (const Measurement& v: values) {
    if (v.value < tmin) tmin = v.value;
    if (v.value > tmax) tmax = v.value;
  }

  QVariantList results;
  results << tmin << tmax;
  return results;
}

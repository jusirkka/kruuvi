/* -*- coding: utf-8-unix -*-
 *
 * File: ./src/dbreader.h
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

#include <QObject>
#include <limits>

class DBReader: public QObject {

  Q_OBJECT

public:

  DBReader(QObject* parent = nullptr);
  ~DBReader();

  Q_INVOKABLE QVariantList addresses();
  Q_INVOKABLE QVariantList temperature(const QString& addr, quint32 start, quint32 duration, quint16 samples);
  Q_INVOKABLE QVariantList humidity(const QString& addr, quint32 start, quint32 duration, quint16 samples);
  Q_INVOKABLE QVariantList pressure(const QString& addr, quint32 start, quint32 duration, quint16 samples);

  Q_INVOKABLE QVariantList temperatureLimits(const QString& addr, quint32 start, quint32 duration);

private:

  QVariantList fetchData(const QString& addr, quint32 start, quint32 end, quint16 samples, const QString& table);

  static const inline double undefined = std::numeric_limits<double>::quiet_NaN();
  static const inline double largeGap = 5 * 3600;

};

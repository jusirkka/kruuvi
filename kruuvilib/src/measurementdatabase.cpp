/* -*- coding: utf-8-unix -*-
 *
 * File: ./kruuvilib/src/measurementdatabase.cpp
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
#include "measurementdatabase.h"

#include <QDebug>
#include <QDateTime>
#include <QSqlError>


void MeasurementDatabase::createTables() {
  {
    auto db = QSqlDatabase::addDatabase("QSQLITE", "MeasurementDatabase::createTables");

    db.setDatabaseName(databaseName("measurements"));
    db.open();
    auto query = QSqlQuery(db);

    query.exec("create table if not exists location ("
               "id integer primary key autoincrement, "
               "address text unique)");

    query.exec("create table if not exists temperature ("
               "id integer primary key autoincrement, "
               "location_id integer not null, "
               "timestamp integer not null, "
               "value real not null)");

    query.exec("create table if not exists pressure ("
               "id integer primary key autoincrement, "
               "location_id integer not null, "
               "timestamp integer not null, "
               "value real not null)");

    query.exec("create table if not exists humidity ("
               "id integer primary key autoincrement, "
               "location_id integer not null, "
               "timestamp integer not null, "
               "value real not null)");

    db.close();
  }
  QSqlDatabase::removeDatabase("MeasurementDatabase::createTables");
}

MeasurementDatabase::MeasurementDatabase(const QString& connName)
  : SQLiteDatabase(connName)
{
  m_DB.setDatabaseName(databaseName("measurements"));
  m_DB.open();
  m_Query = QSqlQuery(m_DB);
}

quint32 MeasurementDatabase::locationId(const QString& addr) {

  auto r0 = prepare("select id from location where address = ?");
  r0.bindValue(0, addr);
  exec(r0);

  if (r0.first()) {
    return r0.value(0).toInt();
  }

  // Not found, insert

  r0 = prepare("insert into location (address) values(?)");
  r0.bindValue(0, addr);
  exec(r0);

  return r0.lastInsertId().toUInt();
}

QStringList MeasurementDatabase::addresses() {
  QStringList as;
  auto r0 = exec("select address from location");
  while (r0.next()) {
    as << r0.value(0).toString();
  }
  return as;
}

quint32 MeasurementDatabase::timestamp(quint32 locId, const QString& table) {
  Q_ASSERT(m_DB.tables().contains(table));
  auto sql = QString("select max(timestamp) from %1 where location_id = ?").arg(table);
  auto r0 = prepare(sql);
  r0.bindValue(0, locId);
  exec(r0);

  if (r0.first()) {
    return r0.value(0).toInt();
  }

  return 0;
}

void MeasurementDatabase::insertMeasurements(quint32 locId, const QString& table,
                                             const MeasurementVector& measurements) {

  Q_ASSERT(m_DB.tables().contains(table));
  const auto sql = QString("insert into %1 (location_id, timestamp, value) values (?, ?, ?)")
      .arg(table);

  if (!transaction()) {
    qWarning() << "Transactions not supported";
  }

  for (const Measurement& m: measurements) {
    auto r0 = prepare(sql);
    r0.bindValue(0, locId);
    r0.bindValue(1, m.ts);
    r0.bindValue(2, m.value);
    exec(r0);
  }

  if (!commit()) {
    qWarning() << "Transactions/Commits not supported";
  }
}

MeasurementVector MeasurementDatabase::measurements(quint32 locId, const QString& table, quint32 start, quint32 end) {
  Q_ASSERT(m_DB.tables().contains(table));
  const auto sql = QString("select timestamp, value from %1 where location_id = ? and timestamp > ? and timestamp < ? order by timestamp")
      .arg(table);

  // qDebug() << sql << locId << start << end;

  auto r0 = prepare(sql);
  r0.bindValue(0, locId);
  r0.bindValue(1, start);
  r0.bindValue(2, end);
  exec(r0);

  MeasurementVector results;
  while (r0.next()) {
    results << Measurement(r0.value(0).toUInt(), r0.value(1).toDouble());
  }

  return results;
}

/* -*- coding: utf-8-unix -*-
 *
 * File: ./kruuvilib/src/sqlitedatabase.h
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

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>

class DatabaseError {
public:
  DatabaseError(QString msg): m_detail(std::move(msg)) {}
  const QString msg() const {return m_detail;}
private:
  QString m_detail;
};

class PlatformError {
public:
  PlatformError(QString msg): m_detail(std::move(msg)) {}
  const QString msg() const {return m_detail;}
private:
  QString m_detail;
};

class SQLiteDatabase {
public:

  static QString databaseName(const QString& bname);

  SQLiteDatabase(const QString& connName);
  virtual ~SQLiteDatabase();

  const QSqlQuery& exec(const QString& sql);
  const QSqlQuery& prepare(const QString& sql);
  void exec(QSqlQuery& query);
  bool transaction();
  bool commit();
  bool rollback();
  void close();

protected:

  void checkError() const;

  QSqlDatabase m_DB;
  QSqlQuery m_Query;
};


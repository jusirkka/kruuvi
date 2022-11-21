/* -*- coding: utf-8-unix -*-
 *
 * File: ./logreader/src/ruuvireader.h
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

#include <BluezQt/Manager>

class RuuviReader: public QObject {

  Q_OBJECT

public:

  RuuviReader(const QStringList& addresses, QObject* parent = nullptr);
  ~RuuviReader();
  static void sigHandler(int sig);

public slots:

  void handleSig();
  void scan();
  void readLog();
  void findDevice();

signals:

  void initialized();
  void deviceFound();

private slots:

  void deviceAdded(BluezQt::DevicePtr device);
  void handleRXNotify(const QByteArray value);
  void setupNUS(BluezQt::GattServiceRemotePtr srv);

private:

  static inline const int RuuviMID = 1177;
  static inline const QString NUSUUID = "6E400001-B5A3-F393-E0A9-E50E24DCCA9E";
  static inline const QString NUSUUID_TX = "6E400002-B5A3-F393-E0A9-E50E24DCCA9E";
  static inline const QString NUSUUID_RX = "6E400003-B5A3-F393-E0A9-E50E24DCCA9E";
  static inline const char addr_env = 0x3a;
  static inline const char addr_temperature = 0x30;
  static inline const char addr_humidity = 0x31;
  static inline const char addr_pressure = 0x32;
  static inline const char op_req = 0x11;
  static inline const char op_rsp = 0x10;
  static inline const int StopScanMSecs = 15000;
  static inline const int WaitBeforeErrorMSecs = 60000;

  static inline const QMap<quint8, QString> tables = {
    {addr_temperature, "temperature"},
    {addr_humidity, "humidity"},
    {addr_pressure, "pressure"}
  };

  void setupScan();
  void cleanupAndExit();
  void connectDevice(BluezQt::DevicePtr);
  void disconnectDevice();
  void updateDB();

  static inline int m_sigFd[2] = {0, 0};

  struct Private;
  Private* const d;

};

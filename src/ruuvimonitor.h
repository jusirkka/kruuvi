/* -*- coding: utf-8-unix -*-
 *
 * File: ./src/ruuvimonitor.h
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

#include <Plasma/DataEngine>
#include <BluezQt/Manager>


class RuuviEngine: public Plasma::DataEngine {

  Q_OBJECT

public:

  RuuviEngine(QObject* parent, const QVariantList& args);
  ~RuuviEngine() override;

protected:

  bool sourceRequestEvent(const QString& name) override;
  bool updateSourceEvent(const QString& name) override;

private slots:

  void kickoff();
  void deviceAdded(BluezQt::DevicePtr device);
  void deviceRemoved(BluezQt::DevicePtr device);

signals:

  void initialized();

private:

  bool setDataFromManufacturerData(const QString& name);
  void scan();
  void stopScanning();
  void setupScan();
  bool allUpdated() const;

  static inline const int StopScanMSecs = 15000;
  static inline const int RefreshStep = 20 * 60 * 1000; // 20 mins
  // static inline const int RefreshStep = 10 * 1000;
  static inline const int RuuviMID = 1177;


  struct Private;
  Private* const d;

};

/* -*- coding: utf-8-unix -*-
 *
 * File: ./src/ruuvimonitor.cpp
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
#include "ruuvimonitor.h"

#include <QTimer>
#include <QDebug>
#include <KPluginFactory>
#include <BluezQt/InitManagerJob>
#include <QDataStream>
#include <cmath>

using DeviceMap = QMap<QString, BluezQt::DevicePtr>;
using BoolMap = QMap<QString, bool>;

struct RuuviEngine::Private {
  BluezQt::Manager *m_manager = nullptr;
  DeviceMap m_tags;
  BoolMap m_updated;
  QTimer* m_deviceSearchTimer = nullptr;
  QTimer* m_refreshTimer = nullptr;
};


RuuviEngine::RuuviEngine(QObject* parent, const QVariantList& args)
  : Plasma::DataEngine(parent, args)
  , d(new Private) {


  d->m_deviceSearchTimer = new QTimer(this);
  d->m_deviceSearchTimer->setSingleShot(true);
  d->m_deviceSearchTimer->setInterval(StopScanMSecs);
  connect(d->m_deviceSearchTimer, &QTimer::timeout, [this] () {
    if (d->m_manager->usableAdapter() && d->m_manager->usableAdapter()->isDiscovering()) {
      qDebug() << "Timeout, Stop scan";
      d->m_manager->usableAdapter()->stopDiscovery();
    }
  });

  d->m_refreshTimer = new QTimer(this);
  d->m_deviceSearchTimer->setSingleShot(false);
  d->m_refreshTimer->setInterval(RefreshStep);
  connect(d->m_refreshTimer, &QTimer::timeout, [this] () {
    for (auto it = d->m_updated.begin(); it != d->m_updated.end(); ++it) {
      qDebug() << "Refreshing" << it.key();
      it.value() = false;
    }
    scan();
  });

  d->m_refreshTimer->start();

  d->m_manager = new BluezQt::Manager(this);

  connect(d->m_manager, &BluezQt::Manager::deviceAdded, this, &RuuviEngine::deviceAdded);
  connect(d->m_manager, &BluezQt::Manager::deviceRemoved, this, &RuuviEngine::deviceRemoved);

  connect(this, &RuuviEngine::initialized, this, &RuuviEngine::kickoff);

  // Initialize BluezQt
  BluezQt::InitManagerJob* job = d->m_manager->init();

  job->start();
  connect(job, &BluezQt::InitManagerJob::result, this, [this] (BluezQt::InitManagerJob* job) {
    if (job->error()) {
      qWarning() << "Bluez manager init failed:" << job->errorText();
    } else {
      emit initialized();
    }
  });

  connect(this, &RuuviEngine::sourceRemoved, this, [this] (const QString& addr) {
    if (d->m_tags.contains(addr)) {
      qDebug() << "Removing Dataengine source" << addr;
      if (d->m_tags[addr] != nullptr) {
        disconnect(d->m_tags[addr].data(), nullptr, this, nullptr);
      }
      d->m_tags.remove(addr);
      d->m_updated.remove(addr);
    }
  });
}

void RuuviEngine::stopScanning() {
  if (d->m_manager->usableAdapter() && d->m_manager->usableAdapter()->isDiscovering()) {
    qDebug() << "Stop scanning";
    d->m_manager->usableAdapter()->stopDiscovery();
  }
  d->m_deviceSearchTimer->stop();
}

void RuuviEngine::kickoff() {
  if (!allUpdated()) { // at least one needs update
    scan();
  }
}

void RuuviEngine::setupScan() {

  if (!d->m_manager->usableAdapter()) return;

  qDebug() << "Setup scan filter";

  const QVariantMap dict {
    {"Transport", "le"},
    {"DuplicateData", true},
    {"Pattern", "Ruuvi"},
  };

  BluezQt::PendingCall* call = d->m_manager->usableAdapter()->setDiscoveryFilter(dict);
  call->waitForFinished();
  if (call->error()) {
    qWarning() << "Error setting up scan filter:" << call->errorText();
  }
}

bool RuuviEngine::allUpdated() const {
  const auto upd = d->m_updated.values();
  auto it = std::find_if(upd.cbegin(), upd.cend(), [] (bool v) {return !v;});
  return it == upd.cend();
}

void RuuviEngine::scan() {
  if (!d->m_manager->usableAdapter()) {
    qWarning() << "No usable adapter exists";
    return;
  }

  if (allUpdated()) {
    qDebug() << "Nothing to update";
    return;
  }

  if (d->m_manager->usableAdapter()->isDiscovering()) {
    qDebug() << "Already discovering";
    return;
  }

  setupScan();

  qDebug() << "Start scan";

  d->m_deviceSearchTimer->start();
  d->m_manager->usableAdapter()->startDiscovery();
}

void RuuviEngine::deviceAdded(BluezQt::DevicePtr p) {
  qDebug() << "device added" << p->name() << p->address();
  if (d->m_tags.contains(p->address()) && d->m_tags[p->address()] == nullptr) {
    qDebug() << "connect device" << p->name() << p->address();
    d->m_tags[p->address()] = p;
    connect(p.data(), &BluezQt::Device::manufacturerDataChanged, this, [this] () {
      auto s = qobject_cast<BluezQt::Device*>(sender());
      qDebug() << "mandata changed" << s->name();
      setDataFromManufacturerData(s->address());
    });
  }
}

void RuuviEngine::deviceRemoved(BluezQt::DevicePtr p) {
  qDebug() << "device removed" << p->name() << p->address();
  if (!d->m_tags.contains(p->address())) return;
  disconnect(p.data(), nullptr, this, nullptr);
  d->m_tags[p->address()] = nullptr;
  d->m_updated[p->address()] = true;
}


RuuviEngine::~RuuviEngine() {}

template<typename T> T read_value(QDataStream& stream) {
  T value;
  stream >> value;
  return value;
}

bool RuuviEngine::setDataFromManufacturerData(const QString& name) {
  const auto data = d->m_tags[name]->manufacturerData();
  if (!data.contains(RuuviMID)) {
    setData(name, DataEngine::Data());
    return false;
  }

  QDataStream stream(data[RuuviMID]);
  stream.setByteOrder(QDataStream::BigEndian);

  stream.skipRawData(1);

  const qreal T = read_value<qint16>(stream) * .005;
  const qreal H = read_value<quint16>(stream) * .0025;
  const qreal P = (read_value<quint16>(stream) + 50000) * .01;

  DataEngine::Data values;
  values["temperature"] = T;
  values["humidity"] = H;
  values["pressure"] = P;

  qDebug() << "setData" << T << H << P;
  setData(name, values);

  d->m_updated[name] = true;
  if (allUpdated()) {
    qDebug() << "All updated";
    stopScanning();
  }

  return true;
}

bool RuuviEngine::sourceRequestEvent(const QString& name) {
  qDebug() << "Source request" << name;
  if (!d->m_tags.contains(name)) {
    setData(name, DataEngine::Data());
    d->m_tags[name] = nullptr;
  }
  auto p = d->m_manager->deviceForAddress(name);
  if (p != nullptr) {
    deviceAdded(p);
  }
  d->m_updated[name] = false;
  scan();
  return true;
}


bool RuuviEngine::updateSourceEvent(const QString& name) {
  qDebug() << "Update source" << name;
  d->m_updated[name] = false;
  if (!d->m_manager->usableAdapter()) { // BT is off
    return false;
  }
  scan();
  return false;
}

K_PLUGIN_CLASS_WITH_JSON(RuuviEngine, "plasma-dataengine-ruuvimonitor.json")

#include "ruuvimonitor.moc"


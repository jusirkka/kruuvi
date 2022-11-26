/* -*- coding: utf-8-unix -*-
 *
 * File: ./logreader/src/ruuvireader.cpp
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
#include "ruuvireader.h"
#include <QDebug>
#include <sys/socket.h>
#include <QSocketNotifier>
#include <unistd.h>
#include <QCoreApplication>
#include <BluezQt/InitManagerJob>
#include <BluezQt/GattCharacteristicRemote>
#include <QDataStream>
#include <QDateTime>
#include "measurementdatabase.h"
#include <QMap>
#include <QTimer>

using MeasurementMap = QMap<quint8, MeasurementVector>;
using MIterator = MeasurementMap::const_iterator;

struct RuuviReader::Private {
  BluezQt::Manager *m_manager = nullptr;
  QSocketNotifier* m_sig = nullptr;
  BluezQt::DevicePtr m_tag = nullptr;
  QStringList m_addresses;
  QTimer* m_deviceSearchTimer = nullptr;
  QTimer* m_errorTimer = nullptr;
  BluezQt::GattCharacteristicRemotePtr m_nus_tx = nullptr;
  BluezQt::GattCharacteristicRemotePtr m_nus_rx = nullptr;
  MeasurementMap m_measurements;
};

RuuviReader::RuuviReader(const QStringList& addresses, QObject *parent)
  : QObject(parent)
  , d(new Private) {

  if (::socketpair(AF_UNIX, SOCK_STREAM, 0, m_sigFd)) {
    qFatal("Couldn't create a socketpair");
  }

  d->m_addresses = addresses;

  d->m_deviceSearchTimer = new QTimer(this);
  d->m_deviceSearchTimer->setSingleShot(true);
  d->m_deviceSearchTimer->setInterval(StopScanMSecs);
  connect(d->m_deviceSearchTimer, &QTimer::timeout, [this] () {
    if (d->m_manager->usableAdapter() && d->m_manager->usableAdapter()->isDiscovering()) {
      qWarning() << d->m_addresses.first() << "Not found. Stop scan";
      d->m_manager->usableAdapter()->stopDiscovery();
    }
    d->m_addresses.pop_front();
    findDevice();
  });

  d->m_errorTimer = new QTimer(this);
  d->m_errorTimer->setSingleShot(true);
  d->m_errorTimer->setInterval(WaitBeforeErrorMSecs);
  connect(d->m_errorTimer, &QTimer::timeout, [this] () {
    qWarning() << "Timeout in" << WaitBeforeErrorMSecs / 1000 << "secs, quitting";
    cleanupAndExit();
  });

  d->m_sig = new QSocketNotifier(m_sigFd[1], QSocketNotifier::Read, this);
  connect(d->m_sig, &QSocketNotifier::activated, this, &RuuviReader::handleSig);

  d->m_manager = new BluezQt::Manager(this);

  connect(d->m_manager, &BluezQt::Manager::deviceAdded, this, &RuuviReader::deviceAdded);

  // Initialize BluezQt
  BluezQt::InitManagerJob* job = d->m_manager->init();

  job->start();
  connect(job, &BluezQt::InitManagerJob::result, this, [this] (BluezQt::InitManagerJob* job) {
    if (job->error()) {
      qWarning() << job->errorText();
      qFatal("Bluez manager init failed");
    }
    // qDebug() << "BluezQt initialized";
    emit initialized();
  });
}

void RuuviReader::sigHandler(int sig) {
  qInfo() << "received sig" << sig;
  const int a = sig;
  ::write(m_sigFd[0], &a, sizeof(a));
}

void RuuviReader::handleSig() {
  d->m_sig->setEnabled(false);
  int a;
  ::read(m_sigFd[1], &a, sizeof(a));

  // qDebug() << "handling sig" << a;

  cleanupAndExit();

  d->m_sig->setEnabled(true);
}

void RuuviReader::cleanupAndExit() {
  if (d->m_manager->usableAdapter() && d->m_manager->usableAdapter()->isDiscovering()) {
    // qInfo() << "Stop scan";
    d->m_manager->usableAdapter()->stopDiscovery();
  }

  if (d->m_tag != nullptr && d->m_tag->isConnected()) {
    // qInfo() << "Disconnect";
    d->m_tag->disconnectFromDevice();
  }

  qInfo() << "bye!";
  qApp->exit();
}

void RuuviReader::deviceAdded(BluezQt::DevicePtr p) {
  if (!d->m_deviceSearchTimer->isActive()) return;

  // qInfo() << "device added" << p->name() << p->address();
  if (!d->m_addresses.isEmpty() && p->address() == d->m_addresses.first()) {
    d->m_deviceSearchTimer->stop();
    // qInfo() << "Stop scanning";
    d->m_manager->usableAdapter()->stopDiscovery();
    connectDevice(p);
  }
}

template<typename T> T read_value(QDataStream& stream) {
  T value;
  stream >> value;
  return value;
}


void RuuviReader::setupScan() {
  if (!d->m_manager->usableAdapter()) {
    qWarning() << "No usable adapter exists";
    cleanupAndExit();
    return;
  }

  // qDebug() << "Setup scan filter";

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


void RuuviReader::scan() {
  if (!d->m_manager->usableAdapter()) {
    qWarning() << "No usable adapter exists";
    cleanupAndExit();
    return;
  }
  setupScan();
  // qInfo() << "Start scan";
  d->m_manager->usableAdapter()->startDiscovery();
}

void RuuviReader::findDevice() {
  if (d->m_addresses.isEmpty()) {
    cleanupAndExit();
    return;
  }
  const auto addr = d->m_addresses.first();
  auto p = d->m_manager->deviceForAddress(addr);
  if (p == nullptr) {
    qInfo() << addr << "not known, scanning ..";
    d->m_deviceSearchTimer->start();
    scan();
    return;
  }
  connectDevice(p);
}

void RuuviReader::connectDevice(BluezQt::DevicePtr p) {
  qInfo() << "connecting to" << p->address();
  d->m_tag = p;

  d->m_errorTimer->start();
  auto call = d->m_tag->connectToDevice();
  qInfo() << "connection to" << d->m_tag->address() << "in progress ...";
  connect(call, &BluezQt::PendingCall::finished, [this] (const BluezQt::PendingCall* rsp) {
    d->m_errorTimer->stop();
    if (rsp->error()) {
      qWarning() << "Error connecting:" << rsp->errorText();
      cleanupAndExit();
      return;
    }
    qInfo() << "connected to" << d->m_tag->address();
    d->m_errorTimer->start();
    for (const auto srv: d->m_tag->gattServices()) {
      setupNUS(srv);
    }
    connect(d->m_tag.data(), &BluezQt::Device::gattServiceAdded, this, &RuuviReader::setupNUS);
    connect(d->m_tag.data(), &BluezQt::Device::gattServiceChanged, this, &RuuviReader::setupNUS);
  });
}

void RuuviReader::setupNUS(BluezQt::GattServiceRemotePtr srv) {
  if (d->m_nus_rx != nullptr && d->m_nus_tx != nullptr) {
    return;
  }
  // qInfo() << "service uuid" << srv->uuid();
  if (srv->uuid().toUpper() == NUSUUID) {
    // qInfo() << "NUS found";
    for (const auto ch: srv->characteristics()) {
      if (ch->uuid().toUpper() == NUSUUID_RX && d->m_nus_rx == nullptr) {
        qInfo() << "NUS_RX found";
        d->m_nus_rx = ch;
        connect(d->m_nus_rx.data(), &BluezQt::GattCharacteristicRemote::valueChanged,
                this, &RuuviReader::handleRXNotify);
      } else if (ch->uuid().toUpper() == NUSUUID_TX && d->m_nus_tx == nullptr) {
        qInfo() << "NUS_TX found";
        d->m_nus_tx = ch;
      }
    }
  }
  if (d->m_nus_rx != nullptr && d->m_nus_tx != nullptr) {
    d->m_errorTimer->stop();
    emit deviceFound();
  }
}

void RuuviReader::readLog() {
  if (d->m_nus_tx == nullptr || d->m_nus_rx == nullptr) {
    qWarning() << "NUS not available, cannot read log";
    cleanupAndExit();
    return;
  }
  qInfo() << "Start notify";
  auto rsp = d->m_nus_rx->startNotify();
  rsp->waitForFinished();
  if (rsp->error()) {
    qWarning() << "RX start notify failed:" << rsp->errorText();
    cleanupAndExit();
    return;
  }

  // Get the last timestamp
  MeasurementDatabase db("RuuviReader::readlog");
  const auto ts = db.timestamp(db.locationId(d->m_addresses.first()), "temperature");

  QByteArray bytes;
  QDataStream stream(&bytes, QIODevice::WriteOnly);
  stream.setByteOrder(QDataStream::BigEndian);
  const char header[3] = {addr_env, addr_env, op_req};
  stream.writeRawData(header, 3);
  const quint32 now = static_cast<quint32>(QDateTime::currentSecsSinceEpoch());
  const quint32 then = std::max(0, static_cast<int>(ts) - 3600); // one hour offset for safety
  stream << now;
  stream << then;

  // qDebug() << bytes;
  auto tx_rsp = d->m_nus_tx->writeValue(bytes, QVariantMap());
  tx_rsp->waitForFinished();
  if (tx_rsp->error()) {
    qWarning() << "Error when writing:" << tx_rsp->errorText();
    cleanupAndExit();
  }
}

void RuuviReader::handleRXNotify(const QByteArray value) {
  // qDebug() << value;
  QDataStream stream(value);
  stream.setByteOrder(QDataStream::BigEndian);

  auto dst = read_value<quint8>(stream);
  if (dst != addr_env) return;

  auto src = read_value<quint8>(stream);

  auto op = read_value<quint8>(stream);
  if (op != op_rsp) return;

  auto ts = read_value<quint32>(stream);

  if (src == addr_env && ts == UINT32_MAX) {
    qInfo() << "Finished reading log from" << d->m_tag->address();
    auto rsp = d->m_nus_rx->stopNotify();
    rsp->waitForFinished();
    if (rsp->error()) {
      qWarning() << "RX stop notify failed:" << rsp->errorText();
    }
    updateDB();

    d->m_addresses.pop_front();
    if (d->m_addresses.isEmpty()) {
      cleanupAndExit();
    } else {
      disconnectDevice();
      findDevice();
    }
    return;
  }

  const float val = .01 * read_value<qint32>(stream);
  d->m_measurements[src] << Measurement(ts, val);

  //  qInfo() << QDateTime::fromSecsSinceEpoch(ts);
  //  if (src == addr_temperature) {
  //    qInfo() << "Temperature:" << val << "°C";
  //  } else if (src == addr_humidity) {
  //    qInfo() << "Humidity:" << val << "%";
  //  } else if (src == addr_pressure) {
  //    qInfo() << "Pressure:" << val << "hPa";
  //  } else {
  //    qWarning() << "Unsupported measurement source" << src;
  //  }
}

void RuuviReader::disconnectDevice() {
  qInfo() << "Disconnect";
  auto call = d->m_tag->disconnectFromDevice();
  call->waitForFinished();
  d->m_tag = nullptr;
  d->m_nus_rx = nullptr;
  d->m_nus_tx = nullptr;
  d->m_measurements.clear();
}

RuuviReader::~RuuviReader() {
  delete d;
}

void RuuviReader::updateDB() {
  qInfo() << "Update DB";
  MeasurementDatabase db("RuuviReader::updateDB");
  const auto addr = d->m_addresses.first();
  const auto locId = db.locationId(addr);

  for (auto it = d->m_measurements.cbegin(); it != d->m_measurements.cend(); ++it) {
    const auto mid = it.key();

    auto values = it.value();
    std::sort(values.begin(), values.end(), [] (const Measurement& a, const Measurement& b) {
      return a.ts < b.ts;
    });

    // qDebug() << "Considering" << values.size() << "measurements to" << addr << tables[mid];
    const auto ts = db.timestamp(locId, tables[mid]);
    while (!values.isEmpty() && values.first().ts < ts) {
      values.pop_front();
    }
    // qDebug() << "Inserting" << values.size() << "measurements to" << addr << tables[mid];
    db.insertMeasurements(locId, tables[mid], values);
  }
}

/* -*- coding: utf-8-unix -*-
 *
 * File: ./logreader/src/main.cpp
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
#include <QCoreApplication>
#include <QDebug>
#include "ruuvireader.h"
#include <signal.h>
#include "measurementdatabase.h"

static int setup_unix_signal_handlers() {

  const int sigs[3] = {SIGHUP, SIGTERM, SIGINT};
  for (int i = 0; i < 3; ++i) {
    struct sigaction a;
    a.sa_handler = RuuviReader::sigHandler;
    sigemptyset(&a.sa_mask);
    a.sa_flags = 0;
    a.sa_flags |= SA_RESTART;
    if (sigaction(sigs[i], &a, 0) != 0) return sigs[i];
  }

  return 0;
}

int main(int argc, char *argv[]) {
  QCoreApplication app(argc, argv);

  auto ret = setup_unix_signal_handlers();
  if (ret > 0) {
    return ret;
  }

  try {
    MeasurementDatabase::createTables();
  } catch (const PlatformError& e) {
    qInfo() << e.msg();
    return 255;
  }

  auto addrs = QCoreApplication::arguments();
  addrs.pop_front();

  auto reader = new RuuviReader(addrs);

  QObject::connect(reader, &RuuviReader::initialized, reader, &RuuviReader::findDevice);
  QObject::connect(reader, &RuuviReader::deviceFound, reader, &RuuviReader::readLog);

  return app.exec();
}

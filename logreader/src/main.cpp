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
#include <QCommandLineParser>
#include <QFileInfo>
#include <QDir>
#include <QMessageLogContext>
#include <QDateTime>
#include <QMap>

class Logger {
public:

  static Logger* instance() {
    static Logger* logger = new Logger;
    return logger;
  }

  Logger() = default;

  QFile* dst() {return m_dest;}

  bool init(const QString& path) {
    if (path.isEmpty()) return false;

    const QFileInfo info(path);
    QDir dir("/");
    if (!dir.mkpath(info.absolutePath())) {
      qWarning() << "Cannot create" << info.absolutePath();
      return false;
    }

    m_dest = new QFile(path);
    if (!m_dest->open(QFile::WriteOnly | QFile::Append)) {
      qWarning() << "Cannot open" << path;
      return false;
    }
    return true;
  }

  static void handler(QtMsgType mt, const QMessageLogContext& ctx, const QString& msg) {
    const QString formattedTime = QDateTime::currentDateTime().toString("MMM dd hh:mm:ss.zzz");
    QString txt = QString("[%1] %2: %3 (%4)")
        .arg(levelNames[mt])
        .arg(formattedTime)
        .arg(msg)
        .arg(ctx.function);
    QTextStream stream(instance()->dst());
    stream << txt << "\n";
  }

private:

  static const inline QMap<QtMsgType, QString> levelNames = {
    {QtDebugMsg, "D"},
    {QtInfoMsg, "I"},
    {QtWarningMsg, "W"},
    {QtCriticalMsg, "C"},
    {QtFatalMsg, "F"}
  };

  QFile* m_dest = nullptr;
};


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

  QCommandLineParser parser;
  parser.setApplicationDescription("Read the RuuviTag measurement storage to a database");
  parser.addOption({{"l", "logfile"}, "Append log messages to <file>.", "file"});
  parser.addHelpOption();
  parser.addPositionalArgument("ruuvitags", "Bluetooth addresses of the RuuviTag devices");
  parser.process(app);

  const auto logfile = parser.value("logfile");
  if (!logfile.isEmpty()) {
    if (Logger::instance()->init(logfile)) {
      qInstallMessageHandler(Logger::handler);
    } else {
      return 1;
    }
  }

  auto ret = setup_unix_signal_handlers();
  if (ret > 0) {
    return ret;
  }

  try {
    MeasurementDatabase::createTables();
  } catch (const PlatformError& e) {
    qWarning() << e.msg();
    return 255;
  }

  auto reader = new RuuviReader(parser.positionalArguments());

  QObject::connect(reader, &RuuviReader::initialized, reader, &RuuviReader::findDevice);
  QObject::connect(reader, &RuuviReader::deviceFound, reader, &RuuviReader::readLog);

  return app.exec();
}

/*
  Wlan-navi project.
  Copyright (C) 2011  Wlan-navi team.

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef SETTINGS_H
#define SETTINGS_H

#include <QMainWindow>

#include "config.h"

namespace Ui {
    class Settings;
}

class Settings : public QMainWindow
{
    Q_OBJECT

public:
    explicit Settings(QWidget *parent = 0);
    ~Settings();
    void showSettingsWindow();

signals:
    void wlanTimerUpdated();

private slots:
    void saveWlanTimer();
    void loadWlanTimer();
    void saveQGeoProvider();
    void loadQGeoProvider();
    void loadItemsCB();
    void saveSettings();

private:
    Ui::Settings *ui;
};

#endif // SETTINGS_H

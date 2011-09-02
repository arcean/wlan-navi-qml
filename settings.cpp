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

#include "settings.h"
#include "ui_settings.h"
#include <QDebug>

#ifdef Q_WS_MAEMO_5
    #include <QMaemo5InformationBox>
#endif

Settings::Settings(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Settings)
{
    ui->setupUi(this);
    setWindowTitle("Settings");
    //setAttribute(Qt::WA_Maemo5StackedWindow);
    connect(ui->saveButton, SIGNAL(clicked()), this, SLOT(saveSettings()));
    loadItemsCB();
}

Settings::~Settings()
{
    delete ui;
}

/**
  Shows this window
*/
void Settings::showSettingsWindow()
{
    loadQGeoProvider();
    loadWlanTimer();

    this->show();
}

/**
  Adds items to the comboboxes
*/
void Settings::loadItemsCB()
{
    ui->mapProviderCB->addItem("OpenStreetMap", "openstreetmap");
    ui->mapProviderCB->addItem("Nokia OVI Maps", "nokia");
    ui->scanInterval_CB->addItem("10", "10");
    ui->scanInterval_CB->addItem("30", "30");
    ui->scanInterval_CB->addItem("60", "60");
    ui->scanInterval_CB->addItem("180", "180");
    ui->scanInterval_CB->addItem("300", "300");
    ui->scanInterval_CB->addItem("600", "600");
}


/**
  Checks what plugin should be set as default
*/
void Settings::loadQGeoProvider()
{
    QString plugin = "";

    GetKeyString(&plugin, "qgeoplugin");
    if(!plugin.compare("nokia"))
    {
        int index = ui->mapProviderCB->findData("nokia");
        ui->mapProviderCB->setCurrentIndex(index);
    }
    else
    {
        int index = ui->mapProviderCB->findData("openstreetmap");
        ui->mapProviderCB->setCurrentIndex(index);
    }
}

/**
  Saves default QGeoServiceProvider plugin
*/
void Settings::saveQGeoProvider()
{
    QString plugin = ui->mapProviderCB->itemData(ui->mapProviderCB->currentIndex()).toString();
    if(!plugin.compare(""))
        StoreKeyString("openstreetmap", "qgeoplugin");
    else
        StoreKeyString(plugin, "qgeoplugin");
}

/**
  Checks what value shoud be set as default
*/
void Settings::loadWlanTimer()
{
    int value = 60;

    GetKeyInt(&value, "wlanTimer");
    if(value > 9)
    {
        int index = ui->scanInterval_CB->findData(value);
        ui->scanInterval_CB->setCurrentIndex(index);
    }
    else
    {
        int index = ui->scanInterval_CB->findData(60);
        ui->scanInterval_CB->setCurrentIndex(index);
    }
}

/**
  Saves wlanTimer settings
*/
void Settings::saveWlanTimer()
{
    QString value_str = ui->scanInterval_CB->currentText();
    int value = value_str.toInt();
    if(value > 9)
        StoreKeyInt(value, "wlanTimer");
    else
        StoreKeyInt(60, "wlanTimer");

    emit this->wlanTimerUpdated();
}

void Settings::saveSettings()
{
    saveQGeoProvider();
    saveWlanTimer();
    #ifdef Q_WS_MAEMO_5
        QMaemo5InformationBox::information(this, "Settings saved");
    #endif
}

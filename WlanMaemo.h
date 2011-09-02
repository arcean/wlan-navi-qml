#ifndef WLANMAEMO_H
#define WLANMAEMO_H

/*
  Wlan-navi project.
  Copyright (C) 2011  Tomasz Pieniążek & the Wlan-navi team.

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

#include <QObject>
#include <QList>
#include <QString>
#include <dbus/dbus.h>
#include <iostream>
#include <list>
#include "marker.h"

enum Type {
  typeNone,            //! Not connected
  typeAuto,            //! Automatic
  typeAdHoc,           //! Ad hoc network between individual cells
  typeInfrastructure,  //! Multi cell network
  typeMaster,          //! Synchronisation master or AP
  typeRepeater,        //! Repeater
  typeSecond,          //! Second master/ repeater (backup)
  typeMonitor,         //! Passive monitor
  typeMesh             //! Mesh network
};

class Network
{
public:
  QString       id;
  std::string   essid;
  Type          type;
  unsigned      bitrate;
  unsigned long encryption;
  int           channel;
  int           quality;
  Marker        *marker;

  Network()
  {
    Clear();
  }

  void Clear()
  {
    type      =  typeNone;
    bitrate   =  0;
    encryption=  0;
    quality   = -1;
    channel   = -1;
  }

  void SetID(QString id)
  {
      this->id = id;
  }
};

class WlanMaemo : public QObject
{
    Q_OBJECT
private:
  DBusConnection* GetDBusConnection();
  QList<Network> wlansWI;

signals:
    void wlansUpdated();

public:
  enum PowerSaving {
    powerSavingOn,
    powerSavingOff,
    powerSavingUnknown
  };

  enum Encryption {
    cryptNone    = 1 << 0,
    cryptWEP     = 1 << 1,
    cryptWPA_PSK = 1 << 2,
    cryptWPA_EAP = 1 << 3,
    cryptWPA2    = 1 << 4
  };

  WlanMaemo();

  std::string GetDefaultInterface() const;
  bool SupportsNetworkRetrieval() const;
  bool UpdateNetworks();
  bool CanChangePowerSaving() const;
  bool SetPowerSaving(bool savePower);
  bool HandleMessage(DBusConnection *connection, DBusMessage *msg);
  int GetQualityFromSignalNoiseDbm(int signal, int noise);
  QList<Network> getWlans();
  void clearList();

  std::string        defaultInterface;
  std::string        typeName;
  Type               type;
  std::string        essid;
  unsigned           bitrate;
  int                channel;
  int                quality; // [0..100%] ([101..201%] - saved records)
  int                signal;  // dBm
  int                noise;   // dBm
  std::string        accesspoint;
  std::string        ip;
  std::string        mac;
  PowerSaving        powerSaving;
  std::list<Network> networks;
};

  QString encryptionToString(unsigned long encryptionType);


#endif // WLANMAEMO_H

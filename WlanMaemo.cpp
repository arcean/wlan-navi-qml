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

#include "WlanMaemo.h"
#include <string.h>
#include <QDebug>

DBusHandlerResult DBusMsgHandler(DBusConnection *connection, DBusMessage *msg, void *data)
{
  WlanMaemo *statusHildon = static_cast<WlanMaemo*>(data);

  statusHildon->HandleMessage(connection, msg);

  return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

void callback(DBusPendingCall* call, void* /*data*/)
{
  DBusMessage *reply;

  reply = dbus_pending_call_steal_reply(call);

  std::cout << "Call finished with reply of type " << dbus_message_type_to_string(dbus_message_get_type(reply)) << std::endl;

  if (dbus_message_get_type(reply) == DBUS_MESSAGE_TYPE_ERROR) {
    std::cout << "Error: " << dbus_message_get_error_name(reply) << std::endl;
  }

  dbus_message_unref(reply);
}

QList<Network> WlanMaemo::getWlans()
{
    return wlansWI;
}

/**
  Class constructor
*/
WlanMaemo::WlanMaemo()
{
  if (GetDBusConnection() == NULL) {
    std::cerr << "No DBus connection!" << std::endl;
    return;
  }

  dbus_connection_add_filter(GetDBusConnection(), DBusMsgHandler, this, NULL);
}

std::string WlanMaemo::GetDefaultInterface() const
{
    /* We have only one wlan device */
    return "wlan0";
}

/**
  Computes signal quality form signal strenght and noise.
*/
int WlanMaemo::GetQualityFromSignalNoiseDbm(int signal, int noise)
{
  int quality = signal - noise;

  if (quality < 0) {
    return 0;
  }
  if (quality >= 0 && quality < 40) {
    return 5 * quality / 2;
  }
  else {
    return 100;
  }
}

DBusConnection* WlanMaemo::GetDBusConnection()
{
    DBusConnection *bus = NULL;
    DBusError error;

    dbus_error_init(&error);

    std::cout << "Connecting to System D-Bus" << std::endl;

    bus = dbus_bus_get(DBUS_BUS_SESSION, &error);

    if(bus == NULL) {
        std::cout << "Failed to open System bus" << std::endl;
    }

    return bus;
}

/**
  Handles received messages.
*/
bool WlanMaemo::HandleMessage(DBusConnection *connection, DBusMessage *msg)
{
    std::cout << "Handle Message!" << std::endl;
  if (dbus_message_get_type(msg) == DBUS_MESSAGE_TYPE_METHOD_CALL) {
    DBusMessage *response;
    std::string appName;
    std::string error;

    response = dbus_message_new_error(msg, "Message not implemented","Message not implemented");
    dbus_connection_send(connection, response, NULL);
    dbus_message_unref(response);
    std::cout << "Message not implemented! return true" << std::endl;
    return true;
  }
  else if (dbus_message_get_type(msg) == DBUS_MESSAGE_TYPE_SIGNAL) {
    if (strcmp(dbus_message_get_interface(msg), "com.nokia.wlancond.signal") != 0 ||
        strcmp(dbus_message_get_path(msg), "/com/nokia/wlancond/signal") != 0 ||
        strcmp(dbus_message_get_member(msg), "scan_results") != 0) {
        std::cout << "ret false!" << std::endl;
      return false;
    }
  }
  else {
      std::cout << "return false!" << std::endl;
    return false;
  }

  std::cout << "Handling network scan result..." << std::endl;

  networks.clear();

  DBusMessageIter iter;
  int             type;
  dbus_int32_t    entries;
  dbus_int32_t    intValue;
  dbus_uint32_t   uintValue;
  std::string     strValue;

  dbus_message_iter_init(msg, &iter);

  type = dbus_message_iter_get_arg_type(&iter);
  if (type != DBUS_TYPE_INT32) {
    std::cerr << "Expected number of entries" << std::endl;
    return true;
  }

  dbus_message_iter_get_basic(&iter, &entries);
  dbus_message_iter_next(&iter);

  for (int i = 0; i<entries; i++) {
    Network         network;
    DBusMessageIter iter2;

    //
    // ESSID
    //

    type = dbus_message_iter_get_arg_type(&iter);
    if (type != DBUS_TYPE_ARRAY) {
      std::cerr << "Expected ESSID" << std::endl;
      return true;
    }

    dbus_message_iter_recurse(&iter, &iter2);
    strValue.clear();

    while ((type = dbus_message_iter_get_arg_type(&iter2)) != DBUS_TYPE_INVALID) {
      std::string tmp;

      if (type != DBUS_TYPE_BYTE) {
        std::cerr << "Wrong type for ESSID" << std::endl;
        return true;
      }

      char value;
      dbus_message_iter_get_basic(&iter2, &value);

      strValue.append(1, value);

      dbus_message_iter_next(&iter2);
    }

    if (strValue.length() > 0 && strValue[strValue.length()-1] == '\0') {
      strValue.erase(strValue.length()-1);
    }

    network.essid = strValue;

    dbus_message_iter_next(&iter);


    type = dbus_message_iter_get_arg_type(&iter);
    if (type != DBUS_TYPE_ARRAY) {
      std::cerr << "Expected BSSID" << std::endl;
      return true;
    }
    dbus_message_iter_next(&iter);

    type = dbus_message_iter_get_arg_type(&iter);
    if (type != DBUS_TYPE_INT32) {
      std::cerr << "Expected RSSI" << std::endl;
      return true;
    }
    dbus_message_iter_get_basic(&iter, &intValue);

    if (noise < 0) {
      std::cerr << "Do not have global noise value, quitting" << std::endl;
      return true;
    }

    if (noise == 0)  {
      network.quality = GetQualityFromSignalNoiseDbm(intValue, -95);
    }
    else {
      network.quality = GetQualityFromSignalNoiseDbm(intValue, noise - 0x100);
    }

    dbus_message_iter_next(&iter);

    type = dbus_message_iter_get_arg_type(&iter);
    if (type != DBUS_TYPE_UINT32) {
      std::cerr << "Expected CHANNEL" << std::endl;
      return true;
    }
    dbus_message_iter_get_basic(&iter, &uintValue);

    network.channel = uintValue;

    dbus_message_iter_next(&iter);

    type = dbus_message_iter_get_arg_type(&iter);
    if (type != DBUS_TYPE_UINT32) {
      std::cerr << "Expected CAPABILITIES" << std::endl;
      return true;
    }
    dbus_message_iter_get_basic(&iter, &uintValue);
    if (uintValue & (1 << 0)) {
      network.type = typeInfrastructure;
    }
    else if (uintValue & (1 << 1)) {
      network.type = typeAdHoc;
    }
    else if (uintValue & (1 << 2)) {
      network.type = typeAuto;
    }

    dbus_message_iter_next(&iter);

    network.encryption = 0;

    if (uintValue & (1<<4)) {
      network.encryption |= cryptNone;
    }
    if (uintValue & (1<<5)) {
      network.encryption |= cryptWEP;
    }
    if (uintValue & (1<<6)) {
      network.encryption |= cryptWPA_PSK;
    }
    if (uintValue & (1<<7)) {
      network.encryption |= cryptWPA_EAP;
    }
    if (uintValue & (1<<8)) {
      network.encryption |= cryptWPA2;
    }

    if (!network.essid.empty()) {
      networks.push_back(network);
    }

    qDebug() << "\n" << network.bitrate << " " << network.channel << " " << network.encryption << " " << QString::fromStdString(network.essid) << " " << network.encryption << " " << network.quality<< "\n";

    network.id = "";
    wlansWI.append(network);

    //Emit signal wlansUpdated(), so the mainwindow will add new markers on the geomap.
    emit this->wlansUpdated();
  }

  return true;
}

bool WlanMaemo::SupportsNetworkRetrieval() const
{
    return true;
}

bool WlanMaemo::UpdateNetworks()
{
  DBusMessage     *msg;
  DBusPendingCall *call;
  dbus_int32_t    power = 4;
  dbus_int32_t    flags = 2;
  unsigned char   a[] = { 0 };
  unsigned char*  ap = a;

  std::cout << "Requesting network scan..." << std::endl;

  if (GetDBusConnection() == NULL) {
    std::cerr << "No DBus connection!" << std::endl;
    return false;
  }

  msg=dbus_message_new_method_call("com.nokia.wlancond",
                                   "/com/nokia/wlancond/request",
                                   "com.nokia.wlancond.request",
                                   "scan");

  dbus_message_append_args(msg,
                           DBUS_TYPE_INT32, &power,
                           DBUS_TYPE_ARRAY, DBUS_TYPE_BYTE, &ap,0,
                           DBUS_TYPE_UINT32, &flags,
                           DBUS_TYPE_INVALID);

  if (!dbus_connection_send_with_reply(GetDBusConnection(), msg, &call,-1)) {
    std::cout << "Cannot send with reply" << std::endl;
    return false;
  }

  dbus_pending_call_set_notify(call, callback, NULL, NULL);

  std::cout << "Requesting network scan done." << std::endl;

  dbus_message_unref(msg);

  return true;
}

bool WlanMaemo::CanChangePowerSaving() const
{
    return true;
}

bool WlanMaemo::SetPowerSaving(bool savePower)
{
    std::cout << "Requesting power saving switch... " << savePower << std::endl;

    DBusMessage     *msg;
    DBusPendingCall *call;
    dbus_bool_t     powerModeActivated = savePower ? TRUE : FALSE;
    bool            result;

    if (GetDBusConnection() == NULL) {
        std::cerr << "No DBus connection!" << std::endl;
        return false;
    }

    msg=dbus_message_new_method_call("com.nokia.wlancond",
                                     "/com/nokia/wlancond/request",
                                     "com.nokia.wlancond.request",
                                     "set_powersave");

    dbus_message_append_args(msg,
                             DBUS_TYPE_BOOLEAN, &powerModeActivated,
                             DBUS_TYPE_INVALID);

    result=dbus_connection_send_with_reply(GetDBusConnection(), msg, &call, -1);

    dbus_pending_call_set_notify(call, callback, NULL, NULL);

    dbus_message_unref(msg);

    return result;
}

void WlanMaemo::clearList()
{
    wlansWI.clear();
}

/**
  Translates id Encryption type into string.
*/
QString encryptionToString(unsigned long encryptionType)
{
    QString name;

    switch (encryptionType) {
        case 0:
            name = "None";
            break;
        case 1:
            name = "WEP";
            break;
        case 2:
            name = "WPA PSK";
            break;
        case 3:
            name = "WPA EAP";
            break;
        case 4:
            name = "WPA2";
            break;
        default:
            name = "Unknown";
            break;
    }
    return name;
}

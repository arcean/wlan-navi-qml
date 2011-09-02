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

#include "config.h"

/**
  Save value via GConf
*/
void StoreKeyString(QString value, QString key)
{
    GConfClient* gconfClient = gconf_client_get_default();
    g_assert(GCONF_IS_CLIENT(gconfClient));

    if(!gconf_client_set_string(gconfClient, QString(GCONF_DIR + key).toAscii(), value.toAscii(), NULL)) {
      g_warning(" Failed to store setting!\n");
    }

    g_object_unref(gconfClient);
}

void StoreKeyBool(bool value, QString key)
{
    GConfClient* gconfClient = gconf_client_get_default();
    g_assert(GCONF_IS_CLIENT(gconfClient));

    if(!gconf_client_set_bool(gconfClient, QString(GCONF_DIR + key).toAscii(), value, NULL)) {
      g_warning(" Failed to store setting!\n");
    }

    g_object_unref(gconfClient);
}

void StoreKeyInt(int value, QString key)
{
    GConfClient* gconfClient = gconf_client_get_default();
    g_assert(GCONF_IS_CLIENT(gconfClient));

    if(!gconf_client_set_int(gconfClient, QString(GCONF_DIR + key).toAscii(), value, NULL)) {
      g_warning(" Failed to store setting!\n");
    }

    g_object_unref(gconfClient);
}

/**
  Load value via GConf
*/
void GetKeyString(QString *value, QString key)
{
    GConfClient* gconfClient = gconf_client_get_default();

    g_assert(GCONF_IS_CLIENT(gconfClient));

    GConfValue* gcValue = NULL;
    gcValue = gconf_client_get_without_default(gconfClient, QString(GCONF_DIR + key).toAscii(), NULL);

    if(gcValue == NULL) {
      g_warning(" key not found\n");
      g_object_unref(gconfClient);
      return;
    }

    if(gcValue->type == GCONF_VALUE_STRING) {
      *value = gconf_value_get_string(gcValue);
    }
    else {
      g_warning(" key is not string\n");
    }

    gconf_value_free(gcValue);
    g_object_unref(gconfClient);
}

void GetKeyBool(bool *value, QString key)
{
    GConfClient* gconfClient = gconf_client_get_default();

    g_assert(GCONF_IS_CLIENT(gconfClient));

    GConfValue* gcValue = NULL;
    gcValue = gconf_client_get_without_default(gconfClient, QString(GCONF_DIR + key).toAscii(), NULL);

    if(gcValue == NULL) {
      g_warning(" key not found\n");
      g_object_unref(gconfClient);
      return;
    }

    if(gcValue->type == GCONF_VALUE_BOOL) {
      *value = gconf_value_get_bool(gcValue);
    }
    else {
      g_warning(" key is not bool\n");
    }

    gconf_value_free(gcValue);
    g_object_unref(gconfClient);
}

void GetKeyInt(int *value, QString key)
{
    GConfClient* gconfClient = gconf_client_get_default();

    g_assert(GCONF_IS_CLIENT(gconfClient));

    GConfValue* gcValue = NULL;
    gcValue = gconf_client_get_without_default(gconfClient, QString(GCONF_DIR + key).toAscii(), NULL);

    if(gcValue == NULL) {
      g_warning(" key not found\n");
      g_object_unref(gconfClient);
      return;
    }

    if(gcValue->type == GCONF_VALUE_INT) {
      *value = gconf_value_get_int(gcValue);
    }
    else {
      g_warning(" key is not int\n");
    }

    gconf_value_free(gcValue);
    g_object_unref(gconfClient);
}

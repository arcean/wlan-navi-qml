/*
  Wlan-navi project.
  Copyright (C) 2011  Wlan-navi team.

  Backend for GConf. It's used to store & load settings.

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

#ifndef CONFIG_H
#define CONFIG_H

#include <QString>
#include <gconf/gconf-client.h>

#define GCONF_DIR "/apps/Maemo/wlan-navi/"

//For string
void StoreKeyString(QString value, QString key);
void GetKeyString(QString *value, QString key);
//For bool
void StoreKeyBool(bool value, QString key);
void GetKeyBool(bool *value, QString key);
//For int
void StoreKeyInt(int value, QString key);
void GetKeyInt(int *value, QString key);

#endif // CONFIG_H

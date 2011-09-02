/*
  Wlan-navi project.
  Copyright (C) 2011  Genio & the Wlan-navi team.

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

#include "marker.h"

#include "qgeomappixmapobject.h"
#include "qgeosearchmanager.h"
#include "qgeocoordinate.h"
#include "qgraphicsgeomap.h"
#include "qgeoaddress.h"
#include "qgeosearchreply.h"

class MarkerPrivate
{
public:
    Marker::MarkerType type;
    QString name;
    QGeoAddress address;
};

Marker::Marker(MarkerType type) :
    QGeoMapPixmapObject(),
    d(new MarkerPrivate)
{
    setMarkerType(type);
}

void Marker::setMarkerType(MarkerType type)
{
    QString filename;
    QPoint offset;
    int scale;
    d->type = type;

    switch (d->type) {
        case MyLocationMarker:
            filename = ":/icons/mylocation.png";
            break;
        case OpenWlanMarker:
            filename = ":/icons/openwlanmarker.png";
            break;
    }

    if (d->type == MyLocationMarker) {
        offset = QPoint(-13,-13);
        scale = 25;
    } else {
        offset = QPoint(-15, -36);
        scale = 30;
    }

    setOffset(offset);
    setPixmap(QPixmap(filename).scaledToWidth(scale, Qt::SmoothTransformation));

}

void Marker::setAddress(QGeoAddress addr)
{
    if (d->address != addr) {
        d->address = addr;
        emit addressChanged(d->address);
    }
}

Marker::MarkerType Marker::getMarkerType() const
{
    return d->type;
}

QString Marker::getName() const
{
    return d->name;
}

QGeoAddress Marker::getAddress() const
{
    return d->address;
}

/**
  EVERY marker should have a name. Marker's name = WLAN's essid.
*/
void Marker::setName(QString name)
{
    if (d->name != name) {
        d->name = name;
        emit nameChanged(d->name);
    }
}

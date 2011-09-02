#ifndef MARKER_H
#define MARKER_H

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

#include <QSignalMapper>

#include "qgeomappixmapobject.h"
#include "qgeosearchmanager.h"
#include "qgeocoordinate.h"
#include "qgraphicsgeomap.h"
#include "qgeoaddress.h"
#include "qgeosearchreply.h"

#include "qlandmark.h"
#include "qgeoboundingcircle.h"

//Our radius for location:
#define radius 0.001

using namespace QtMobility;

class MarkerPrivate;
class Marker : public QGeoMapPixmapObject
{
    Q_OBJECT

    Q_PROPERTY(MarkerType markerType READ getMarkerType WRITE setMarkerType NOTIFY markerTypeChanged)
    Q_PROPERTY(QString name READ getName WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QGeoAddress address READ getAddress WRITE setAddress NOTIFY addressChanged)
public:
    enum MarkerType {
        MyLocationMarker,
        OpenWlanMarker
    };

    explicit Marker(MarkerType type);

    MarkerType getMarkerType() const;
    void setMarkerType(MarkerType type);

    QString getName() const;
    QGeoAddress getAddress() const;

public slots:
    void setName(QString name);
    void setAddress(QGeoAddress addr);

signals:
    void markerTypeChanged(const MarkerType &type);
    void nameChanged(const QString &name);
    void addressChanged(const QGeoAddress &address);

private:
    MarkerPrivate *d;
};

#endif // MARKER_H

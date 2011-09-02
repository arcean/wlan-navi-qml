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

#include "mapswidget.h"

#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneWheelEvent>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QTimer>
#include <QDebug>
#include <QPixmap>

#include "qgeocoordinate.h"

GeoMap::GeoMap(QGeoMappingManager *manager, MapsWidget *mapsWidget) :
    QGraphicsGeoMap(manager), mapsWidget(mapsWidget)
{
    this->setFocus();
    connect(this, SIGNAL(mapTouched()), mapsWidget, SLOT(showFullscreenButton()));
}

GeoMap::~GeoMap()
{
}

double GeoMap::centerLatitude() const
{
    return center().latitude();
}

double GeoMap::centerLongitude() const
{
    return center().longitude();
}

void GeoMap::setCenterLatitude(double lat)
{
    QGeoCoordinate c = center();
    c.setLatitude(lat);
    setCenter(c);
}

void GeoMap::setCenterLongitude(double lon)
{
    QGeoCoordinate c = center();
    c.setLongitude(lon);
    setCenter(c);
}

void GeoMap::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    panActive = true;

    markerPressed = false;
    QPointF point = event->pos();
    qreal x = point.x()-5;
    qreal y = point.y()-5;
    qreal width = 10;
    qreal height = 10;
    QRect rect(x, y, width, height);
    //QList<QGeoMapObject*> objects = mapObjectsAtScreenPosition(event->pos());
    QList<QGeoMapObject*> objects = this->mapObjectsInScreenRect(rect);
    if (objects.size() > 0) {
        pressed = objects.first();
        markerPressed = true;
    }

    this->setFocus();
    event->accept();
}

void GeoMap::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    panActive = false;

    if (markerPressed) {
        // check if we're still over the object
        QPointF point = event->pos();
        qreal x = point.x()-5;
        qreal y = point.y()-5;
        qreal width = 10;
        qreal height = 10;
        QRect rect(x, y, width, height);
        //QList<QGeoMapObject*> objects = mapObjectsAtScreenPosition(event->pos());
        QList<QGeoMapObject*> objects = this->mapObjectsInScreenRect(rect);
        if (objects.contains(pressed)) {
            Marker *m = dynamic_cast<Marker*>(pressed);
            if (m)
                emit clicked(m);
        }

        markerPressed = false;
    }
    emit mapTouched();
    this->setFocus();
    event->accept();
}

void GeoMap::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (panActive) {
        QPointF delta = event->lastPos() - event->pos();
        pan(delta.x(), delta.y());
        emit panned();
    }

    this->setFocus();
    event->accept();
}

void GeoMap::wheelEvent(QGraphicsSceneWheelEvent *event)
{

}

/**
  Actually, there's only support for the zoom key
*/
void GeoMap::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
        case Qt::Key_F7:
            if (zoomLevel() < maximumZoomLevel()) {
                setZoomLevel(zoomLevel() + 1);
            }
            break;
        case Qt::Key_F8:
            if (zoomLevel() > minimumZoomLevel()) {
                setZoomLevel(zoomLevel() - 1);
            }
            break;
    }
    emit mapTouched();
    this->setFocus();
    event->accept();
}

FixedGraphicsView::FixedGraphicsView(QGraphicsScene *scene, QWidget *parent) :
    QGraphicsView(scene, parent)
{
}

void FixedGraphicsView::scrollContentsBy(int dx, int dy)
{
    Q_UNUSED(dx)
    Q_UNUSED(dy)
}

MapsWidget::MapsWidget(QWidget *parent) :
    QWidget(parent)
{
    geomap = 0;
    view = 0;
}

MapsWidget::~MapsWidget()
{
}

void MapsWidget::initialize(QGeoMappingManager *manager)
{
    geomap = new GeoMap(manager, this);

    connect(geomap, SIGNAL(panned()),
            this, SIGNAL(mapPanned()));
    connect(geomap, SIGNAL(clicked(Marker*)),
            this, SIGNAL(markerClicked(Marker*)));

    sc = new QGraphicsScene;
    sc->addItem(geomap);

    geomap->setPos(0, 0);
    geomap->resize(this->size());

    view = new FixedGraphicsView(sc, this);
    view->setVisible(true);
    view->setInteractive(true);
    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    fsButtonItem = new FullscreenButtonItem(geomap);
    sc->addItem(fsButtonItem);
    sc->addItem(fsButtonItem->fsPixmap);
    connect(fsButtonItem, SIGNAL(mouseReleaseEventSignal()), this->parentWidget(), SLOT(toggleFullScreen()));
    fsButtonItem->timer = new QTimer(this);
    fsButtonItem->timer->setInterval(500);
    connect(fsButtonItem->timer, SIGNAL(timeout()), fsButtonItem, SLOT(onTimeOut()));
    fsButtonItem->timer->start();

    networkInfoView = new NetworkInfoView(geomap);
    sc->addItem(networkInfoView);
    networkInfoView->nameText->hide();
    networkInfoView->sigStrength->hide();
    networkInfoView->sigStrength1->hide();
    networkInfoView->strengthText->hide();
    networkInfoView->setRect(0,0,0,0);
    networkInfoView->visible = false;

    view->resize(this->size());
    view->centerOn(geomap);

//Sample markers
    /*
    Marker *me = new Marker(Marker::MyLocationMarker);
    me->setCoordinate(QGeoCoordinate(51.11, 17.022222));
    geomap->addMapObject(me);


    Marker *wlan = new Marker(Marker::OpenWlanMarker);
    wlan->setCoordinate(QGeoCoordinate(51.1115, 17.022256));
    geomap->addMapObject(wlan);*/
//End of sample markers

    resizeEvent(0);

    //Ugly hack for resizing mapsWidget, without that it's not taking whole window
    if(width() > height())
    {
        //We are in landscape mode
        view->resize(802,424);
        geomap->resize(800 - 2, 424 - 2);
    }
    else
    {
        //We are in portrait mode
        view->resize(482, 744);
        geomap->resize(480 - 2, 744 - 2);
    }
    view->centerOn(geomap);
    geomap->setCenter(QGeoCoordinate(51.11, 17.022222));
    geomap->setZoomLevel(15);
}

void MapsWidget::showFullscreenButton()
{
    fsButtonItem->fsPixmap->show();
    fsButtonItem->timer->start();
}

void MapsWidget::setMyLocation(QGeoCoordinate location, bool center)
{
    if (geomap && center)
        geomap->setCenter(location);
}

QGraphicsGeoMap *MapsWidget::map() const
{
    return geomap;
}

void MapsWidget::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event)

    if (view && geomap) {
        view->resize(size());
        geomap->resize(width()-2, height()-2);
        view->centerOn(geomap);

        fsButtonItem->setRect(width() - 48, height() - 48, 48, 48);
        if(networkInfoView->visible)
        {
            networkInfoView->nameText->hide();
            networkInfoView->sigStrength->hide();
            networkInfoView->sigStrength1->hide();
            networkInfoView->strengthText->hide();
            networkInfoView->setRect(0,0,0,0);
            /*
            networkInfoView->setRect(2,2,width()/2,height()/2);
            networkInfoView->nameText->show();
            networkInfoView->sigStrength->show();
            networkInfoView->sigStrength1->show();
            networkInfoView->strengthText->show();
            */
        }
    }
}

void MapsWidget::animatedPanTo(QGeoCoordinate center)
{
    if (!geomap)
        return;

    QPropertyAnimation *latAnim = new QPropertyAnimation(geomap, "centerLatitude");
    latAnim->setEndValue(center.latitude());
    latAnim->setDuration(200);
    QPropertyAnimation *lonAnim = new QPropertyAnimation(geomap, "centerLongitude");
    lonAnim->setEndValue(center.longitude());
    lonAnim->setDuration(200);

    QParallelAnimationGroup *group = new QParallelAnimationGroup;
    group->addAnimation(latAnim);
    group->addAnimation(lonAnim);
    group->start(QAbstractAnimation::DeleteWhenStopped);
}

void MapsWidget::showEvent(QShowEvent *event)
{
    Q_UNUSED(event)
    resizeEvent(0);
}

/**
  Adds WLAN marker based on network and our coordinates
*/
void MapsWidget::addWlanMarker(Network network, QGeoCoordinate cords)
{
    network.marker = new Marker(Marker::OpenWlanMarker);
    network.marker->setCoordinate(cords);
    network.marker->setName(network.id);
    geomap->addMapObject(network.marker);
}

/**
  Removes WLAN marker
*/
void MapsWidget::removeWlanMarker(Network network)
{
    geomap->removeMapObject(network.marker);
}

/**
  Shows wlan info dialog
  */
void MapsWidget::showWlanInfo(Marker *marker, QList<Network> wlanList)
{
    if(marker->getMarkerType() != 1)
        return;

    networkInfoView->setRect(2,2,320,130);
    networkInfoView->nameText->show();
    networkInfoView->sigStrength->show();
    networkInfoView->sigStrength1->show();
    networkInfoView->strengthText->show();
    networkInfoView->visible = true;

    loadWlanInfo(marker, wlanList);
}

/**
  Loads informations for the wlan info dialog
  */
void MapsWidget::loadWlanInfo(Marker *marker, QList<Network> wlanList)
{
    QString wlanName = marker->getName();
    bool found = false;
    int index = -1;

    for(int i = 0; i < wlanList.size(); i++)
    {
        if(!wlanName.compare(wlanList.at(i).id))
        {
            //It's what we're looking for
            found = true;
            index = i;
            break;
        }
    }


    if(found)
    {
        networkInfoView->setNetworkName(QString::fromStdString(wlanList.at(index).essid));
        networkInfoView->setStrength(wlanList.at(index).quality/100.0);
    }
}

FullscreenButtonItem::FullscreenButtonItem(GeoMap *map)
{
    map = map;
    pressedFullscreenButton = false;

    setPen(QPen(QBrush(), 0));
    setBrush(QBrush(Qt::transparent));

    fsPixmap = new QGraphicsPixmapItem(QPixmap("/usr/share/icons/hicolor/48x48/hildon/general_fullsize.png"));
}

void FullscreenButtonItem::setRect(qreal x, qreal y, qreal w, qreal h)
{
    QGraphicsRectItem::setRect(x, y, w, h);

    QRectF fsBound = fsPixmap->boundingRect();
    QPointF fsCenter(x+w/2.0, y+h/2.0);
    QPointF fsDelta = fsCenter - fsBound.center();
    fsPixmap->setPos(fsDelta);

}

void FullscreenButtonItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    const QPointF pos = event->pos();

    pressedFullscreenButton = true;
    timer->start();
    fsPixmap->show();
    event->accept();

}

void FullscreenButtonItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    emit mouseReleaseEventSignal();
    pressedFullscreenButton = false;
    event->accept();
}

void FullscreenButtonItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    event->accept();
}

void FullscreenButtonItem::onTimeOut()
{
    this->fsPixmap->hide();
    this->timer->stop();
}

NetworkInfoView::NetworkInfoView(GeoMap *map)
{
    map = map;
    setPen(QPen(QBrush(), 0));
    setBrush(QBrush(QColor(0,0,0,150)));

    nameText = new QGraphicsSimpleTextItem(this);
    strengthText = new QGraphicsSimpleTextItem(this);

    sigStrength= new QGraphicsRectItem(this);
    sigStrength1= new QGraphicsRectItem(this);

    sigStrength->setPen(QPen(QBrush(), 0));
    sigStrength->setBrush(QBrush(QColor(0,0,0,150)));

    sigStrength1->setPen(QPen(QBrush(), 0));
    sigStrength1->setBrush(QBrush(QColor(0,200,200,200)));



}

void NetworkInfoView::setRect(qreal x, qreal y, qreal w, qreal h){
    QGraphicsRectItem::setRect(x,y,w,h);

    QFont f;
    f.setFixedPitch(true);
    f.setPixelSize(20.0);

    nameText->setText("Network: ");
    nameText->setBrush(QBrush(Qt::white));
    nameText->setFont(f);

    nameText->setPos(20.0, 10.0);

    strengthText->setText("Strength: ");
    strengthText->setBrush(QBrush(Qt::white));
    strengthText->setFont(f);
    strengthText->setPos(20.0, 40.0);

    this->setBarBound(25,70,180,50);

}
/**
  Draws bar on the window
*/
void NetworkInfoView::setBarBound(int x,int y, int w, int h){
    xb=x;
    yb=y;
    wb=w;
    hb=h;
    sigStrength->setRect(xb,yb,wb,hb);
    sigStrength1->setRect(xb,yb,wb*0.0,hb);

}
/**
  Sets network name
  */

void NetworkInfoView::setNetworkName(QString s){
    nameText->setText("Network: "+s);
}


/**
  Sets signal strength; double type, from 0 to 1
  */
void NetworkInfoView::setStrength(double s){

    QString str="Strength: ";
    str.append(QString("%1").arg(s*100));
    strengthText->setText(str);

    sigStrength1->setRect(xb,yb,wb*s,hb);
}

void NetworkInfoView::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    event->accept();
}

/**
  Closes the window
  */
void NetworkInfoView::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    nameText->hide();
    sigStrength->hide();
    sigStrength1->hide();
    strengthText->hide();
    this->setRect(0,0,0,0);
    this->visible = false;

    event->accept();

}

NetworkInfoView::~NetworkInfoView(){
}

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

#include "mainwindow.h"

#include <QCoreApplication>
#include <QMenuBar>
#include <QMenu>
#include <QMessageBox>
#include <QAction>
#include <QVBoxLayout>
#include <QMap>
#include <QtGui/QX11Info>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <QDebug>
#include "config.h"

#ifdef Q_WS_MAEMO_5
    #include <QMaemo5InformationBox>
#endif

/**
    Initialize MainWindow
*/
MainWindow::MainWindow() :
    serviceProvider(0),
    positionSource(0),
    tracking(true),
    firstUpdate(true)
{
    //For auto-rotation, currently unsupported
    //setAttribute(Qt::WA_Maemo5AutoOrientation, true);

    // our actual maps widget is the centre of the mainwindow
    mapsWidget = new MapsWidget;
    setCentralWidget(mapsWidget);


    // set up the menus
    QMenuBar *mbar = new QMenuBar(this);
    mbar->addAction("My Location", this, SLOT(goToMyLocation()));
    mbar->addAction("Force scanning", this, SLOT(updateWlan()));
    //mbar->addAction("Available wlans", this, SLOT(showWlanAvailableWindow()));
    mbar->addAction("Settings", this, SLOT(showSettingsWindow()));
    mbar->addAction("Statistics", this, SLOT(showStatisticsWindow()));

    setMenuBar(mbar);
    setWindowTitle("wlan-navi");

    this->createConnection();
    this->createTable();

    // now begin the process of opening the network link
    netConfigManager = new QNetworkConfigurationManager;
    connect(netConfigManager, SIGNAL(updateCompleted()),
            this, SLOT(openNetworkSession()));
    netConfigManager->updateConfigurations();

    settingsWindow = new Settings(this);

    scannedCntr=0;
    this->openScannedCntr = 0;

    this->grabZoomKeys(true);
    this->initializeWlan();
}

void MainWindow::setNetworkID(int i)
{

    int BufSize = 255;
    char latitude[BufSize];
    char longitude[BufSize];

    ::snprintf(latitude, BufSize, "%2.3f", myCords.latitude());
    ::snprintf(longitude, BufSize, "%2.3f", myCords.longitude());
    QString s_latitude = QString::fromAscii(latitude);
    QString s_longitude = QString::fromAscii(longitude);
    QString essid = QString::fromStdString(tempWlan.at(i).essid);
    QString id = s_latitude + s_longitude + essid;
    QString addInfo;

    if(!checkIfExists(id))
    {
        qDebug() << "\nWLAN doesn't exists! Adding new.";
        Network net = tempWlan.at(i);
        net.id = id;
        wlans.append(net);
        mapsWidget->addWlanMarker(wlans.last(), getMyCords());
        this->insertWlan(id, QString::fromStdString(wlans.last().essid), myCords.latitude(), myCords.longitude(), wlans.last().quality, addInfo);
        /**/
    }
    else
    {
        int temp = getNetworkWithID(id);
        if(temp != -1)
        {
            if(wlans.at(temp).quality < tempWlan.at(i).quality)
            {
                qDebug() << "\nWLAN exists! Adding new with better signal quality.";
                //Lower signal quality, actual network is better
                mapsWidget->removeWlanMarker(wlans.at(temp));
                this->deleteWlan(wlans.at(temp).id);

                Network net = tempWlan.at(i);
                net.id = id;
                wlans.replace(temp, net);

                mapsWidget->addWlanMarker(wlans.at(temp), getMyCords());
                this->insertWlan(id,  QString::fromStdString(wlans.last().essid), myCords.latitude(), myCords.longitude(), wlans.at(temp).quality,addInfo);
            }
            else
            {
                qDebug() << "\nWLAN exists! I'm not addind a new one.";
            }
        }
    }
}

int MainWindow::getNetworkWithID(QString id)
{
    for(int i = 0; i < wlans.size(); i++)
    {
        if(wlans.at(i).id.compare(id) == 0)
        {
            qDebug() << "\nGetting network " << id << " exists";
            return i;
        }
    }
    qDebug() << "\nNetwork " << id << " doesn't exist";
    return -1;
}

bool MainWindow::checkIfExists(QString id)
{
    for(int i = 0; i < wlans.size(); i++)
    {
        int a = wlans.at(i).id.compare(id);
        qDebug() << "\n a: " << a << " id_receiv: " << id << " net_id: " << wlans.at(i).id;
        if(wlans.at(i).id.compare(id) == 0)
        {
            qDebug() << "\nNetwork " << id << " exists";
            return true;
        }
    }
    qDebug() << "\nNetwork " << id << " doesn't exist";
    return false;
}

/**
  Update wlan status
*/
void MainWindow::updateWlan()
{
    wlanInterface->UpdateNetworks();
    #ifdef Q_WS_MAEMO_5
        QMaemo5InformationBox::information(this, "Wifi status updated");
    #endif
}

/**
  Add new markers for just discovered wlans
*/
void MainWindow::updateWlanAddMarker()
{
    //Sets uniqe id for each available network

    tempWlan.clear();
    tempWlan = wlanInterface->getWlans();

    for(int i = 0; i < tempWlan.size(); i++)
    {
        this->scannedCntr++;
        if(encryptionToString(tempWlan.at(i).encryption).compare("None") == 0 || encryptionToString(tempWlan.at(i).encryption).compare("WEP") == 0)
        {
            qDebug() << "\nWLAN: " << QString::fromStdString(tempWlan.at(i).essid);
            this->openScannedCntr++;
            setNetworkID(i);
            qDebug() << "\nWLAN2: "<< tempWlan.at(i).id << ";" << QString::fromStdString(tempWlan.at(i).essid);
        }
        else
        {
            qDebug() << "\nWLAN: " << QString::fromStdString(tempWlan.at(i).essid)  << encryptionToString(tempWlan.at(i).encryption);
        }
    }

    for(int i = 0; i < wlans.size(); i++)
    {
        qDebug() << "\nWIFI: "<< wlans.at(i).id << ";" << QString::fromStdString(wlans.at(i).essid);
    }

    wlanInterface->clearList();
}

/**
  Initialize wlan interface
*/
void MainWindow::initializeWlan()
{
    wlanInterface = new WlanMaemo();
    //wlanInterface->setWlans(&wlans);
    wlanInterface->UpdateNetworks();
}

/**
  Open a network connection
*/
void MainWindow::openNetworkSession()
{
    session = new QNetworkSession(netConfigManager->defaultConfiguration());
    if (session->isOpen()) {
        initialize();
    } else {
        connect(session, SIGNAL(opened()),
                this, SLOT(initialize()));
        session->open();
    }
}

MainWindow::~MainWindow()
{
    delete mapsWidget;
    if (serviceProvider)
        delete serviceProvider;
}

/**
  Show our position
*/
void MainWindow::goToMyLocation()
{
    mapsWidget->map()->setFocus();
    tracking = true;
}

/**
  Set up the mapsWidget
*/
void MainWindow::initialize()
{
    if (serviceProvider)
        delete serviceProvider;

    QList<QString> providers = QGeoServiceProvider::availableServiceProviders();

    for (int i = 0; i < providers.size(); ++i)
       qDebug() << providers.at(i).toLocal8Bit().constData() << endl;

    if (providers.size() < 1) {
        QMessageBox::information(this, tr("Wlan-navi problem"),
                                 tr("No service providers are available"));
        QCoreApplication::quit();
        return;
    }

 //   serviceProvider = new QGeoServiceProvider(loadQGeoPlugin());

    serviceProvider = new QGeoServiceProvider("nokia");

    if(loadQGeoPlugin().compare("nokia") == 0)
    {

        m_pViewActionGroup = new QActionGroup(this);
        m_pViewActionGroup->setExclusive(true);

        QAction *pAction;
        pAction = new QAction("Street Map", m_pViewActionGroup); pAction->setCheckable(true); pAction->setChecked(1);
        pAction = new QAction("Satellite Map", m_pViewActionGroup); pAction ->setCheckable(true); pAction->setChecked(0);
        menuBar()->addActions(m_pViewActionGroup->actions());
        connect(m_pViewActionGroup, SIGNAL(triggered(QAction*)), this, SLOT(onViewChanged(QAction*)));
    }

    mapsWidget->initialize(serviceProvider->mappingManager());

    connect(mapsWidget, SIGNAL(mapPanned()),
            this, SLOT(disableTracking()));
    connect(mapsWidget, SIGNAL(markerClicked(Marker*)),
            this, SLOT(showMarkerInfoWindow(Marker*)));
    connect(wlanInterface, SIGNAL(wlansUpdated()),
            this, SLOT(updateWlanAddMarker()));

    if (positionSource)
        delete positionSource;

    positionSource = QGeoPositionInfoSource::createDefaultSource(this);

    if (!positionSource) {
        mapsWidget->setMyLocation(QGeoCoordinate(51.11, 17.022222));
        myCords = QGeoCoordinate(51.11, 17.022222);
    } else {
        me = new Marker(Marker::MyLocationMarker);
        positionSource->setUpdateInterval(1000);
        connect(positionSource, SIGNAL(positionUpdated(QGeoPositionInfo)),
                this, SLOT(updateMyPosition(QGeoPositionInfo)));
        positionSource->startUpdates();
    }

    loadWlanSQL();

    wlanTimer = new QTimer(this);
    wlanTimer->setInterval(loadWlanTimerSettings());
    connect(wlanTimer, SIGNAL(timeout()), this, SLOT(updateWlan()));
    connect(settingsWindow, SIGNAL(wlanTimerUpdated()),
            this, SLOT(reloadWlanTimerSettings()));
    wlanTimer->start();

}

/**
  Changes map type. Only OVI Maps
*/
void MainWindow::onViewChanged(QAction* pAction)
{
    qDebug() << "\nVALUE: " << m_pViewActionGroup->actions().indexOf(pAction);
    int value = m_pViewActionGroup->actions().indexOf(pAction);

    if(value)
        mapsWidget->geomap->setMapType(QGraphicsGeoMap::SatelliteMapDay);
    else
        mapsWidget->geomap->setMapType(QGraphicsGeoMap::StreetMap);
}

/**
  Load settings // QGeoServiceProvider plugin for maps
*/
QString MainWindow::loadQGeoPlugin()
{
    QString plugin = "";

    GetKeyString(&plugin, "qgeoplugin");
    if(!plugin.compare("nokia") || !plugin.compare("openstreetmap"))
        return plugin;
    else
        return "openstreetmap";
}

/**
  Reloads wlanTimer config
*/
void MainWindow::reloadWlanTimerSettings()
{
    this->wlanTimer->stop();
    wlanTimer->setInterval(loadWlanTimerSettings());
    this->wlanTimer->start();
}

/**
  Loads settings for wlanTimer
*/
int MainWindow::loadWlanTimerSettings()
{
    int value = 60;

    GetKeyInt(&value, "wlanTimer");
    value = value * 1000;
    return value;
}

void MainWindow::disableTracking()
{
    tracking = false;
}

/**
  Locate us on a map
*/
void MainWindow::updateMyPosition(QGeoPositionInfo info)
{
    if (mapsWidget) {
        mapsWidget->setMyLocation(info.coordinate(), false);
        if (tracking)
            mapsWidget->animatedPanTo(info.coordinate());

        me->setCoordinate(info.coordinate());
        mapsWidget->geomap->addMapObject(me);
    }
    if (firstUpdate) {
        firstUpdate = false;
    }
    myCords = info.coordinate();
}

QGeoCoordinate MainWindow::getMyCords()
{
    return myCords;
}

void MainWindow::showNavigateDialog()
{

}

void MainWindow::showSearchDialog()
{

}

void MainWindow::showWlanAvailableWindow()
{
    WlanAvailable *window = new WlanAvailable(this);
    window->show();
}

void MainWindow::showSettingsWindow()
{
    settingsWindow->showSettingsWindow();
}

void MainWindow::showMarkerInfoWindow(Marker *marker)
{
    mapsWidget->showWlanInfo(marker, wlans);
}

void MainWindow::showStatisticsWindow()
{
    Statistics *window = new Statistics(this);
    window->loadData(wlans.size(), scannedCntr, openScannedCntr);
    window->show();
}

/**
  Enable zoom keys
*/
void MainWindow::grabZoomKeys(bool grab)
{
    #ifdef Q_WS_MAEMO_5
        if (!winId()) {
            qWarning("Can't grab keys unless we have a window id");
            return;
        }

        unsigned long val = grab ? 1 : 0;
        Atom atom = XInternAtom(QX11Info::display(), "_HILDON_ZOOM_KEY_ATOM", False);
        if (!atom) {
            qWarning("Unable to obtain _HILDON_ZOOM_KEY_ATOM. This will only work "
                    "on a Maemo 5 device!");
            return;
        }

        XChangeProperty (QX11Info::display(),
                winId(),
                atom,
                XA_INTEGER,
                32,
                PropModeReplace,
                reinterpret_cast<unsigned char *>(&val),
                1);
    #endif
}

/**
  Turns fullscreen mode on and off.
*/
void MainWindow::toggleFullScreen()
{
    bool isFullScreen = windowState() & Qt::WindowFullScreen;

    if (isFullScreen)
        showNormal();
    else
        showFullScreen();
}


/**
  SQL
  */

void MainWindow::loadWlanSQL()
{
    qDebug() << "\nLoadWlan";
    Network net;
    QGeoCoordinate cord;
    QSqlQuery query("SELECT * FROM Wlan");
    while (query.next()) {
        net.id = query.value(0).toString();
        net.essid = query.value(1).toString().toStdString();

        cord.setLatitude(query.value(2).toDouble());
        cord.setLongitude(query.value(3).toDouble());
        net.quality = query.value(4).toInt();
        qDebug() << "\nloadTEST: " << query.value(0).toString() <<  ";" << cord.latitude() << ";" << cord.longitude() << "; qual: " << net.quality << ";" << query.value(4);
        wlans.append(net);
        this->mapsWidget->addWlanMarker(net, cord);
    }
}

void MainWindow::insertWlan(QString id, QString ssid, double latitude, double longitute, int networkQuality, QString additionalInf)
{
    qDebug() << "\ninsertWlan " << latitude << " " << longitute;
    QSqlQuery query;
    QString str;
    bool ret = false;

    ret = query.exec(QString("insert into Wlan values('%1','%2',%3,%4,%5,'%6')")
            .arg(id).arg(ssid).arg(latitude).arg(longitute).arg(networkQuality).arg(additionalInf));
}


void MainWindow::deleteWlan(QString id)
{

    QSqlQuery query;
    QString queryy = "delete from Wlan where Wlan.id ='"+id+"';";
    query.exec(queryy);
}

void MainWindow::createTable(){
    QSqlQuery query;
    bool ret = false;
    ret = query.exec("create table if not exists Wlan(id varchar(90) PRIMARY KEY, essid VARCHAR(50), latitude REAL, longitude REAL, networkQuality INTEGER, additionalInf varchar(90));");
}

bool MainWindow::createConnection()
        {
        db = QSqlDatabase::addDatabase("QSQLITE");
        QString path(QDir::home().path());
            path.append(QDir::separator()).append("my.db.sqlite");
            path = QDir::toNativeSeparators(path);
        db.setDatabaseName(path);
        if (!db.open()) {
            QMessageBox::critical(0, "Cannot open database",
            "Unable to establish a database connection.\n"
            "Click Cancel to exit.", QMessageBox::Cancel);
            return false;
        }
        return true;
}

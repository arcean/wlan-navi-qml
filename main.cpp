#include "mainwindow.h"

#include <QApplication>
#include <QList>
#include <QString>
#include <QUrl>
#include <QSettings>
#include <QProcessEnvironment>
#include <QNetworkProxyFactory>

#include "qgeoserviceprovider.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QApplication::setOrganizationName("K0l40a");
    QApplication::setApplicationName("wlan-navi");

    QSettings settings;

    QVariant value = settings.value("http.proxy");
    if (value.isValid()) {
        QUrl url(value.toString(), QUrl::TolerantMode);
        QNetworkProxy proxy;
        proxy.setType(QNetworkProxy::HttpProxy);
        proxy.setHostName(url.host());
        proxy.setPort(url.port(8080));
        QNetworkProxy::setApplicationProxy(proxy);
    }


    MainWindow mw;

    //mw.setAttribute(Qt::WA_Maemo5StackedWindow);

#if defined(Q_OS_SYMBIAN) || defined(Q_WS_MAEMO_5) || defined(Q_WS_SIMULATOR)
    mw.showMaximized();
#else
    mw.show();
#endif

    return a.exec();
}

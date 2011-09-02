#ifndef WLANAVAILABLE_H
#define WLANAVAILABLE_H

#include <QMainWindow>

#include "WlanMaemo.h"
#include "mainwindow.h"

namespace Ui {
    class WlanAvailable;
}

class WlanAvailable : public QMainWindow
{
    Q_OBJECT

public:
    explicit WlanAvailable(QWidget *parent = 0);
    ~WlanAvailable();

public slots:
  /*  void setWlans(QList<Network> *wlans);
    QList<Network>* getWlans();*/

private slots:
    void loadWlans();

private:
    Ui::WlanAvailable *ui;
  //  QList<Network> *wlans;
};

#endif // WLANAVAILABLE_H

#include "statistics.h"
#include "ui_statistics.h"

Statistics::Statistics(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Statistics)
{
    ui->setupUi(this);
    setWindowTitle("Statistics");
    //setAttribute(Qt::WA_Maemo5StackedWindow);
}

void Statistics::loadData(int saved, int today, int todayOpen)
{
    QString saved_str;
    ui->savedLabel->setText(saved_str.setNum(saved));
    ui->scannedLabel->setText(saved_str.setNum(today));
    ui->todayopenLabel->setText(saved_str.setNum(todayOpen));
}

Statistics::~Statistics()
{
    delete ui;
}

#include "qtsqlcheck.h"

qtsqlcheck::qtsqlcheck()
{
   QSqlDatabase db2 = QSqlDatabase::addDatabase("QMYSQL");

   db2.setHostName("localhost");
   db2.setDatabaseName("qt");
   db2.setUserName("login");
   db2.setPassword("pass");

   if (!db2.open())
   {
      qDebug() << "Error: can't connect to a database!";
   }
   else
   {

      QSqlQuery query("create table if not exists Wlan(id VARCHAR PRIMARY KEY, essid VARCHAR, latitude DOUBLE, longitude DOUBLE, netowrkQuality INTEGER, additionalInf TEXT", db2);

      db2.close();
   }
     return;
}

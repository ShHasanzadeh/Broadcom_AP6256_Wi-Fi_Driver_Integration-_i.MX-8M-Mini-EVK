#ifndef INTERNETCHECKER_H
#define INTERNETCHECKER_H

#include <QObject>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QLabel>
#include <QPixmap>
#include "wifiwindow.h"

//class WifiWindow;

class InternetChecker : public QObject {
    Q_OBJECT
public:
   // explicit InternetChecker(QLabel *statusLabel, QObject *parent = nullptr);
    InternetChecker(QLabel *statusLabel, WifiWindow *wifiWindow, QObject *parent = nullptr);

private slots:
    //void checkInternet();
    //void replyFinished(QNetworkReply *reply);

private:

    QLabel *label;
    QTimer *timer;
    QNetworkAccessManager *manager;
    WifiWindow *wifiWindow;

    /*  to ping the  OTA server */
    QTcpSocket *tcpSocket;
    QTimer *connectionTimer;
    bool checkFinished = true;

        void checkInternet();
        void finishInternetCheck(bool connected);
};

#endif // INTERNETCHECKER_H

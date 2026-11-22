#include "internetchecker.h"

InternetChecker::InternetChecker(QLabel *statusLabel,
                               WifiWindow *wifiWindow,
                               QObject *parent)
    : QObject(parent),
      label(statusLabel),
      wifiWindow(wifiWindow)
{
    // TCP connection test
    tcpSocket = new QTcpSocket(this);
    connectionTimer = new QTimer(this);
    connectionTimer->setSingleShot(true);

    // Check connection success
    connect(tcpSocket, &QTcpSocket::connected, this, [this]()
    {
        finishInternetCheck(true);
    });

    // Check connection error
    connect(tcpSocket, &QTcpSocket::errorOccurred, this,
            [this](QAbstractSocket::SocketError error)
    {
        Q_UNUSED(error);
        finishInternetCheck(false);
    });

    // Connection timeout (3 seconds)
    connect(connectionTimer, &QTimer::timeout, this, [this]()
    {
        finishInternetCheck(false);
    });

    // Initial disconnected icon
    QPixmap icon(":/icons/icons/images_tp_dis.png");

    label->setPixmap(icon.scaled(40, 40,
                                Qt::KeepAspectRatio,
                                Qt::SmoothTransformation));

    // Periodic check timer
    timer = new QTimer(this);

    connect(timer, &QTimer::timeout,
            this, &InternetChecker::checkInternet);

    // Initial check
    checkInternet();

    // Check every 10 seconds
    timer->start(10000);
}


// Check OTA server connection
void InternetChecker::checkInternet()
{
    // Prevent overlapping checks
    if (!checkFinished)
        return;

    checkFinished = false;

    // Reset previous connection if necessary
    if (tcpSocket->state() != QAbstractSocket::UnconnectedState)
    {
        tcpSocket->abort();
    }

    connectionTimer->stop();
    connectionTimer->start(3000);

    tcpSocket->connectToHost("37.32.12.151", 4152);

    //qDebug() << "Checking server connection...";
}


// Update internet status
void InternetChecker::finishInternetCheck(bool connected)
{
    // Prevent multiple results for the same test
    if (checkFinished)
        return;

    checkFinished = true;

    connectionTimer->stop();
    tcpSocket->abort();

    if (connected)
    {
        QPixmap icon(":/icons/icons/images_tp.png");

        label->setPixmap(icon.scaled(40, 40,
                                    Qt::KeepAspectRatio,
                                    Qt::SmoothTransformation));

       // qDebug() << "SERVER CONNECTION OK";
    }
    else
    {
        QPixmap icon(":/icons/icons/images_tp_dis.png");

        label->setPixmap(icon.scaled(40, 40,
                                    Qt::KeepAspectRatio,
                                    Qt::SmoothTransformation));

        //qDebug() << "SERVER CONNECTION FAILED";
    }

    // Update Wi-Fi list
    wifiWindow->get_current_wifi();
}

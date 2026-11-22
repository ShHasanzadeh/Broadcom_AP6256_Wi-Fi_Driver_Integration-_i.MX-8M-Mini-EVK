#ifndef WIFIWINDOW_H
#define WIFIWINDOW_H

#include <QWidget>
#include <QPushButton>
#include <QListWidgetItem>
#include <QTimer>
#include <QString>
#include <QStringList>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QListWidgetItem>
#include <QProcess>
#include <QMessageBox>
#include <QApplication>
#include <QInputDialog>
#include <QRect>
#include <QString>
#include <QVBoxLayout>
#include <QLabel>
#include <QProcess>

#include <dirent.h>
#include <unistd.h>

#include <QStatusBar>


extern QString select_ssid;
extern bool connect_sta;
extern QString cur_id;


namespace Ui {
class WifiWindow;
}

class WifiWindow : public QDialog
{
    Q_OBJECT

public:
    explicit WifiWindow(QDialog *parent = 0);
    ~WifiWindow();
    Ui::WifiWindow *ui;
    void setStyle();


private slots:
    void on_search_BT_clicked();

    void on_connect_BT_clicked();

    void timerTimeOut();

    void on_pushButton_2_clicked();

    void on_disconnect_BT_clicked();

    void on_unsave_BT_clicked();


    void on_listWidget_2_itemClicked(QListWidgetItem *item);

    void on_listWidget_itemClicked(QListWidgetItem *item);

    void wifi_init();

    void on_enable_CB_stateChanged(int arg1);

    void on_exitBt_clicked();

    void button_disable(int sta);

    void on_exit_BT_clicked();

private:
    QTimer *timer;
    QProcess *process;
    QString CNT_GREY_COLOR="6F6F6E";
    QString CNT_DARKBLUE_COLOR="004F97";
    void clearLayoutItem();
    QString getWifiMacAddress();

public slots:
     QString process_create(QString command,QStringList arguments);
     QStringList spit_str(QString str);
     QString spit_list(QString str,QString ssid);
     void closewindow();
     void get_current_wifi();
     void uishow(bool sta);
     QStringList spit_connected(QString str);
     void get_wifi_sta();
     void wifi_connect_disable(bool sta);

     void ShowMessage(QString msg,int timeOut=3500,bool isSuccess=true);
     // QWidget interface
protected:
     void showEvent(QShowEvent *event);
};

struct Wifi_struct
{
    QString wpa_cli = "wpa_cli";
    QStringList scan{"-i", "wlan0", "scan"};
    QStringList scan_result{"-i", "wlan0", "scan_result"};
    QStringList add_network{"-i", "wlan0", "add_network"};
    QStringList set_network{"-i", "wlan0", "set_network"};
    QStringList enable_network{"-i", "wlan0", "enable_network"};
    QStringList status{"-i", "wlan0", "status"};
    QStringList disable_network{"-i", "wlan0", "disable_network"};
    QStringList list_network{"-i", "wlan0", "list_network"};
    QStringList remove_network{"-i", "wlan0", "remove_network"};
    QStringList save_config{"-i", "wlan0", "save_config"};
};

extern Wifi_struct wifi;
#endif // MAINWINDOW_H

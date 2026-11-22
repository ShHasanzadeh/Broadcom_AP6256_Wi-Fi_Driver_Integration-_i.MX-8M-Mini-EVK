
#include "wifiwindow.h"
#include "ui_wifiwindow.h"

#include "passworddialog.h"
#include "internetchecker.h"

bool scan_timer = 0;
bool connect_timer = 0;
bool connect_sta = 0;
bool enable_time = 0;
int times = 0;
bool WifiisAlive = false;
QString select_ssid;
QString cur_id,cur_ssid;
Wifi_struct wifi;

//static void showWarningMessage(QString msg){
//    QMessageBox message(QMessageBox::Warning, "Warning", msg, QMessageBox::Yes, NULL);
//    message.exec();
//}

//static void showInformationMessage(QString msg){
//    QMessageBox message(QMessageBox::Information, "Tip", msg, QMessageBox::Yes, NULL);
//    message.exec();
//}



static int find_pid_by_name( char* ProcName, int* foundpid)
{
    DIR             *dir;
    struct dirent   *d;
    int             pid, i;
    char            *s;
    int pnlen;

    i = 0;
    foundpid[0] = 0;
    pnlen = strlen(ProcName);

    /* Open the /proc directory. */
    dir = opendir("/proc");
    if (!dir)
    {
        printf("cannot open /proc");
        return -1;
    }

    /* Walk through the directory. */
    while ((d = readdir(dir)) != NULL) {

        char exe [PATH_MAX+1];
        char path[PATH_MAX+1];
        int len;
        int namelen;

        /* See if this is a process */
        if ((pid = atoi(d->d_name)) == 0)       continue;

        snprintf(exe, sizeof(exe), "/proc/%s/exe", d->d_name);
        if ((len = readlink(exe, path, PATH_MAX)) < 0)
            continue;
        path[len] = '\0';

        /* Find ProcName */
        s = strrchr(path, '/');
        if(s == NULL) continue;
        s++;

        /* we don't need small name len */
        namelen = strlen(s);
        if(namelen < pnlen)     continue;

        if(!strncmp(ProcName, s, pnlen)) {
            /* to avoid subname like search proc tao but proc taolinke matched */
            if(s[pnlen] == ' ' || s[pnlen] == '\0') {
                foundpid[i] = pid;
                i++;
            }
        }
    }
    foundpid[i] = 0;
    closedir(dir);

    return  0;

}

/***************************************************************************************
* @brief Determine if a process exists
* @param char *str
* @return bool exists->1, does not exist->0
 ***************************************************************************************/
bool isAlive(char *str){
    //    qDebug("zyz --> isAliveAcc --> %s!\n", QCoreApplication::applicationName().toLatin1().data());
    int i,ret,pid_t[128] = {0},num=0;
    ret = find_pid_by_name(str, pid_t);
    if(!ret){
        for(i=0; pid_t[i]!=0; i++){
            qDebug("zyz0 --> %d\n", pid_t[i]);
            num++;
        }
    }
    if(num > 0)
        return true;
    else {
        return false;
    }
}


WifiWindow::WifiWindow(QDialog *parent) :
    QDialog(parent),
    ui(new Ui::WifiWindow)
{



    ui->setupUi(this);

    //sh_new design
    ui->lblWifiMac->setText(getWifiMacAddress());

    InternetChecker *checker = new InternetChecker(ui->labelInternet, this);

    setGeometry(QRect(0,0,this->width(),this->height()));
    this->setWindowTitle("Wifi connect");

    wifi_init();

    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(timerTimeOut()));

    if (ui->enable_CB->isChecked() && isAlive("wpa_supplicant")){
        button_disable(0);
        //qDebug()<< "wifi is Enable at the First Time";
    }else{
        button_disable(1);
       // qDebug()<< "wifi is Disable at the First Time";
    }



}

//sh_new design
/* To get Mac Address via OS */
QString WifiWindow::getWifiMacAddress()
{
    QFile file("/sys/class/net/wlan0/address");

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "Cannot read WiFi MAC Address";
        return "Unavailable";
    }

    QTextStream in(&file);
    QString macAddress = in.readLine().trimmed().toUpper();

    file.close();

    return macAddress;
}


void WifiWindow::showEvent(QShowEvent *event)
{

    setStyle();

}
void WifiWindow::setStyle()
{

    QString btnStyle = QString(R"(QPushButton{
                         border: 1px solid #%0;
                         border-radius: 5px;
                         background-color:white;
                        }
                        QPushButton:hover{
                          border: 2px solid #%1;
                         }
                        QPushButton:pressed{
                          border: 2px solid #%1;
                         })").arg(CNT_GREY_COLOR)
            .arg(CNT_DARKBLUE_COLOR);


    ui->search_BT->setStyleSheet(btnStyle);
    ui->unsave_BT->setStyleSheet(btnStyle);
    ui->connect_BT->setStyleSheet(btnStyle);
    ui->disconnect_BT->setStyleSheet(btnStyle);
    ui->exit_BT->setStyleSheet(btnStyle);

}
void WifiWindow::clearLayoutItem()
{
    QLayoutItem *wItem;
     while ((wItem = ui->statusBarLayout->layout()->takeAt(0)) != 0)
     {
         if (wItem->widget())
             wItem->widget()->setParent(NULL);
         delete wItem;
     }
}
void WifiWindow::ShowMessage(QString msg, int timeOut, bool isSuccess)
{
    clearLayoutItem();

      QStatusBar *statusBar = new QStatusBar;
      if (isSuccess)
          statusBar->setStyleSheet("color:green");
      else
          statusBar->setStyleSheet("color:red");

      QFont font("Ubuntu",12);
      font.setBold(true);
      statusBar->setFont(font);

      ui->statusBarLayout->addWidget(statusBar);
      statusBar->showMessage(msg, timeOut);
      ui->statusBarLayout->setMargin(0);
      ui->statusBarLayout->setSpacing(0);
}

WifiWindow::~WifiWindow()
{
    delete ui;
}


void WifiWindow::closewindow()
{
    this->close();
}


//wpa_cli -i wlan0 scan_result
void WifiWindow :: timerTimeOut()
{
    if(scan_timer == 1)
    {
        times ++;
        //Reading results

        QString scan_out = process_create(wifi.wpa_cli,wifi.scan_result);
        qDebug() << "scan_out : "<< scan_out;
        if(scan_out.count("\n")>2)//Results have more than one row
        {
            QStringList scan_ssid = spit_str(scan_out);
            ui->listWidget->addItems(scan_ssid);
            timer->stop();
            scan_timer = 0;
            times=0;
        }

        if(times > 20)
        {
            connect_timer = 0;
            times=0;
            timer->stop();
        }

    }
    if(connect_timer == 1)
    {
        times ++;
        get_wifi_sta();
        if(connect_sta == 1)
        {
            connect_timer = 0;
            timer->stop();
            QString msg = select_ssid+" connect successfully";
            //showInformationMessage(msg);
            ShowMessage(msg);
            QTimer::singleShot(1000, this, [this]()
            {
                get_current_wifi();
            });
            times = 0;
        }

        if(times > 20)
        {
            connect_timer = 0;
            times=0;
            timer->stop();
        }

    }
    if(enable_time = 1)
    {
        times++;
        if(times > 5)
        {
            QTimer::singleShot(1000, this, [this]()
            {
                get_current_wifi();
            });
            timer->stop();
            enable_time = 0;
            times = 0;
            button_disable(0);

            process= new QProcess(this);
            process->start("udhcpc -i wlan0");
        }
    }

}

//wpa_cli -i wlan0 scan
void WifiWindow::on_search_BT_clicked()
{
    QString output = process_create(wifi.wpa_cli,wifi.scan);
    qDebug() << "scan : "<< output;

    ui->listWidget->clear();
    scan_timer = 1;
    timer->start(1000);
}

//Connect Button
void WifiWindow::on_connect_BT_clicked()
{
    QListWidgetItem *selectedItem = ui->listWidget->currentItem();
    QListWidgetItem *selectedItem2 = ui->listWidget_2->currentItem();


    //Connect to the searched wifi
    if (selectedItem) {

        //Get the current connection
        get_wifi_sta();

        qDebug() << "cur_ssid: " << cur_ssid;

        //Get the selected device
        select_ssid = selectedItem->text().trimmed();
        qDebug() << "select_ssid" << select_ssid;

        if(select_ssid == cur_ssid)
        {
            QString msg = select_ssid+" is already connected";
            //showWarningMessage(msg);
            ShowMessage(msg);
            return;
        }

        if(connect_sta == 1)
        {
            QStringList argu;
            argu << wifi.disable_network << cur_id;
            QString connect_result = process_create(wifi.wpa_cli,argu);
            qDebug() << "connect_result : "<< connect_result;
        }

        passworddialog* passwordui = new passworddialog(this);
        connect(passwordui,&passworddialog::wifi_connect,this,&WifiWindow::uishow);
        connect(passwordui,&passworddialog::ShowMessageSignal,this,&WifiWindow::ShowMessage);
        passwordui->show();

    }

    //Connect to saved wifi
    else if (selectedItem2) {

        //Get the current connection
        get_wifi_sta();

        qDebug() << "cur_ssid: " << cur_ssid;

        //Get the selected device
        select_ssid = selectedItem2->text().split("\t", QString::SkipEmptyParts).at(1);
        qDebug() << "select_ssid" << select_ssid;

        //Get the selected device
        QString select_id = selectedItem2->text().split("\t", QString::SkipEmptyParts).at(0);
        qDebug() << "select_id" << select_id;

        if(select_ssid == cur_ssid)
        {
            QString msg = select_ssid+" is already connected";
            //showWarningMessage(msg);
            ShowMessage(msg);
            return;
        }

        //Disconnect an existing connection
        if(connect_sta == 1)
        {
            QStringList argu;
            argu << wifi.disable_network << cur_id;
            QString connect_result = process_create(wifi.wpa_cli,argu);
            qDebug() << "connect_result : "<< connect_result;
        }

        //wpa_cli -i wlan0 enable_network ssid
        QStringList argu;
        argu << wifi.enable_network << select_id;
        QString connect_result = process_create(wifi.wpa_cli,argu);
        qDebug() << "connect_result : "<< connect_result;
        timer->start(1000);
        connect_timer = 1;

    }

    //Get IP
    process= new QProcess(this);
    process->start("udhcpc -i wlan0");


}

//Disconnect the currently connected wifi
//wpa_cli -i wlan0 disable_network 0
void WifiWindow::on_disconnect_BT_clicked()
{
    QListWidgetItem *selectedItem2 = ui->listWidget_2->currentItem();
    if (selectedItem2) {
        QString select_id = selectedItem2->text().split("\t", QString::SkipEmptyParts).at(0);
        qDebug() << "on_disconnect_BT_clicked()" << "select ssid: " << select_id;

        QStringList argu;
        argu << wifi.disable_network << select_id;
        QString result = process_create(wifi.wpa_cli,argu);
        qDebug() << "result : "<< result;
        //showInformationMessage("Wifi is disconnected!");
        ShowMessage("Wifi is disconnected!");
        QTimer::singleShot(1000, this, [this]()
        {
            get_current_wifi();
        });
    }
}


/***************************************************************************************
* @brief Create a process
* @param command command
* @param arguments parameters
* @return process standard output
 ***************************************************************************************/
QString WifiWindow::process_create(QString command,QStringList arguments)
{
    QProcess process;

    qDebug() << command << arguments;
    process.start(command, arguments);
    process.waitForFinished(); // Wait for the command to complete
    QByteArray output = process.readAllStandardOutput();
    QString outputString = QString::fromUtf8(output);
    return outputString;
}


/***************************************************************************************
* @brief Filter wifi search results, only display signal and ssid
* @param str wifi search results
* @return only with signal value and wifi name
* //bssid / frequency / signal level / flags / ssid
 ***************************************************************************************/
QStringList WifiWindow::spit_str(QString str)
{
    QStringList lines = str.split('\n');
    QStringList output;
    QSet<QString> res;

    for (const QString& line : lines) {
        QStringList parts = line.split("\t", QString::SkipEmptyParts);
        if(parts.size() >= 5)
        {
            int fixedWidth = 30; // want a fixed width
            QString content = parts.at(4); // Content obtained from parts
            content = content.leftJustified(fixedWidth,' '); // Left-aligned and padded with spaces to a fixed width
            //             output.append(content + "signal:" +parts.at(2));
            //output.append(content);
            res.insert(content);
        }
        else
        {
            //             qDebug("%s","size is not ok");
            //             break;
        }
    }
    output.clear();
    output.append(res.values());
    return output;
}



/***************************************************************************************
* @brief Display connected devices
* network id / ssid / bssid / flags
 ***************************************************************************************/
QStringList WifiWindow::spit_connected(QString str)
{
    QStringList lines = str.split('\n');
    QStringList output;

    for (int i = 1; i < lines.size() - 1; ++i) {  // Start looping from the second element
        output.append(lines.at(i));
    }
    return output;
}


/***************************************************************************************
* @brief Get id based on ssid value
* @param str
* @param ssid wifi name
* @return id value
 ***************************************************************************************/
QString WifiWindow::spit_list(QString str,QString ssid)
{
    QStringList lines = str.split('\n');
    QString output;

    for (const QString& line : lines) {
        QStringList parts = line.split("\t", QString::SkipEmptyParts);
        if(parts.size() >= 4)
        {
            if(parts.at(1) == ssid)
            {
                output = parts.at(0); // Content obtained from parts
                break;
            }
            else
            {
                output = "";
            }
        }

    }
    return output;
}

void WifiWindow::on_pushButton_2_clicked()
{
    this->close();
}

//Refresh the wifi list
//wpa_cli -i wlan0 status
void WifiWindow::get_current_wifi()
{
    ui->listWidget_2->clear();
    //Show connected wifi
    QString list_network_output = process_create(wifi.wpa_cli,wifi.list_network);
   // qDebug() << "list_network_output:" << list_network_output;
   // qDebug() << "connect_result : "<< list_network_output;
    ui->listWidget_2->addItems(spit_connected(list_network_output));
    ui->listWidget_2->update();
}

//Display Wi-Fi connection status
void WifiWindow::get_wifi_sta()
{
    cur_ssid = "";
    cur_id = "";
    //Get connection status
    QString result = process_create(wifi.wpa_cli,wifi.status);
    //    qDebug() << "connect_status : "<< result;
    connect_sta  = result.contains("COMPLETED");
    //    qDebug() << "connect_sta : "<< connect_sta;

    //If wifi is connected, get ssid and id
    if(connect_sta == 1)
    {
        QStringList lines = result.split('\n');
        for (const QString& line : lines) {
            if(line.contains("ssid") && !line.contains("bssid"))
            {
                cur_ssid = line.section("=",1);
                //                    qDebug() << "wifi is connected : "<< output;
                //                    uishow(output);
            }
            if(line.contains("id") && !line.contains("ssid") && !line.contains("bssid") && !line.contains("uuid"))
            {
                cur_id = line.section("=",1);
                //                    qDebug() << "current id : " << cur_id;
            }
        }
    }
    else
    {
       // qDebug() << "wifi is no connected";
        //            ui->listWidget_2->clear();
        //            ui->listWidget_2->addItem("Wifi is not connected, please connect wifi!");
        //        ui->listWidget_2->setStyleSheet("QListWidget { background-color: #FF0000; }");
        //        ui->listWidget_2->update();

    }

}

void WifiWindow::uishow(bool sta)
{
    //    ui->listWidget_2->clear();
    //    ui->listWidget_2->addItem("wifi is connected : "+output);
    //    ui->listWidget_2->setStyleSheet("QListWidget { background-color: #00FF00; }");
    //    ui->listWidget_2->update();
    if(sta == 1){
        wifi_connect_disable(1);

    }
    else{
        wifi_connect_disable(0);
        QTimer::singleShot(1000, this, [this]()
        {
            get_current_wifi();
        });
        process_create(wifi.wpa_cli,wifi.save_config);
    }



}

//Unsave the Wi-Fi connection
//wpa_cli -i wlan0 remove_network 0
void WifiWindow::on_unsave_BT_clicked()
{
    QListWidgetItem *selectedItem = ui->listWidget_2->currentItem();

    if (selectedItem) {

        //Get ssid
        QString unsave_ssid = selectedItem->text().split("\t", QString::SkipEmptyParts).at(0);
        qDebug() << "select ssid" << unsave_ssid;

        //Cancel Save
        QStringList argu;
        argu << wifi.remove_network << unsave_ssid;
        QString unsave_result = process_create(wifi.wpa_cli,argu);
        qDebug() << "unsave_result : "<< unsave_result;

        //Get current wifi
        QTimer::singleShot(1000, this, [this]()
        {
            get_current_wifi();
        });
        process_create(wifi.wpa_cli,wifi.remove_network);

    }
}


void WifiWindow::wifi_connect_disable(bool sta)
{
    ui->search_BT->setDisabled(sta);
    ui->connect_BT->setDisabled(sta);
    ui->disconnect_BT->setDisabled(sta);
    ui->unsave_BT->setDisabled(sta);
}


void WifiWindow::on_listWidget_2_itemClicked(QListWidgetItem *item)
{
    ui->listWidget->clearSelection();
    ui->listWidget->setCurrentItem(NULL);

}

void WifiWindow::on_listWidget_itemClicked(QListWidgetItem *item)
{
    ui->listWidget_2->clearSelection();
    ui->listWidget_2->setCurrentItem(NULL);

}

void WifiWindow::wifi_init()
{
    if(isAlive("wpa_supplicant")){
        ui->enable_CB->setChecked(1);
        qDebug() << "wifi is alive";

        QTimer::singleShot(1000, this, [this]()
        {
            get_current_wifi();
        });
    }
    else
    {
        qDebug() << "wifi is not alive";
    }
}


void WifiWindow::on_enable_CB_stateChanged(int arg1)
{
    if (arg1 == Qt::Checked) {
        if(!isAlive("wpa_supplicant")){
            qDebug() << "wpa_supplicant run";
            process= new QProcess(this);
            process->start("/usr/sbin/wpa_supplicant -B  -i wlan0 -c /etc/wpa_supplicant.conf");
            enable_time = 1;
            button_disable(1);
            /* to decrease Btn Display Time */
            timer->start(10);
            //sh_new design to update current Network
            QTimer::singleShot(1000, this, [this]()
            {
                get_current_wifi();
            });
        }
    } else {
        process= new QProcess(this);
        process->start("killall wpa_supplicant");
        ui->listWidget->clear();
        ui->listWidget_2->clear();
        button_disable(1);
    }
}

void WifiWindow::on_exitBt_clicked()
{
    this->close();
}

void WifiWindow::button_disable(int sta)
{
    ui->search_BT->setDisabled(sta);
    ui->connect_BT->setDisabled(sta);
    ui->disconnect_BT->setDisabled(sta);
    ui->unsave_BT->setDisabled(sta);
}

void WifiWindow::on_exit_BT_clicked()
{
   // QApplication::quit();
    /* to Close the App immediately */
    qDebug()<< "exite right now !!!";
    QCoreApplication::exit(0);
    std::exit(0);
}




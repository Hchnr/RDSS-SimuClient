#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <stdio.h>
#include <string>
#include <string.h>
#include <vector>

#include <QMainWindow>
#include <QUdpSocket>

#include "inifile.h"

// #define INI_FILE_NAME "I:\\hcr-workspace\\workspace\\qt-workspace\\RDSSClient1\\build-RDSSClient-Desktop_Qt_5_5_1_MinGW_32bit-Debug\\debug\\config.ini"
#define INI_FILE_NAME ".\\config.ini"
#define BUFFER_LENGTH 1024
#define SNR_LEN 2

using namespace std;

enum POS {
    recv_dest_user=0,
    send_src_user=0,
    recv_src_user=4,
    send_dest_user=4,
    frame_id=8,
    option=10,
    stamp=12,
    ret=12,
    data=12,
    direction=16,
    longitude=17,
    latitude=21,
    altitude=25,
    cable=29,
    beam_length=30,
    snr=31
};

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    QUdpSocket m_udpsocket;
    string m_name;
    string m_channelIP;
    unsigned int m_userID;
    unsigned short m_frameNo;
    unsigned short m_channelPort;
    unsigned short m_clientPort;
    vector<unsigned int> m_peers;

    void Test();
    int Init();

    int readInitFile();
    int userOn();
    int userOnWait();
    int userMove();
    int userMoveWait();
    int userSend(bool isRet, bool isCnf, unsigned int id);
    int userSend(bool isRet, bool isCnf, unsigned int id, QString str);
    int userSend(bool isRet, bool isCnf, unsigned int id, char* buf, int len);
    int userRecv(char* buf, int iRev);
    int userRecvWait(unsigned int id);
    int userSendWaitRet(unsigned int id);
    int userSendWaitCnf(unsigned int id);

    /* to reuse old code */
    int htonl(int);
    int ntohl(int);
    short htons(short);
    short ntohs(short);

private slots:
    void recvMessage();
    void on_pushButton_move_clicked();
    void on_pushButton_send_clicked();
};

#endif // MAINWINDOW_H

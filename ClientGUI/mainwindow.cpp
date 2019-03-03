#include <time.h>

#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->Init();
}


int MainWindow::Init()
{
    /* default init config, better use Client::init() */
    m_userID = 1;
    m_name = "Client";
    // srand((unsigned)time(NULL));
    // m_frameNo = rand() % 0xffff;
    m_frameNo = 0x00ff;

    readInitFile();
    ui->lineEdit_stat_userid->setText(QString::number(m_userID));
    ui->lineEdit_stat_clientport->setText(QString::number(m_clientPort));
    ui->lineEdit_beamid->setText(QString::number(m_userID));

    if (!m_udpsocket.bind(m_clientPort))
    {
        printf("Client Init new UDP socket error!\n");
        return -1;
    }

    connect(&m_udpsocket, SIGNAL(readyRead()), this, SLOT(recvMessage()));

    return 0;
}

void MainWindow::recvMessage()
{
    if(!m_udpsocket.hasPendingDatagrams())
        return;

    char buf[BUFFER_LENGTH];
    QHostAddress *srcIP = new QHostAddress;
    unsigned short srcPort;

    int iRecv = m_udpsocket.readDatagram(buf, BUFFER_LENGTH, srcIP, &srcPort);
    if(iRecv < 0)
    {
        ui->textBrowser_stat->append("client recvfrom(): error when user send. " + iRecv);
        return;
    }

    ui->textBrowser_stat->append("[client recv packet from] " + srcIP->toString() + " " + QString::number(srcPort));
    char output[BUFFER_LENGTH];
    for(int i = 0; i < iRecv; i++)
        sprintf(output + i * 5, "0x%02X ", buf[i]);
    ui->textBrowser_stat->append(output);

    userRecv(buf, iRecv);

}

MainWindow::~MainWindow()
{
    delete ui;
}

int MainWindow::userMoveWait()
{
    char buf[BUFFER_LENGTH];
    QHostAddress *srcIP = new QHostAddress();
    unsigned short srcPort;

    while(1) {
        int iRecv = m_udpsocket.readDatagram(buf, BUFFER_LENGTH, srcIP, &srcPort);
        if(iRecv < 0)
        {
            printf("client recvfrom(): error %d when user move wait.\n", iRecv);
            continue;
        }

        printf("[client recv packet from %d %d:] \n", srcIP, srcPort);
        for(int i = 0; i < iRecv; i++)
            printf("0x%02X ", buf[i]);
        printf("\n");

        if( ntohl(*(int*) (buf+recv_dest_user)) != m_userID) {
            printf("client dest_user_id: %d \n", ntohl(* (int*) (buf+recv_dest_user)));
            continue;
        }
        if( ntohl(*(int*) (buf+recv_src_user)) != 0) {
            printf("client src_user_id: %d \n", ntohl(* (int*) (buf+recv_src_user)));
            continue;
        }
        /*
        if( ntohs(*(unsigned short*) (buf+frame_id)) != m_frameNo + 1) {
            printf("client frame_id: %d %d \n", m_frameNo, ntohs(* (unsigned short*) (buf+frame_id)));
            continue;
        }
        */
        if(*(buf+option) != 0x80) {
            printf("client option word: %d \n", (*(buf+option)));
            continue;
        }
        if(* (buf+ret) == 0x03) {
            printf("移动性报告成功！ \n");
            m_frameNo += 2;
            return 0;
        }

        break;
    }

    return -1;
}

int MainWindow::userMove()
{
    unsigned char buf[BUFFER_LENGTH] = {
    0x00, 0x00, 0x00, 0x01, /* 4 dest user */
    0x00, 0x00, 0x00, 0x00, /* 8 src user */
    0x00, 0xff,             /* 10 frame id */
    0x03, 0x00,             /* 12 OPTION: user on */
    0x00, 0x00, 0x00, 0x00, /* 16 time stamp */
    0x00,                   /* 17 E/W, S/N */
    0x00, 0x00, 0x00, 0x00, /* 21 longitude */
    0x00, 0x00, 0x00, 0x00, /* 25 latitude */
    0x00, 0x00, 0x00, 0x00, /* 29 altitude */
    0x01,                   /* 30 cable network */
    0x02,                   /* 31 beam length */
    0x00, 0x01,             /* 33 beam id */
    0x00, 0x00, 0x00, 0x00, /* 37 SNR */
    0x00, 0x02,             /* 39 beam id */
    0x00, 0x00, 0x00, 0x00, /* 43 SNR */
    };

    /* SET frame id*/
    unsigned short *pFrame = (unsigned short *) (buf + POS::frame_id);
    *pFrame = htons(m_frameNo);

    /* SET time stamp */
    time_t stamp = time(NULL);
    unsigned int *pStamp = ( unsigned int *) (buf + POS::stamp);
    *pStamp = (unsigned int) htonl(stamp);

    /* SET SNR */
    for(int i = 0; i < SNR_LEN; i++) {
        int *pSNR = (int *) (buf + i * 6 + POS::snr + 2);
        /* float(1.0): [signal, exponent, fraction]=[0, 127, 0]=[3f 80 00 00] */
        int *tmp = (int *) new float(2.0);
        // *pSNR = htonl(*tmp);
        *pSNR = *tmp;
    }

    int packet_len = POS::snr + 6 * SNR_LEN;
    printf("[client send packet to: %s %d]\n", m_channelIP.data(), m_channelPort);
    for(int i = 0; i < packet_len; i++)
         printf("0x%02X ", buf[i]);
    printf("\n");

    int iSend = m_udpsocket.writeDatagram((char *) buf, packet_len, QHostAddress(QString(m_channelIP.data())), m_channelPort);
    if (iSend < 0)
    {
        printf("client sendto(): error %d when user move.\n", iSend);
        return -1;
    }

    userOnWait();
    return 0;

}
int MainWindow::userRecvWait(unsigned int id)
{
    unsigned char buf[BUFFER_LENGTH] = {
    0x00, 0x00, 0x00, 0x01, /* 4 dest user */
    0x00, 0x00, 0x00, 0x00, /* 8 src user */
    0x00, 0xff,             /* 10 frame id */
    0x80, 0x00,             /* 12 OPTION */
    0x00,                   /* 13 OPTION kind */
    0x00,                   /* 14 OPTION hint */
    };

    /* SET dest id id*/
    unsigned int *pDest = (unsigned int *) (buf + POS::send_dest_user);
    *pDest = htonl(id);

    /* SET src id id*/
    unsigned int *pSrc = (unsigned int *) (buf + POS::send_src_user);
    *pSrc = htonl(m_userID);

    /* SET frame id*/
    unsigned short *pFrame = (unsigned short *) (buf + POS::frame_id);
    *pFrame = htons(m_frameNo);

    /* SET option */
    unsigned char *pOpt = (unsigned char *) buf + POS::option;
    *pOpt = 0x80;

    /* SET option kind */
    unsigned char *pKind = (unsigned char *) buf + POS::option + 2;
    *pKind = 0x60;

    /* print send data */
    int packet_len = 2 + POS::data;
    ui->textBrowser_stat->append("[client send packet to] " + QString(m_channelIP.data()) + " " + QString::number(m_channelPort));
    char output[BUFFER_LENGTH];
    for(int i = 0; i < packet_len; i++)
        sprintf(output + i * 5, "0x%02X ", buf[i]);
    ui->textBrowser_stat->append(output);
    ui->textBrowser_stat->repaint();

    int iSend = m_udpsocket.writeDatagram((char *) buf, packet_len, QHostAddress(QString(m_channelIP.data())), m_channelPort);
    if (iSend < 0)
    {
        printf("client sendto(): error %d when user send recv cnf.\n", iSend);
        return -1;
    }

}

int MainWindow::userRecv(char *buf, int iRecv)
{
    string srcIP;
    unsigned short srcPort;

    if( ntohl(*(int*) (buf+recv_dest_user)) != m_userID) {
        printf("client recv dest_user_id: %d \n", ntohl(* (int*) (buf+recv_dest_user)));
        return -1;
    }
    unsigned int id =  ntohl(*(int*) (buf+recv_src_user));
    /*
    if( ntohs(*(unsigned short*) (buf+frame_id)) != m_frameNo + 1) {
        printf("client frame_id: %d %d \n", m_frameNo, ntohs(* (unsigned short*) (buf+frame_id)));
        continue;
    }
    */
    switch(*(buf+option))
    {
        case 0x40:
        case 0x50:
        case 0x51:
        case 0x60: ui->textBrowser_recv->append("收到来自#" + QString::number(id) + "的短消息：");
                   char output[BUFFER_LENGTH];
                   for(int i = POS::data; i < iRecv; i++)
                       sprintf(output + (i - POS::data) * 5, "0x%02X ", buf[i]);
                   ui->textBrowser_recv->append(QString(output));
    }
    if( (unsigned char) *(buf + option) == 0x60) {
        userRecvWait(id);
        return 1;
    }

    return 1;
}

int MainWindow::userSendWaitCnf(unsigned int id)
{
    char buf[BUFFER_LENGTH];
    QHostAddress *srcIP = new QHostAddress;
    unsigned short srcPort;

    while(1) {
        m_udpsocket.waitForReadyRead();
        int iRecv = m_udpsocket.readDatagram(buf, BUFFER_LENGTH, srcIP, &srcPort);
        if(iRecv < 0)
        {
            printf("client recvfrom(): error %d when user send.\n", iRecv);
            continue;
        }

        printf("[client recv packet from %d %d:] \n", srcIP, srcPort);
        for(int i = 0; i < iRecv; i++)
            printf("0x%02X ", buf[i]);
        printf("\n");

        if( ntohl(*(int*) (buf+recv_dest_user)) != m_userID) {
            printf("client dest_user_id: %d \n", ntohl(* (int*) (buf+recv_dest_user)));
            continue;
        }
        if( ntohl(*(int*) (buf+recv_src_user)) != id) {
            printf("client src_user_id: %d \n", ntohl(* (int*) (buf+recv_src_user)));
            continue;
        }
        /*
        if( ntohs(*(unsigned short*) (buf+frame_id)) != m_frameNo + 1) {
            printf("client frame_id: %d %d \n", m_frameNo, ntohs(* (unsigned short*) (buf+frame_id)));
            continue;
        }
        */
        if( (unsigned char) *(buf+option) != 0x80) {
            printf("client option word: %d \n", (*(buf+option)));
            continue;
        }

        /*
        if( ntohs(*(unsigned short*) (buf + POS::data + 4 + i*8)) != m_frameNo+1)
            continue;
        */
        if( (unsigned char) *(buf + POS::data ) != 0x60 )
            continue;

        ui->textBrowser_recv->append("短消息发送成功！(用户已确认) \n");
        return 0;
    }

    return 1;
}

int MainWindow::userSendWaitRet(unsigned int id)
{
    char buf[BUFFER_LENGTH];
    QHostAddress *srcIP = new QHostAddress();
    unsigned short srcPort;

    while(1) {
        m_udpsocket.waitForReadyRead();
        int iRecv = m_udpsocket.readDatagram(buf, BUFFER_LENGTH, srcIP, &srcPort);
        if(iRecv < 0)
        {
            printf("client recvfrom(): error %d when user send.\n", iRecv);
            continue;
        }

        printf("[client recv packet from %d %d:] \n", srcIP, srcPort);
        for(int i = 0; i < iRecv; i++)
            printf("0x%02X ", buf[i]);
        printf("\n");

        if( ntohl(*(int*) (buf+recv_dest_user)) != m_userID) {
            printf("client dest_user_id: %d \n", ntohl(* (int*) (buf+recv_dest_user)));
            continue;
        }
        if( ntohl(*(int*) (buf+recv_src_user)) != 0) {
            printf("client src_user_id: %d \n", ntohl(* (int*) (buf+recv_src_user)));
            continue;
        }
        /*
        if( ntohs(*(unsigned short*) (buf+frame_id)) != m_frameNo + 1) {
            printf("client frame_id: %d %d \n", m_frameNo, ntohs(* (unsigned short*) (buf+frame_id)));
            continue;
        }
        */
        if( (unsigned char) *(buf+option) != 0x80) {
            printf("client option word: %d \n", (*(buf+option)));
            continue;
        }

        /*
        if( ntohs(*(unsigned short*) (buf + POS::data + 4 + i*8)) != m_frameNo+1)
            continue;
        */
        if( (unsigned char) *(buf + POS::data ) != 0x50 )
            continue;

        ui->textBrowser_recv->append("短消息发送成功！(系统回执) \n");
        ui->textBrowser_recv->repaint();
        return 0;
    }

    return 1;
}

int MainWindow::userSend(bool isRet, bool isCnf, unsigned int id, QString str)
{
    unsigned char buf[BUFFER_LENGTH] = {
    0x00, 0x00, 0x00, 0x01, /* 4 dest user */
    0x00, 0x00, 0x00, 0x00, /* 8 src user */
    0x00, 0xff,             /* 10 frame id */
    0x40, 0x00,             /* 12 OPTION */
    0x00, 0x00, 0x00, 0x00, /* 16 message */
    };

    /* SET message data */
    strncpy((char*) buf + POS::data, str.toLatin1(), str.length());

    /* SET dest id id*/
    unsigned int *pDest = (unsigned int *) (buf + send_dest_user);
    *pDest = htonl(id);

    /* SET src id id*/
    unsigned int *pSrc = (unsigned int *) (buf + POS::send_src_user);
    *pSrc = htonl(m_userID);

    /* SET frame id*/
    unsigned short *pFrame = (unsigned short *) (buf + POS::frame_id);
    *pFrame = htons(m_frameNo);

    /* SET option */
    unsigned char *pOpt = ( unsigned char *) buf + POS::option;
    if(!isRet && !isCnf)
        *pOpt = 0x40;
    else if(isRet && !isCnf)
        *pOpt = 0x50;
    else if(isRet && isCnf)
        *pOpt = 0x60;

    int packet_len = str.length() + POS::data;
    int iSend = m_udpsocket.writeDatagram((char *) buf, packet_len, QHostAddress(QString(m_channelIP.data())), m_channelPort);
    if (iSend < 0)
    {
        printf("client sendto(): error %d when user send.\n", iSend);
        return -1;
    }

    /* print send data */
    ui->textBrowser_stat->append("[client send packet to] " + QString(m_channelIP.data()) + " " + QString::number(m_channelPort));
    char output[BUFFER_LENGTH];
    for(int i = 0; i < packet_len; i++)
        sprintf(output + i * 5, "0x%02X ", buf[i]);
    ui->textBrowser_stat->append(output);
    ui->textBrowser_stat->repaint();

    /* DISABLE slot to block confirm process */
    disconnect(&m_udpsocket, SIGNAL(readyRead()), this, SLOT(recvMessage()));
    if(isRet) {
        userSendWaitRet(id);
    }
    if(isCnf) {
        userSendWaitCnf(id);
    }
    connect(&m_udpsocket, SIGNAL(readyRead()), this, SLOT(recvMessage()));

    return 0;
}

int MainWindow::userSend(bool isRet, bool isCnf, unsigned int id)
{
    unsigned char buf[BUFFER_LENGTH] = {
    0x00, 0x00, 0x00, 0x01, /* 4 dest user */
    0x00, 0x00, 0x00, 0x00, /* 8 src user */
    0x00, 0xff,             /* 10 frame id */
    0x40, 0x00,             /* 12 OPTION */
    0x00, 0x00, 0x00, 0x00, /* 16 message */
    };

    /* SET dest id id*/
    unsigned int *pDest = (unsigned int *) (buf + send_dest_user);
    *pDest = htonl(id);

    /* SET src id id*/
    unsigned int *pSrc = (unsigned int *) (buf + POS::send_src_user);
    *pSrc = htonl(m_userID);

    /* SET frame id*/
    unsigned short *pFrame = (unsigned short *) (buf + POS::frame_id);
    *pFrame = htons(m_frameNo);

    /* SET option */
    unsigned char *pOpt = ( unsigned char *) buf + POS::option;
    if(!isRet && !isCnf)
        *pOpt = 0x40;
    else if(isRet && !isCnf)
        *pOpt = 0x50;
    else if(isRet && isCnf)
        *pOpt = 0x60;

    string msg = "test:";
    msg += std::to_string(id);
    msg += "->";
    msg += std::to_string(m_userID);
    memcpy(buf + POS::data, msg.data(), msg.length());

    int packet_len = msg.length() + POS::data;
    int iSend = m_udpsocket.writeDatagram((char *) buf, packet_len, QHostAddress(QString(m_channelIP.data())), m_channelPort);
    if (iSend < 0)
    {
        printf("client sendto(): error %d when user send.\n", iSend);
        return -1;
    }

    /* print send data */
    ui->textBrowser_stat->append("[client send packet to] " + QString(m_channelIP.data()) + " " + QString::number(m_channelPort));
    char output[BUFFER_LENGTH];
    for(int i = 0; i < packet_len; i++)
        sprintf(output + i * 5, "0x%02X ", buf[i]);
    ui->textBrowser_stat->append(output);
    ui->textBrowser_stat->repaint();

    /* DISABLE slot to block confirm process */
    disconnect(&m_udpsocket, SIGNAL(readyRead()), this, SLOT(recvMessage()));
    if(isRet) {
        userSendWaitRet(id);
    }
    if(isCnf) {
        userSendWaitCnf(id);
    }
    connect(&m_udpsocket, SIGNAL(readyRead()), this, SLOT(recvMessage()));

    return 0;
}

int MainWindow::userOnWait()
{
    char buf[BUFFER_LENGTH];
    QHostAddress *srcIP = new QHostAddress();
    unsigned short srcPort;

    m_udpsocket.disconnect();
    while(1) {

        m_udpsocket.waitForReadyRead(600000);
        int iRecv = m_udpsocket.readDatagram(buf, BUFFER_LENGTH, srcIP, &srcPort);
        if(iRecv < 0)
        {
            printf("client recvfrom(): error %d when user on.\n", iRecv);
            continue;
        }

        ui->textBrowser_stat->append("[client recv useron packet]\n" + srcIP->toString() + QString::number(srcPort));
        char tmp[BUFFER_LENGTH];
        for(int i = 0; i < iRecv; i++)
             sprintf(tmp + i*5, "0x%02X ", buf[i]);
        ui->textBrowser_stat->append(tmp);
        QApplication::processEvents();

        if( ntohl(*(int*) (buf+recv_dest_user)) != m_userID) {
            printf("clinet dest_user_id: %d \n", ntohl(* (int*) (buf+recv_dest_user)));
            continue;
        }
        if( ntohl(*(int*) (buf+recv_src_user)) != 0) {
            printf("client src_user_id: %d \n", ntohl(* (int*) (buf+recv_src_user)));
            continue;
        }
        /*
        if( ntohs(*(unsigned short*) (buf+frame_id)) != m_frameNo + 1) {
            printf("frame_id: %d %d \n", m_frameNo, ntohs(* (unsigned short*) (buf+frame_id)));
            continue;
        }
        */
        if((unsigned char) *(buf+option) != 0x80) {
            printf("client option word: %d \n", (*(buf+option)));
            continue;
        }

        connect(&m_udpsocket, SIGNAL(readyRead()), this, SLOT(recvMessage()));
        break;
    }

    if(* (buf+ret) == 0x00) {
        printf("开机成功！ \n");
        printf("未接受短消息： %d条。 \n", * (buf+ret+1));
        m_frameNo += 2;
        return 0;
    }
    else if(* (buf+ret) == 0x01) {
        printf("开机失败！ \n");
        return 1;
    }

    return -1;
}

int MainWindow::userOn()
{
    unsigned char buf[BUFFER_LENGTH] = {
    0x00, 0x00, 0x00, 0x01, /* 4 dest user */
    0x00, 0x00, 0x00, 0x00, /* 8 src user */
    0x00, 0xff,             /* 10 frame id */
    0x00, 0x00,             /* 12 OPTION: user on */
    0x00, 0x00, 0x00, 0x00, /* 16 time stamp */
    0x00,                   /* 17 E/W, S/N */
    0x00, 0x00, 0x00, 0x00, /* 21 longitude */
    0x00, 0x00, 0x00, 0x00, /* 25 latitude */
    0x00, 0x00, 0x00, 0x00, /* 29 altitude */
    0x01,                   /* 30 cable network */
    0x02,                   /* 31 beam length */
    0x00, 0x01,             /* 33 beam id */
    0x00, 0x00, 0x00, 0x00, /* 37 SNR */
    0x00, 0x02,             /* 39 beam id */
    0x00, 0x00, 0x00, 0x00, /* 43 SNR */
    };

    /* SET user id*/
    unsigned int *pID = (unsigned int *) (buf + POS::send_src_user);
    *pID = htonl(m_userID);

    /* SET frame id*/
    unsigned short *pFrame = (unsigned short *) (buf + POS::frame_id);
    *pFrame = htons(m_frameNo);

    /* SET time stamp */
    time_t stamp = time(NULL);
    unsigned int *pStamp = ( unsigned int *) (buf + POS::stamp);
    *pStamp = (unsigned int) htonl(stamp);

    /* SET SNR */
    for(int i = 0; i < SNR_LEN; i++) {
        int *pSNR = (int *) (buf + i * 6 + POS::snr + 2);
        /* float(1.0): [signal, exponent, fraction]=[0, 127, 0]=[3f 80 00 00] */
        int *tmp = (int *) new float(1.0);
        // *pSNR = htonl(*tmp);
        *pSNR = *tmp;
    }

    int packet_len = POS::snr + 6 * SNR_LEN;
    ui->textBrowser_stat->append("[client send packet]\n" + QString(m_channelIP.data()) + QString::number(m_channelPort));
    char tmp[BUFFER_LENGTH];
    for(int i = 0; i < packet_len; i++)
         sprintf(tmp + i*5, "0x%02X ", buf[i]);
    ui->textBrowser_stat->append(tmp);
    QApplication::processEvents();

    int iSend = m_udpsocket.writeDatagram((char *) buf, packet_len, QHostAddress(QString(m_channelIP.data())), m_channelPort);
    if (iSend < 0)
    {
        printf("client sendto(): error %d when user on.\n", iSend);
        return -1;
    }

    userOnWait();
    return 0;
}

void MainWindow::Test()
{
    printf("client Channel IP: %s\n", m_channelIP.data());
    printf("client Channle Port: %d\n", m_channelPort);
    printf("client Client Port: %d\n", m_clientPort);
}

int MainWindow::htonl(int n)
{
    int ans = n;
    unsigned char *p = (unsigned char *) &ans;
    unsigned char tmp = p[0];
    p[0] = p[3];
    p[3] = tmp;
    tmp = p[1];
    p[1] = p[2];
    p[2] = tmp;
    return ans;
}

int MainWindow::ntohl(int n)
{
    return htonl(n);
}

short MainWindow::htons(short n)
{
    short ans = n;
    unsigned char *p = (unsigned char *) &ans;
    unsigned char tmp = p[0];
    p[0] = p[1];
    p[1] = tmp;
    return ans;
}

short MainWindow::ntohs(short n)
{
    return htons(n);
}

int MainWindow::readInitFile()
{
    char buf[BUFFER_LENGTH];

    if (!read_profile_string("Client", "name", buf, 16, "client_default", INI_FILE_NAME))
    {
        printf("%s: Read Client name from %s failed!\n", m_name.data(), INI_FILE_NAME);
        ui->textBrowser_stat->append("Read config.ini failed!\n");
        return -1;
    }
    m_name = buf;

    /* READ server */
    m_channelPort = (unsigned short) read_profile_int("Channel", "Port", 0, INI_FILE_NAME);
    if (m_channelPort == 0)
    {
        printf("%s: Read Channel Port from %s failed!\n", m_name.data(), INI_FILE_NAME);
        return -1;
    }

    if (!read_profile_string("Channel", "IP", buf, 16, "127.0.0.1", INI_FILE_NAME))
    {
        printf("%s: Read Channel IP from %s failed!\n", m_name.data(), INI_FILE_NAME);
        return -1;
    }
    m_channelIP = buf;

    m_clientPort = (unsigned short) read_profile_int("Client", "Port", 0, INI_FILE_NAME);
    if (m_clientPort == 0)
    {
        printf("%s: Read Client port from %s failed!\n", m_name.data(), INI_FILE_NAME);
        return -1;
    }

    m_userID = (unsigned int) read_profile_int("Client", "ID", 0, INI_FILE_NAME);
    if (m_userID == 0)
    {
        printf("%s: Read Client ID from %s failed!\n", m_name.data(), INI_FILE_NAME);
        return -1;
    }

    int n = read_profile_int("Client", "Length", 0, INI_FILE_NAME);
    if (n == 0)
    {
        printf("%s: Read Client peer num from %s failed!\n", m_name.data(), INI_FILE_NAME);
        return -1;
    }

    for (int i = 0; i < n; i ++) {
        string sector = "Peer" + std::to_string(i);
        int peer = (unsigned int) read_profile_int(sector.data(), "ID", 0xffffff, INI_FILE_NAME);
        if (peer == 0xffffff)
        {
            printf("%s: Read Peer ID from %s failed!\n", m_name.data(), INI_FILE_NAME);
            return -1;
        }
        m_peers.push_back(peer);
    }

    return 0;
}

void MainWindow::on_pushButton_move_clicked()
{
    userOn();
}

void MainWindow::on_pushButton_send_clicked()
{
    unsigned int id = ui->lineEdit_send_userid->text().toInt();
    bool isRet = 0, isCnf = 0;
    QString str = ui->textEdit_send->toPlainText();

    if(ui->radioButton_5->isChecked()) {
        isRet = false;
        isCnf = false;
    }
    else if (ui->radioButton_6->isChecked()) {
        isRet = true;
        isCnf = false;
    }
    else if (ui->radioButton_7->isChecked()) {
        isRet = true;
        isCnf = true;
    }
    if(str == "发送内容" || str == "")
        userSend(isRet, isCnf, id);
    else
        userSend(isRet, isCnf, id, str);
}

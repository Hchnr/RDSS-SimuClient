#include "mainwindow.h"
#include "ui_mainwindow.h"

#define TABNUM 5

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_channel = new ChannelUnitForward();
    connect(this, SIGNAL(changeClients(vector<string>& clientIPs, vector<unsigned short> clientPorts)),
            m_channel->m_thread, SLOT(changeClients(vector<string>& clientIPs, vector<unsigned short> clientPorts)));
    m_channel->Init();
    m_channel->Test();
    m_channel->Run();

    ui->lineEdit_name->setText(m_channel->m_name.data());
    ui->lineEdit_name->setReadOnly(true);
    ui->lineEdit_clientNum->setText(QString::number(m_channel->m_clientIPs.size()));
    ui->lineEdit_ip->setText(m_channel->m_serverIP.data());
    ui->lineEdit_port->setText(QString::number(m_channel->m_serverForwardPort));
    /*
    for(int i = 0; i < TABNUM; i ++)
        ui->tabWidget->setTabEnabled(i, i < m_channel->m_clientIPs.size());
        */
    ui->lineEdit_ip0->setText(m_channel->m_clientIPs[0].data());
    ui->lineEdit_ip1->setText(m_channel->m_clientIPs[1].data());
    ui->lineEdit_port0->setText(QString::number(m_channel->m_clientPorts[0]));
    ui->lineEdit_port1->setText(QString::number(m_channel->m_clientPorts[1]));
    ui->tab->setFocus();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    int num = ui->lineEdit_clientNum->text().toInt();
    if(num <= 0 || num > 5)
        return;
    m_channel->m_clientIPs.clear();
    m_channel->m_clientPorts.clear();
    switch (num) {
    case 5: m_channel->m_clientPorts.push_back(ui->lineEdit_port4->text().toInt());
            m_channel->m_clientIPs.push_back(ui->lineEdit_ip4->text().toStdString());
    case 4: m_channel->m_clientPorts.push_back(ui->lineEdit_port3->text().toInt());
            m_channel->m_clientIPs.push_back(ui->lineEdit_ip3->text().toStdString());
    case 3: m_channel->m_clientPorts.push_back(ui->lineEdit_port2->text().toInt());
            m_channel->m_clientIPs.push_back(ui->lineEdit_ip2->text().toStdString());
    case 2: m_channel->m_clientPorts.push_back(ui->lineEdit_port1->text().toInt());
            m_channel->m_clientIPs.push_back(ui->lineEdit_ip1->text().toStdString());
    case 1: m_channel->m_clientPorts.push_back(ui->lineEdit_port0->text().toInt());
            m_channel->m_clientIPs.push_back(ui->lineEdit_ip0->text().toStdString());
    default:
        break;
    }
    qDebug() << m_channel->m_clientIPs[0].data();
    qDebug() << m_channel->m_clientPorts[0];
    emit changeClients(m_channel->m_clientIPs, m_channel->m_clientPorts);
}

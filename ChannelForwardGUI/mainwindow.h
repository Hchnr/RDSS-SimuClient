#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtDebug>

#include "ChannelUnitForward.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
signals:
    void changeClients(vector<string>& clientIPs, vector<unsigned short> clientPorts);

private slots:
    void on_pushButton_clicked();

private:
    Ui::MainWindow *ui;
    ChannelUnitForward *m_channel;
};

#endif // MAINWINDOW_H

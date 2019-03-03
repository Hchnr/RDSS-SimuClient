#include <stdio.h>
#include <string>
#include <vector>

#include <QThread>

#include "inifile.h"
#include "UDPInterface.h"
#include "tcp_socket.h"

#define INI_FILE_NAME "./config.ini"

class ChannelThread;

class ChannelUnitForward
{
public:
    int Init();
	int Run();
    int run_blocked();
    void run() { run_blocked(); }
	void Test();
	ChannelUnitForward();
	string m_name;
	string m_serverIP;
	unsigned short m_serverBackwardPort;
	unsigned short m_serverForwardPort;
	unsigned short m_clientPort;
	vector<string> m_clientIPs;
	vector<unsigned short> m_clientPorts;
    ChannelThread* m_thread;
 private:
    CUDPInterface m_UDPInterface;
    TcpSocket m_TcpSocket;

	int readInitFile();
};

class ChannelThread: public QThread{
    ChannelUnitForward* m_channel;
public:
    ChannelThread(ChannelUnitForward* p) { m_channel = p; }
    void run() { m_channel->run(); }
public slots:
    void changeClients(vector<string>& clientIPs, vector<unsigned short> clientPorts);
};

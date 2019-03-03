#include <stdio.h>
#include <string>
#include <vector>

#include "inifile.h"
#include "UDPInterface.h"
#include "tcp_socket.h"

#define INI_FILE_NAME "./config.ini"

class ChannelUnitBackward
{
  public:
    int Init();
	int Run();
	void Test();
	ChannelUnitBackward();
  private:
	string m_name;
	string m_serverIP;
	unsigned short m_serverBackwardPort;
	unsigned short m_serverForwardPort;
	unsigned short m_clientPort;
	vector<unsigned short> m_clientPorts;
	vector<string> m_clientIPs;
  CUDPInterface m_UDPInterface;
  TcpSocket m_TcpSocket;

	int readInitFile();
};

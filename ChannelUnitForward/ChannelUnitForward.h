#include <stdio.h>
#include <string>
#include <vector>

#include "inifile.h"
#include "UDPInterface.h"

#define INI_FILE_NAME "./config.ini"

class ChannelUnitForward
{
  public:
    int Init();
	int Run();
	void Test();
	ChannelUnitForward();
  private:
	string m_name;
	string m_serverIP;
	unsigned short m_serverPort;
	unsigned short m_clientPort;
	vector<string> m_clientIPs;
	vector<unsigned short> m_clientPorts;
    CUDPInterface m_UDPInterface;

	int readInitFile();
};

#include <stdio.h>
#include <string>
#include <vector>

#include "inifile.h"
#include "UDPInterface.h"

#define INI_FILE_NAME "./config.ini"

class Client
{
  public:
    int Init();
	int Run();
	void Test();
	Client();
  private:
	string m_name;
	string m_channelIP;
	unsigned short m_frameNo;
	unsigned short m_channelPort;
	unsigned short m_clientPort;
    CUDPInterface m_UDPInterface;

	int readInitFile();
	int userOn();
};

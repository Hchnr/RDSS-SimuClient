#include <stdio.h>
#include <string>
#include <vector>

#include "inifile.h"
#include "UDPInterface.h"

#define INI_FILE_NAME "./config.ini"

enum POS {
    dest_user=0,
    src_user=4,
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
	unsigned int m_userID;
	unsigned short m_frameNo;
	unsigned short m_channelPort;
	unsigned short m_clientPort;
	vector<unsigned int> m_peers;
    CUDPInterface m_UDPInterface;

	int readInitFile();
	int userOn();
	int userOnWait();
	int userMove();
	int userMoveWait();
	int userSend(bool isRet, bool isCnf, unsigned int id);
	int userRecv();
	int userRecvWait(unsigned int id, unsigned short frame);
	int userSendWaitRet(unsigned int id);
	int userSendWaitCnf(unsigned int id);
};




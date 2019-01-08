#include <stdio.h>
#include <string>
#include <string.h>
#include <vector>

#include "inifile.h"
#include "udp_socket.h"
#include "epoll_wrapper.h"

#define INI_FILE_NAME "./config.ini"

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
	unsigned int m_channelIPint;
	unsigned int m_userID;
	unsigned short m_frameNo;
	unsigned short m_channelPort;
	unsigned short m_clientPort;
	vector<unsigned int> m_peers;

    UdpSocket m_udpsocket;
	Epoll m_epoll;


	int readInitFile();
	int userOn();
	int userOnWait();
	int userMove();
	int userMoveWait();
	int userSend(bool isRet, bool isCnf, unsigned int id);
	int userRecv(char* buf, int iRev);
	int userRecvWait(unsigned int id);
	int userSendWaitRet(unsigned int id);
	int userSendWaitCnf(unsigned int id);
	void handleEvent(int socket_fd);
};




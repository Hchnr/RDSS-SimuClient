#include <netdb.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "Client.h"

#define INI_FILE_NAME "./config.ini"
#define BUFFER_LENGTH 1024

#define SNR_LEN 2

int Client::Run()
{
    userOn();
	/* need central station return, need peer confirm */
	// userSend(true, true, 2);
    userRecv();
}

int Client::userMoveWait()
{
    unsigned char buf[BUFFER_LENGTH];
    string srcIP;
	unsigned short srcPort;

    while(1) {
        int iRecv = m_UDPInterface.RecvFrom(buf, BUFFER_LENGTH, srcIP, srcPort);
    	if(iRecv < 0)
    	{
    	    printf("recvfrom(): error %d when user on.\n", iRecv);
    		continue;
        }

		printf("[recv packet from %s %d:] \n", srcIP.data(), srcPort);
	    for(int i = 0; i < iRecv; i++)
	        printf("0x%02X ", buf[i]);
        printf("\n");

		if( ntohl(*(int*) (buf+dest_user)) != m_userID) {
			printf("dest_user_id: %d \n", ntohl(* (int*) (buf+dest_user)));
		    continue;
		}
		if( ntohl(*(int*) (buf+src_user)) != 0) {
			printf("src_user_id: %d \n", ntohl(* (int*) (buf+src_user)));
		    continue;
		}
		/*
		if( ntohs(*(unsigned short*) (buf+frame_id)) != m_frameNo + 1) {
			printf("frame_id: %d %d \n", m_frameNo, ntohs(* (unsigned short*) (buf+frame_id)));
		    continue;
		}
		*/
		if(*(buf+option) != 0x80) {
			printf("option word: %d \n", (*(buf+option)));
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

int Client::userMove()
{
    unsigned char buf[BUFFER_LENGTH] = {
	0x00, 0x00, 0x00, 0x00, /* 4 dest user */ 
	0x00, 0x00, 0x00, 0x01, /* 8 src user */
	0x00, 0xff,             /* 10 frame id */
    0x03, 0x00,             /* 12 OPTION: user on */	
	0x00, 0x00, 0x00, 0x00, /* 16 time stamp */
	0x00,                   /* 17 E/W, S/N */
	0x00, 0x00, 0x00, 0x00, /* 21 longitude */
	0x00, 0x00, 0x00, 0x00, /* 25 latitude */
	0x00, 0x00, 0x00, 0x00, /* 29 altitude */
	0x01,                   /* 30 cable network */
	0x02,                   /* 31 beam length */
	0x00, 0x00,             /* 33 beam id */
	0x00, 0x00, 0x00, 0x00, /* 37 SNR */
	0x00, 0x01,             /* 39 beam id */
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
		int *tmp = (int *) new float(1.0);
        *pSNR = htonl(*tmp);
	}

	int packet_len = POS::snr + 6 * SNR_LEN;  
    printf("[send packet to: %s %d]\n", m_channelIP.data(), m_channelPort);
	for(int i = 0; i < packet_len; i++)
	     printf("0x%02X ", buf[i]);
	printf("\n");

    int iSend = m_UDPInterface.SendTo(buf, packet_len, m_channelIP, m_channelPort);
	if (iSend < 0)
	{
	    printf("sendto(): error %d when user on.\n", iSend);
		return -1;
	}
    
	userOnWait();
    return 0;

}
int Client::userRecvWait(unsigned int id)
{
    unsigned char buf[BUFFER_LENGTH] = {
    0x00, 0x00, 0x00, 0x00, /* 4 dest user */
    0x00, 0x00, 0x00, 0x01, /* 8 src user */
    0x00, 0xff,             /* 10 frame id */
    0x80, 0x00,             /* 12 OPTION */
    0x00,                   /* 13 OPTION kind */
    0x00,                   /* 14 OPTION hint */
    };

    /* SET dest id id*/
    unsigned int *pDest = (unsigned int *) (buf + POS::dest_user);
    *pDest = htonl(id);

    /* SET src id id*/
    unsigned int *pSrc = (unsigned int *) (buf + POS::src_user);
    *pSrc = htonl(m_userID);

    /* SET frame id*/
    unsigned short *pFrame = (unsigned short *) (buf + POS::frame_id);
    *pFrame = htons(m_frameNo);

    /* SET option */
    unsigned char *pOpt = buf + POS::option;
    *pOpt = 0x80;

    /* SET option kind */
    unsigned char *pKind = buf + POS::option + 2;
    *pKind = 0x60;

    int packet_len = 2 + POS::data;
    printf("[send packet to: %s %d]\n", m_channelIP.data(), m_channelPort);
    for(int i = 0; i < packet_len; i++)
         printf("0x%02X ", buf[i]);
    printf("\n");

    int iSend = m_UDPInterface.SendTo(buf, packet_len, m_channelIP, m_channelPort);
    if (iSend < 0)
    {
        printf("sendto(): error %d when user recv.\n", iSend);
        return -1;
    }

}
 
int Client::userRecv()
{
    unsigned char buf[BUFFER_LENGTH];
    string srcIP;
	unsigned short srcPort;
	int nRet;

    while(1) {
        int iRecv = m_UDPInterface.RecvFrom(buf, BUFFER_LENGTH, srcIP, srcPort);
    	if(iRecv < 0)
    	{
    	    printf("recvfrom(): error %d when user send.\n", iRecv);
    		continue;
        }

		printf("[recv packet from %s %d:] \n", srcIP.data(), srcPort);
	    for(int i = 0; i < iRecv; i++)
	        printf("0x%02X ", buf[i]);
        printf("\n");

		if( ntohl(*(int*) (buf+dest_user)) != m_userID) {
			printf("dest_user_id: %d \n", ntohl(* (int*) (buf+dest_user)));
		    continue;
		}
		unsigned int id =  ntohl(*(int*) (buf+src_user));
		/*
		if( ntohs(*(unsigned short*) (buf+frame_id)) != m_frameNo + 1) {
			printf("frame_id: %d %d \n", m_frameNo, ntohs(* (unsigned short*) (buf+frame_id)));
		    continue;
		}
		*/
		switch(*(buf+option))
		{
		    case 0x40:
			case 0x50:
			case 0x51:
			case 0x60: printf("收到短消息：");
					   for(int i = POS::data; i < iRecv; i++)
						   printf("0x%02X ", buf[i]);
					   printf("\n");
					   break;
		}
		if(*(buf + option) == 0x60) {
		    userRecvWait(id);
		    continue;
		}

	}

    return 1;
}

int Client::userSendWaitCnf(unsigned int id)
{
    unsigned char buf[BUFFER_LENGTH];
    string srcIP;
	unsigned short srcPort;
	int nRet;

    while(1) {
        int iRecv = m_UDPInterface.RecvFrom(buf, BUFFER_LENGTH, srcIP, srcPort);
    	if(iRecv < 0)
    	{
    	    printf("recvfrom(): error %d when user send.\n", iRecv);
    		continue;
        }

		printf("[recv packet from %s %d:] \n", srcIP.data(), srcPort);
	    for(int i = 0; i < iRecv; i++)
	        printf("0x%02X ", buf[i]);
        printf("\n");

		if( ntohl(*(int*) (buf+dest_user)) != m_userID) {
			printf("dest_user_id: %d \n", ntohl(* (int*) (buf+dest_user)));
		    continue;
		}
		if( ntohl(*(int*) (buf+src_user)) != id) {
			printf("src_user_id: %d \n", ntohl(* (int*) (buf+src_user)));
		    continue;
		}
		/*
		if( ntohs(*(unsigned short*) (buf+frame_id)) != m_frameNo + 1) {
			printf("frame_id: %d %d \n", m_frameNo, ntohs(* (unsigned short*) (buf+frame_id)));
		    continue;
		}
		*/
		if(*(buf+option) != 0x80) {
			printf("option word: %d \n", (*(buf+option)));
		    continue;
		}

		nRet = (iRecv - POS::data) / 6;
    	for(int i = 0; i < nRet; i++) 
		{
    	    if( ntohl(*(int*) (buf + POS::data + i*8)) != m_userID )
    		    continue;
    		/*
			if( ntohs(*(unsigned short*) (buf + POS::data + 4 + i*8)) != m_frameNo+1)
    		    continue;
    		*/
		    if( *(buf + POS::data + 6 + i*8) != 0x60 )
    		    continue;
    
    	    printf("短消息发送成功！(用户已确认) \n");
    		return 0;
    	}
	}

    return 1;
}

int Client::userSendWaitRet(unsigned int id)
{
    unsigned char buf[BUFFER_LENGTH];
    string srcIP;
	unsigned short srcPort;
	int nRet;

    while(1) {
        int iRecv = m_UDPInterface.RecvFrom(buf, BUFFER_LENGTH, srcIP, srcPort);
    	if(iRecv < 0)
    	{
    	    printf("recvfrom(): error %d when user send.\n", iRecv);
    		continue;
        }

		printf("[recv packet from %s %d:] \n", srcIP.data(), srcPort);
	    for(int i = 0; i < iRecv; i++)
	        printf("0x%02X ", buf[i]);
        printf("\n");

		if( ntohl(*(int*) (buf+dest_user)) != m_userID) {
			printf("dest_user_id: %d \n", ntohl(* (int*) (buf+dest_user)));
		    continue;
		}
		if( ntohl(*(int*) (buf+src_user)) != 0) {
			printf("src_user_id: %d \n", ntohl(* (int*) (buf+src_user)));
		    continue;
		}
		/*
		if( ntohs(*(unsigned short*) (buf+frame_id)) != m_frameNo + 1) {
			printf("frame_id: %d %d \n", m_frameNo, ntohs(* (unsigned short*) (buf+frame_id)));
		    continue;
		}
		*/
		if(*(buf+option) != 0x80) {
			printf("option word: %d \n", (*(buf+option)));
		    continue;
		}

		nRet = (iRecv - POS::data) / 6;
    	for(int i = 0; i < nRet; i++) 
		{
    	    if( ntohl(*(int*) (buf + POS::data + i*8)) != m_userID )
    		    continue;
    		/*
			if( ntohs(*(unsigned short*) (buf + POS::data + 4 + i*8)) != m_frameNo+1)
    		    continue;
    		*/
		    if( *(buf + POS::data + 6 + i*8) != 0x50 )
    		    continue;
    
    	    printf("短消息发送成功！(系统回执) \n");
    		return 0;
    	}
	}

    return 1;
}

int Client::userSend(bool isRet, bool isCnf, unsigned int id)
{
    unsigned char buf[BUFFER_LENGTH] = {
	0x00, 0x00, 0x00, 0x00, /* 4 dest user */ 
	0x00, 0x00, 0x00, 0x01, /* 8 src user */
	0x00, 0xff,             /* 10 frame id */
    0x40, 0x00,             /* 12 OPTION */	
	0x00, 0x00, 0x00, 0x00, /* 16 message */
	};

    /* SET dest id id*/
    unsigned int *pDest = (unsigned int *) (buf + POS::dest_user);
	*pDest = htonl(id);

    /* SET src id id*/
    unsigned int *pSrc = (unsigned int *) (buf + POS::src_user);
	*pSrc = htonl(m_userID);

    /* SET frame id*/
    unsigned short *pFrame = (unsigned short *) (buf + POS::frame_id);
	*pFrame = htons(m_frameNo);

	/* SET option */
	unsigned char *pOpt = buf + POS::option;
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
    printf("[send packet to: %s %d]\n", m_channelIP.data(), m_channelPort);
	for(int i = 0; i < packet_len; i++)
	     printf("0x%02X ", buf[i]);
	printf("\n");

    int iSend = m_UDPInterface.SendTo(buf, packet_len, m_channelIP, m_channelPort);
	if (iSend < 0)
	{
	    printf("sendto(): error %d when user on.\n", iSend);
		return -1;
	}
    
    if(isRet)
	    userSendWaitRet(id);
    if(isCnf)
	    userSendWaitCnf(id);

    return 0;
}

int Client::userOnWait()
{
    unsigned char buf[BUFFER_LENGTH];
    string srcIP;
	unsigned short srcPort;

    while(1) {
        int iRecv = m_UDPInterface.RecvFrom(buf, BUFFER_LENGTH, srcIP, srcPort);
    	if(iRecv < 0)
    	{
    	    printf("recvfrom(): error %d when user on.\n", iRecv);
    		continue;
        }

		printf("[recv packet from %s %d:] \n", srcIP.data(), srcPort);
	    for(int i = 0; i < iRecv; i++)
	        printf("0x%02X ", buf[i]);
        printf("\n");

		if( ntohl(*(int*) (buf+dest_user)) != m_userID) {
			printf("dest_user_id: %d \n", ntohl(* (int*) (buf+dest_user)));
		    continue;
		}
		if( ntohl(*(int*) (buf+src_user)) != 0) {
			printf("src_user_id: %d \n", ntohl(* (int*) (buf+src_user)));
		    continue;
		}
		/*
		if( ntohs(*(unsigned short*) (buf+frame_id)) != m_frameNo + 1) {
			printf("frame_id: %d %d \n", m_frameNo, ntohs(* (unsigned short*) (buf+frame_id)));
		    continue;
		}
		*/
		if(*(buf+option) != 0x80) {
			printf("option word: %d \n", (*(buf+option)));
		    continue;
		}
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

int Client::userOn()
{
    unsigned char buf[BUFFER_LENGTH] = {
	0x00, 0x00, 0x00, 0x00, /* 4 dest user */ 
	0x00, 0x00, 0x00, 0x01, /* 8 src user */
	0x00, 0xff,             /* 10 frame id */
    0x00, 0x00,             /* 12 OPTION: user on */	
	0x00, 0x00, 0x00, 0x00, /* 16 time stamp */
	0x00,                   /* 17 E/W, S/N */
	0x00, 0x00, 0x00, 0x00, /* 21 longitude */
	0x00, 0x00, 0x00, 0x00, /* 25 latitude */
	0x00, 0x00, 0x00, 0x00, /* 29 altitude */
	0x01,                   /* 30 cable network */
	0x02,                   /* 31 beam length */
	0x00, 0x00,             /* 33 beam id */
	0x00, 0x00, 0x00, 0x00, /* 37 SNR */
	0x00, 0x01,             /* 39 beam id */
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
		int *tmp = (int *) new float(1.0);
        *pSNR = htonl(*tmp);
	}

	int packet_len = POS::snr + 6 * SNR_LEN;  
    printf("[send packet to: %s %d]\n", m_channelIP.data(), m_channelPort);
	for(int i = 0; i < packet_len; i++)
	     printf("0x%02X ", buf[i]);
	printf("\n");

    int iSend = m_UDPInterface.SendTo(buf, packet_len, m_channelIP, m_channelPort);
	if (iSend < 0)
	{
	    printf("sendto(): error %d when user on.\n", iSend);
		return -1;
	}
    
	userOnWait();
    return 0;

}

int Client::Init()
{
    readInitFile();
    if (m_UDPInterface.Create(m_clientPort))
    {
        printf("Init UDP socket error!\n");
        return -1;
    }

}

void Client::Test()
{
    printf("Channel IP: %s\n", m_channelIP.data());
    printf("Channle Port: %d\n", m_channelPort);
	printf("Client Port: %d\n", m_clientPort);
}
int Client::readInitFile()
{
    char buf[BUFFER_LENGTH];

    if (!read_profile_string("Client", "name", buf, 16, "client_default", INI_FILE_NAME))
    {
        printf("%s: Read Client name from %s failed!\n", m_name.data(), INI_FILE_NAME);
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

Client::Client() {
    /* default init config, better use Client::init() */
    m_userID = 1;
    m_name = "Client";
	srand((unsigned)time(NULL)); 
	m_frameNo = rand() % 0xffff;
}


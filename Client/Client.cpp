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

#define FRAME_POS 8
#define STAMP_POS 12
#define SNR_POS 31
#define SNR_LEN 2

int Client::Run()
{
    userOn();
}

int Client::userOn()
{
    unsigned char buf[BUFFER_LENGTH] = {
	0x00, 0x00, 0x00, 0x00, /* 4 dest user */ 
	0x00, 0x00, 0x00, 0x01, /* 8 src user */
	0x00, 0x01,             /* 10 frame id */
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
    unsigned short *pFrame = (unsigned short *) (buf + FRAME_POS);
	*pFrame = (unsigned short) m_frameNo;

	/* SET time stamp */
	time_t stamp = time(NULL);
	unsigned int *pStamp = ( unsigned int *) (buf + STAMP_POS);
    *pStamp = (unsigned int) stamp;

	/* SET SNR */
	for(int i = 0; i < SNR_LEN; i++) {
        float *pSNR = (float *) (buf + i * 6);
        *pSNR = 1.0;
	}

	int packet_len = SNR_POS + SNR_LEN * 6;
    printf("send packet: %s %d\n", m_channelIP.data(), m_channelPort);
	for(int i = 0; i < packet_len; i++)
	     printf("0x%02X ", buf[i]);

    int iSend = m_UDPInterface.SendTo(buf, packet_len, m_channelIP, m_channelPort);
	if (iSend < 0)
	{
	    printf("sendto(): error %d when user on.\n", iSend);
		return -1;
	}
    
    return 0;

}

int Client::Init()
{
    readInitFile();
    if (m_UDPInterface.Create(0))
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

    return 0;
}

Client::Client() {
    m_name = "Client";
	m_frameNo = 1+ (unsigned short) (0x0fff * rand()/(RAND_MAX+1.0));

}


#include <netdb.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

#include "ChannelUnitForward.h"

#define INI_FILE_NAME "./config.ini"
#define BUFFER_LENGTH 1024

int ChannelUnitForward::Run()
{
    int sockfd, num;
    struct sockaddr_in server;
    unsigned char buf[BUFFER_LENGTH];
    
    if((sockfd= socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        printf("socket() error\n");
        return -1;
    }

    bzero(&server, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(m_serverPort);
    server.sin_addr.s_addr = inet_addr(m_serverIP.data());
     
    if(connect(sockfd, (struct sockaddr *)&server, sizeof(server)) == -1)
    {
        printf("connect() error\n");
        return -1;
    }
    
    while (1)
    { 
        if((num = recv(sockfd, buf, BUFFER_LENGTH, 0)) == -1)
        {
            printf("recv() error\n");
            break;
        }
     
        buf[num] = 0;
        printf("server message: %s\n", buf);
        for (int i = 0; i < m_clientIPs.size(); i ++)
        {
            printf("sendto(): %s %d\n", m_clientIPs[i].data(), m_clientPorts[i]);
		    int iSend = m_UDPInterface.SendTo(buf, num, m_clientIPs[i], m_clientPorts[i]);
			if (iSend < 0)
			{
			    printf("sendto(): error %d\n", iSend);
			}
        }

    }
    
    close(sockfd);
    return 0;

}

int ChannelUnitForward::Init()
{
    readInitFile();
    if (m_UDPInterface.Create(0))
    {
        printf("Init UDP socket error!\n");
        return -1;
    }

}

void ChannelUnitForward::Test()
{
    printf("Server IP: %s\n", m_serverIP.data());
    printf("Server Port: %d\n\n", m_serverPort);
	printf("Client Length: %d\n", m_clientIPs.size());
	printf("Client Port: %d\n", m_clientPort);
	for (int i = 0; i < m_clientIPs.size(); i++)
	{
	    printf("Cleint IP: %s\n", m_clientIPs[i].data());
	    printf("Cleint Port: %d\n", m_clientPorts[i]);
	}

}

int ChannelUnitForward::readInitFile() 
{
    char buf[BUFFER_LENGTH];

    m_serverPort = (unsigned short) read_profile_int("Server", "Port", 0, INI_FILE_NAME);
    if (m_serverPort == 0)
    {
        printf("%s: Read ServerPort from %s failed!\n", m_name.data(), INI_FILE_NAME);
        return -1;
    }

	if (!read_profile_string("Server", "IP", buf, 16, "127.0.0.1", INI_FILE_NAME))
	{
        printf("%s: Read Server IP from %s failed!\n", m_name.data(), INI_FILE_NAME);
		return -1;
	}
	m_serverIP = buf;

	int clientNum = read_profile_int("Client", "Length", -1, INI_FILE_NAME);
	if (clientNum == -1)
	{
        printf("%s: Read client length from %s failed!\n", m_name.data(), INI_FILE_NAME);
		return -1;
	}

    m_clientPort = (unsigned short) read_profile_int("Client", "Port", 0, INI_FILE_NAME);
    if (m_clientPort == 0)
    {
        printf("%s: Read Client Port from %s failed!\n", m_name.data(), INI_FILE_NAME);
        return -1;
    }

    for (int i = 0; i < clientNum; i ++)
    {
        string clientSector = "ClientInstance" + std::to_string(i);
        printf("clientSector %s\n", clientSector.data());

		unsigned short clientPort = (unsigned short) read_profile_int(clientSector.data(), "InstancePort", 0, INI_FILE_NAME);
        if (clientPort == 0)
        {
            printf("%s: Read Client Port from %s failed!\n", m_name.data(), INI_FILE_NAME);
            return -1;
        }
    
    	if (!read_profile_string(clientSector.data(), "InstanceIP", buf, 16, "127.0.0.1", INI_FILE_NAME))
    	{
            printf("%s: Read Client IP from %s failed!\n", m_name.data(), INI_FILE_NAME);
    		return -1;
    	}
		string clientIP = buf;
        m_clientIPs.push_back(clientIP);
        m_clientPorts.push_back(clientPort);
	}

    return 0;
}

ChannelUnitForward::ChannelUnitForward() {
    m_name = "ChannelUnitForward";
	m_clientIPs.clear();
	m_clientPorts.clear();
}


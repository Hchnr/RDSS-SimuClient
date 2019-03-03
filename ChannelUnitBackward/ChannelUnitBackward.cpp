#include <unistd.h>
#include <string.h>

#include "ChannelUnitBackward.h"

#define INI_FILE_NAME "./config.ini"
#define BUFFER_LENGTH 1024

int ChannelUnitBackward::Run()
{
    unsigned char buf[BUFFER_LENGTH];
    /*
    int sockfd, num;
    struct sockaddr_in server;
    
    if((sockfd= socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        printf("socket() error\n");
        return -1;
    }

    bzero(&server, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(m_serverBackwardPort);
    server.sin_addr.s_addr = inet_addr(m_serverIP.data());
     
    if(connect(sockfd, (struct sockaddr *)&server, sizeof(server)) == -1)
    {
        printf("connect() error\n");
        return -1;
    }
    */
    if (! m_TcpSocket.Create())
    {
        printf("Init TCP socket error!\n");
        return -1;
    }
    m_TcpSocket.SetBlocking(true);
	printf("1\n");
    if (! m_TcpSocket.Connect(m_serverIP.data(), m_serverBackwardPort) )
    {
        printf("Connect TCP error!\n");
        return -1;
    }
	printf("2\n");

    if (m_UDPInterface.Create(m_clientPort))
    {
        printf("Init UDP socket error!\n");
        return -1;
    }
 
    while (1)
    { 
        string strSrcIP;
        unsigned short usSrcPort;
        int iLen = m_UDPInterface.RecvFrom(buf, BUFFER_LENGTH, strSrcIP, usSrcPort);
        if (iLen < 0)
        {
            printf("Recv UDP error!\n");
            return -1;
        } 
        printf("backward channel send: \n");
	    for(int i = 0; i < iLen; i ++) {
		    printf("%02x ", buf[i]);
		}	
		printf("\n");
        m_TcpSocket.SendPacket((char*) buf, iLen);

    }
    
    m_TcpSocket.Close();
    return 0;

}

int ChannelUnitBackward::Init()
{
    readInitFile();
}

void ChannelUnitBackward::Test()
{
    printf("Server IP: %s\n", m_serverIP.data());
    printf("Server Forward Port: %d\n\n", m_serverForwardPort);
    printf("Client Port: %d\n", m_clientPort);
}

int ChannelUnitBackward::readInitFile() 
{
    char buf[BUFFER_LENGTH];

    /* READ server */
    m_serverBackwardPort = (unsigned short) read_profile_int("Server", "BackwardPort", 0, INI_FILE_NAME);
    if (m_serverBackwardPort == 0)
    {
        printf("%s: Read ServerBackwardPort from %s failed!\n", m_name.data(), INI_FILE_NAME);
        return -1;
    }

    if (!read_profile_string("Server", "IP", buf, 16, "127.0.0.1", INI_FILE_NAME))
    {
        printf("%s: Read Server IP from %s failed!\n", m_name.data(), INI_FILE_NAME);
        return -1;
    }
    m_serverIP = buf;

    int serverForwardPortLength = 0, serverForwardPortIndex = 0;
    serverForwardPortLength = (unsigned short) read_profile_int("Server", "ForwardPortLength", 0, INI_FILE_NAME);
    if (serverForwardPortLength == 0)
    {
        printf("%s: Read ServerForwardPortLength from %s failed!\n", m_name.data(), INI_FILE_NAME);
        return -1;
    }

    serverForwardPortIndex = (unsigned short) read_profile_int("Server", "serverForwardPortIndex", 0, INI_FILE_NAME);
    if (serverForwardPortLength == 0)
    {
        printf("%s: Read ServerForwardPortIndex from %s failed!\n", m_name.data(), INI_FILE_NAME);
        return -1;
    }

    vector<unsigned short> serverForwardPorts;
    for (int i = 0; i < serverForwardPortLength; i ++)
    {
        string sector = "ForwardPort" + std::to_string(i);
        
        unsigned short port = (unsigned short) read_profile_int("Server", sector.data(), 0, INI_FILE_NAME);
        if (port == 0)
        {
            printf("%s: Read server forward port from %s failed!\n", m_name.data(), INI_FILE_NAME);
            return -1;
        }
    
        serverForwardPorts.push_back(port);
    }
    m_serverForwardPort = serverForwardPorts[serverForwardPortIndex];
    
    /* READ client */
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

ChannelUnitBackward::ChannelUnitBackward() {
    m_name = "ChannelUnitBackward";
    m_clientIPs.clear();
    m_clientPorts.clear();
}


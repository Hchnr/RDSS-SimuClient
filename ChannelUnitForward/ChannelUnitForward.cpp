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
    unsigned char buf[BUFFER_LENGTH];
    
    if (! m_TcpSocket.Create())
    {
        printf("Init TCP socket error!\n");
        return -1;
    }
    m_TcpSocket.SetBlocking(true);
    if (! m_TcpSocket.Connect(m_serverIP.data(), m_serverForwardPort) )
    {
        printf("Connect TCP error!\n");
        return -1;
    }
 
    int len, kHeadLen = 4;
    while (1)
    { 
        int complete_packet_num = m_TcpSocket.RecvPacket((char *) buf, &len);
        if (complete_packet_num < 0){ 
            printf("receive failed or not receive 1 or more complete packet");
            break;
        }
        else{
            //std::cout << "package num: " << complete_packet_num << std::endl; 
            char* read_pos = (char *) buf;
            for (int j = 0; j < complete_packet_num; j++){
                int packet_len = *(int*)read_pos;
                read_pos += kHeadLen;

                /* SEND packet to all client in channel */
                printf("server message: \n");
                for(int i = 0; i < packet_len; i++)
                    printf("0x%02X ", read_pos[i]);
				printf("\n");
                for (int i = 0; i < m_clientIPs.size(); i ++)
                {
                    printf("sendto(): %s %d\n", m_clientIPs[i].data(), m_clientPorts[i]);
        		    int iSend = m_UDPInterface.SendTo(( unsigned char*) read_pos, packet_len, m_clientIPs[i], m_clientPorts[i]);
        			if (iSend < 0)
        			{
        			    printf("sendto(): error %d\n", iSend);
        			}
                }

                read_pos += packet_len;
            }
        }    
 

    }
    
    m_TcpSocket.Close();
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
    printf("Server Forward Port: %d\n\n", m_serverForwardPort);
    printf("Server Backward Port: %d\n\n", m_serverBackwardPort);
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

ChannelUnitForward::ChannelUnitForward() {
    m_name = "ChannelUnitForward";
	m_clientIPs.clear();
	m_clientPorts.clear();
}


#include <string.h>

#include <qdebug.h>

#include "ChannelUnitForward.h"

#define INI_FILE_NAME ".\\config.ini"
#define BUFFER_LENGTH 1024

int ChannelUnitForward::Run() {
    m_thread->start();
    return 0;
}

int ChannelUnitForward::run_blocked()
{
    unsigned char buf[BUFFER_LENGTH];
    
    if (! m_TcpSocket.Create())
    {
        qDebug("Init TCP socket error!\n");
        return -1;
    }
    m_TcpSocket.SetBlocking(true);
    if (! m_TcpSocket.Connect(m_serverIP.data(), m_serverForwardPort) )
    {
        qDebug("Connect TCP error!\n");
        return -1;
    }
 
    int len, kHeadLen = 4;
    while (1)
    { 
        qDebug("receiving...\n");
        int complete_packet_num = m_TcpSocket.RecvPacket((char *) buf, &len);
        if (complete_packet_num < 0){ 
            qDebug("receive failed or not receive 1 or more complete packet");
            break;
        }
        else{
            //std::cout << "package num: " << complete_packet_num << std::endl; 
            char* read_pos = (char *) buf;
            qDebug("readpos: %d\n", read_pos);
            for (int j = 0; j < complete_packet_num; j++){
                int packet_len = *(int*)read_pos;
                read_pos += kHeadLen;

                /* SEND packet to all client in channel */
                qDebug("server message: \n");
                for(int i = 0; i < packet_len; i++)
                    qDebug("0x%02X ", read_pos[i]);
                qDebug("\n");
                for (int i = 0; i < m_clientIPs.size(); i ++)
                {
                    qDebug("sendto(): %s %d\n", m_clientIPs[i].data(), m_clientPorts[i]);
                    int iSend = m_UDPInterface.SendTo(( unsigned char*) read_pos, packet_len, m_clientIPs[i], m_clientPorts[i]);
                    if (iSend < 0)
                    {
                        qDebug("sendto(): error %d\n", iSend);
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
        qDebug("Init UDP socket error!\n");
        return -1;
    }

}

void ChannelUnitForward::Test()
{
    qDebug("Server IP: %s\n", m_serverIP.data());
    qDebug("Server Forward Port: %d\n\n", m_serverForwardPort);
    qDebug("Server Backward Port: %d\n\n", m_serverBackwardPort);
    qDebug("Client Length: %d\n", m_clientIPs.size());
    qDebug("Client Port: %d\n", m_clientPort);
    for (int i = 0; i < m_clientIPs.size(); i++)
    {
        qDebug("Cleint IP: %s\n", m_clientIPs[i].data());
        qDebug("Cleint Port: %d\n", m_clientPorts[i]);
    }

}
int ChannelUnitForward::readInitFile()
{
    char buf[BUFFER_LENGTH];

    if (!read_profile_string("Meta", "Name", buf, 16, "forward", INI_FILE_NAME))
    {
        qDebug("%s: Read Channel Name from %s failed!\n", m_name.data(), INI_FILE_NAME);
        return -1;
    }
    m_name = buf;

        /* READ server */
    m_serverBackwardPort = (unsigned short) read_profile_int("Server", "BackwardPort", 0, INI_FILE_NAME);
    if (m_serverBackwardPort == 0)
    {
        qDebug("%s: Read ServerBackwardPort from %s failed!\n", m_name.data(), INI_FILE_NAME);
        return -1;
    }

        if (!read_profile_string("Server", "IP", buf, 16, "127.0.0.1", INI_FILE_NAME))
        {
        qDebug("%s: Read Server IP from %s failed!\n", m_name.data(), INI_FILE_NAME);
                return -1;
        }
        m_serverIP = buf;

        int serverForwardPortLength = 0, serverForwardPortIndex = 0;
    serverForwardPortLength = (unsigned short) read_profile_int("Server", "ForwardPortLength", 0, INI_FILE_NAME);
    if (serverForwardPortLength == 0)
    {
        qDebug("%s: Read ServerForwardPortLength from %s failed!\n", m_name.data(), INI_FILE_NAME);
        return -1;
    }

    serverForwardPortIndex = (unsigned short) read_profile_int("Server", "ForwardPortIndex", 0, INI_FILE_NAME);
    qDebug("Server Forward Port index: %d\n\n", serverForwardPortIndex);

    string sector = "ForwardPort" + std::to_string(serverForwardPortIndex);
    m_serverForwardPort =  (unsigned short) read_profile_int("Server", sector.data(), 0, INI_FILE_NAME);
    if (m_serverForwardPort == 0)
    {
        qDebug("%s: Read ServerForwardPort from %s failed!\n", m_name.data(), INI_FILE_NAME);
        return -1;
    }

    /* READ client */
    int clientNum = read_profile_int("Client", "Length", -1, INI_FILE_NAME);
    if (clientNum == -1)
    {
    qDebug("%s: Read client length from %s failed!\n", m_name.data(), INI_FILE_NAME);
            return -1;
    }

    m_clientPort = (unsigned short) read_profile_int("Client", "Port", 0, INI_FILE_NAME);
    if (m_clientPort == 0)
    {
        qDebug("%s: Read Client Port from %s failed!\n", m_name.data(), INI_FILE_NAME);
        return -1;
    }

        for (int i = 0; i < clientNum; i ++)
    {
        string clientSector = "ClientInstance" + std::to_string(i);

                unsigned short clientPort = (unsigned short) read_profile_int(clientSector.data(), "InstancePort", 0, INI_FILE_NAME);
        if (clientPort == 0)
        {
            qDebug("%s: Read Client Port from %s failed!\n", m_name.data(), INI_FILE_NAME);
            return -1;
        }

        if (!read_profile_string(clientSector.data(), "InstanceIP", buf, 16, "127.0.0.1", INI_FILE_NAME))
        {
            qDebug("%s: Read Client IP from %s failed!\n", m_name.data(), INI_FILE_NAME);
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
    m_thread = new ChannelThread(this);
}


#ifndef UTILS_H_
#define UTILS_H_
#include<iostream>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fstream>
#include <assert.h>
#ifdef LINUX
#include <pthread.h>
#include <netpacket/packet.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <linux/if_ether.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <linux/if.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/ip.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/ip.h>
#include <netpacket/packet.h>
#include <linux/stddef.h>
#include <sys/select.h>
#include <sys/time.h>
typedef  int SockInt;
#else
#include<winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#pragma comment(lib, "ws2_32.lib")
typedef SOCKET SockInt;
#endif
 
template<typename T>
inline void SafeDelete(T& p){
    if (p){
        delete p;
        p = NULL;
    }
}  
#ifdef PTHREAD
//pthread functions
bool StartThread(pthread_t* thread_id, void* thread_func(void*), void* param);
void StopThread(pthread_t thread_id);
void JoinThread(pthread_t& );
#else
#include<thread>
bool StartThread(std::thread* t, void* thread_func(void*), void*param);
void JoinThread(std::thread&t);
#endif

//time
void GetTime(char* buffer);
//build socket addr
void BuildSockAddr(const char* ip, const uint16_t port, struct sockaddr_in* sock_addr);
#endif

#ifdef WIN32
int gettimeofday(struct timeval *tp, void *tzp);
#endif




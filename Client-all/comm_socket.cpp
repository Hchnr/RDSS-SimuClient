#include"comm_socket.h"
#ifndef LINUX
bool CommonSocket::wsa_inited_ = false;
#endif

CommonSocket::CommonSocket(){
	memset(&local_addr_, 0,  sizeof(local_addr_));
	memset(&peer_addr_, 0, sizeof(peer_addr_));

#ifndef LINUX //in windows system
	if (wsa_inited_){
		return;
	}

	WSADATA wsaData;
	int iErrorCode;
	if (WSAStartup(MAKEWORD(2, 1), &wsaData)) //调用Windows Sockets DLL
	{
		printf("Winsock init failed\n");
		WSACleanup();
		return;
	}
	wsa_inited_ = true;
#endif
}
CommonSocket::~CommonSocket(){
	Close();
}
//绑定local_addr_，并设置SO_REUSEADDR
bool CommonSocket::Bind(const char* listen_ip, uint16_t listen_port){
	   local_addr_.sin_family = AF_INET;
	   local_addr_.sin_port = htons(listen_port);
	   local_addr_.sin_addr.s_addr = inet_addr(listen_ip);
	   uint32_t option = 1;
	   if (setsockopt(socket_fd_, SOL_SOCKET, SO_REUSEADDR, (char*)&option, sizeof(option)) < 0){
		   perror("set socket reuse addr \n");
		   printf("set socket reuse addr %d\n", errno);
		   return false;
	   }
	   if (bind(socket_fd_, (sockaddr*)&local_addr_, sizeof(local_addr_)) != 0){
		   perror("bind socket\n");
		   return false;
	   }
	   return true;
}
//设置缓冲区大小
bool CommonSocket::SetSendBufSize(int size){
	int result = setsockopt(socket_fd_, SOL_SOCKET, SO_SNDBUF, (char*)&size, sizeof(int));
	if (result < 0){
		perror("setsockopt send buf size\n");
		return false;
	}
	return true;
}

bool CommonSocket::SetRecvBufSize(int size){
	int result = setsockopt(socket_fd_, SOL_SOCKET, SO_RCVBUF, (char*)&size, sizeof(int));
	if (result < 0){
		perror("setsockopt recv buf size\n");
		return false;
	}
	return true;
}

//设置socket是否阻塞：0为阻塞，1为非阻塞
bool CommonSocket::SetBlocking(bool block){
#ifdef LINUX
	int opts;
	opts = fcntl(socket_fd_, F_GETFL);
	if (opts < 0){
		perror("fcntl get");
		return false;
	}

	if (block){ //set block mode
		opts = opts & (~O_NONBLOCK);
	}
	else{     // set nonblock mode
		opts = opts | O_NONBLOCK;
	}

	if (fcntl(socket_fd_, F_SETFL, opts) < 0){
		perror("fcntl set");
		return false;
	}
#else
	unsigned int uint;
	int ret;
	if (block){
		uint = 0;
		ret = ioctlsocket(socket_fd_, FIONBIO, (unsigned long *)&uint);//block
	}
	else{
		uint = 1;
		ret = ioctlsocket(socket_fd_, FIONBIO, (unsigned long *)&uint);//non block
	}

	if (ret == SOCKET_ERROR) {
		perror("set non block failed\n");
	}
#endif

	return true;
}

bool CommonSocket::Connect(const char* peer_ip, uint16_t peer_port){
	peer_addr_.sin_family = AF_INET;
	peer_addr_.sin_addr.s_addr = inet_addr(peer_ip);
	peer_addr_.sin_port = htons(peer_port);
	int result = connect(socket_fd_, (sockaddr*)&peer_addr_, sizeof(peer_addr_));
	if (result < 0){
		perror("Common connect\n");
	}
	return result  == 0;
}   

//关闭socket_fd
void CommonSocket::Close(){
#ifdef LINUX
	close(socket_fd_);
#else
	closesocket(socket_fd_);
#endif
}

int CommonSocket::Send(char*buf, int len){
	int result = send(socket_fd_, buf, len, 0);
	if (result < 0){
		perror("Common send to connected peer\n");
	}
	return result;
}
int CommonSocket::Recv(char* buf, int buf_len){
	int recv_bytes = recv(socket_fd_, buf, buf_len, 0);
	if (recv_bytes < 0){
		perror("Common recv from connected peer \n");
	}
	return recv_bytes;
}


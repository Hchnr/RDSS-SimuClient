#include"utils.h"
//pthread functions
#ifdef PTHREAD
bool StartThread(pthread_t* thread_id, void* thread_func(void*), void* param){
	int ret = pthread_create(thread_id, NULL, thread_func, param);
	if (ret != 0){
		perror("create thread!\n");
		return false;
	}
	return true;
}

void StopThread(pthread_t thread_id){
	pthread_cancel(thread_id);
}
void JoinThread(pthread_t& thread_id){
	pthread_join(thread_id, NULL);
}
#else
bool StartThread(std::thread* thread, void* thread_func(void*), void* param){
	*thread = std::thread(thread_func, param);
	if (!thread){
		perror("create thread!\n");
		return false;
	}
	return true;
}
void JoinThread(std::thread& t){
	t.join();
}
#endif


//time function
void GetTime(char* buffer){
	time_t now;
	struct tm* timenow;
	time(&now);
	timenow = localtime(&now);
	strcpy(buffer, asctime(timenow));
}

void BuildSockAddr(const char* ip, const uint16_t port, struct sockaddr_in* sock_addr){
	memset((char*)sock_addr, 0, sizeof(struct sockaddr_in));
	sock_addr->sin_family = AF_INET;
	sock_addr->sin_port = htons(port);
	sock_addr->sin_addr.s_addr = inet_addr(ip);
}

#ifdef WIN32
int gettimeofday(struct timeval *tp, void *tzp)
{
	time_t clock;
	struct tm tm;
	SYSTEMTIME wtm;
	GetLocalTime(&wtm);
	tm.tm_year = wtm.wYear - 1900;
	tm.tm_mon = wtm.wMonth - 1;
	tm.tm_mday = wtm.wDay;
	tm.tm_hour = wtm.wHour;
	tm.tm_min = wtm.wMinute;
	tm.tm_sec = wtm.wSecond;
	tm.tm_isdst = -1;
	clock = mktime(&tm);
	tp->tv_sec = clock;
	tp->tv_usec = wtm.wMilliseconds * 1000;
	return (0);
}
#endif

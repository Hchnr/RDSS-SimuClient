#ifndef FIFO_QUEUE_H_
#define FIFO_QUEUE_H_
#include<stdint.h>

#include <boost/thread/condition.hpp>
#include <boost/thread/mutex.hpp>

#define MAX_PACK_SIZE 65535
#define INIT_BUF_SIZE 10000000
#define END_FLAG 0x0000
#define PACK_LEN_SIZE 2

#define USE_LOCK

struct Cell{
	uint32_t cell_id;		//Transaction ID

	uint32_t unicast_ip;    //unicast ip, this is not necessary, 4 bytes len
	
	uint16_t data_len;           //data length of cell_body

	int body_start_pos;     //the cell body's start position

	char cell_data[900];        //pointer to the buffer where store cell body

	Cell():unicast_ip(0), data_len(0), body_start_pos(0){
	};

	~Cell(){
	}
};

class FifoQueue{
public:
	FifoQueue(int buf_size = INIT_BUF_SIZE);
	~FifoQueue();

	void Reset();

	uint16_t PeekNextPackLen();  

	bool Update(char*& recv_pos, int recv_len);

	//push a complete packet into buffer
	bool PushPacket(char* packet, uint16_t len);

	//pop a complete packet from buffer
	bool PopPacket(char* pacet, uint16_t * len);
private:
	void Init(int);
	char* buffer_;		
	char* buf_end_;		
	volatile char* read_pos_;
	volatile char* write_pos_;
	volatile uint64_t read_bytes_;
	volatile uint64_t write_bytes_;
	
	int buf_size_;
#ifdef USE_LOCK
	boost::condition condition_;
	boost::mutex mutex_lock_;
#endif

};
#endif

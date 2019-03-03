#include<assert.h>
#include<string.h>
#include<stdio.h>
#include"fifo_queue.h"



FifoQueue::FifoQueue(int buf_size){
	Init(buf_size);
}

FifoQueue::~FifoQueue(){
	delete[] buffer_;
}

void FifoQueue::Init(int buf_size){
	buf_size_ = buf_size;
	buffer_ = new char[buf_size_];
	buf_end_ = buffer_ + buf_size_;

	read_pos_ = write_pos_ = buffer_;
	read_bytes_ = write_bytes_ = 0;
}

void FifoQueue::Reset(){
	read_pos_ = write_pos_ = buffer_;
	read_bytes_ = write_bytes_ = 0;
}

bool FifoQueue::PushPacket(char* packet, uint16_t len){
	assert(len < MAX_PACK_SIZE);   
#ifdef USE_LOCK
	boost::unique_lock<boost::mutex> ulock(mutex_lock_);
#endif

	if (write_bytes_ + 2*len + 2*(PACK_LEN_SIZE - 1) > read_bytes_ + buf_size_){
		printf("push data faile, buffer overflow!\n");
		return false;
	}    
	if (write_pos_ + PACK_LEN_SIZE > buf_end_){
		write_pos_ = buffer_;
	}else if (write_pos_ + len + sizeof(len) > buf_end_){     
		*(uint16_t*)write_pos_ = END_FLAG;
		write_pos_ = buffer_;
	}
	
	*(uint16_t*)write_pos_ = len;
	write_pos_ += PACK_LEN_SIZE;
	memcpy((void*)write_pos_, packet, len);
	write_pos_ += len;
	write_bytes_ += (len + PACK_LEN_SIZE);

#ifdef USE_LOCK    
	condition_.notify_one();
#endif
	return true;
}

bool FifoQueue::PopPacket(char* packet, uint16_t* len){

#ifdef USE_LOCK
	boost::unique_lock<boost::mutex> ulock(mutex_lock_);
	while (read_bytes_ >= write_bytes_)
		condition_.wait(ulock);
#else
	if (read_bytes_ >= write_bytes_){
		return false;
	}
#endif

	if (read_pos_ + PACK_LEN_SIZE > buf_end_){
		read_pos_ = buffer_;
	}
	*len = *(uint16_t*)read_pos_;
	if (*len == END_FLAG){
		read_pos_ = buffer_;
		*len = *(uint16_t*)read_pos_;
	}
	read_pos_ += PACK_LEN_SIZE;
	memcpy(packet,(void*)read_pos_, *len);
	read_pos_ += *len;
	read_bytes_ += (*len + PACK_LEN_SIZE);
	return true;
}

uint16_t FifoQueue::PeekNextPackLen(){
	uint16_t len;

	if (read_bytes_ >= write_bytes_){
		return 0;
	}

	if (read_pos_ + PACK_LEN_SIZE > buf_end_){
		read_pos_ = buffer_;
	}
	len = *(uint16_t*)read_pos_;
	if (len == END_FLAG){
		read_pos_ = buffer_;
		len = *(uint16_t*)read_pos_;
	}
	return len;
}


bool FifoQueue::Update(char*& recv_pos, int len){
	 assert(len < MAX_PACK_SIZE);
	*(uint16_t*)(recv_pos - PACK_LEN_SIZE) = (uint16_t)len;
#ifdef USE_LOCK
	boost::unique_lock<boost::mutex> ulock(mutex_lock_);
#endif

	if (write_bytes_ + 2*len + 2*(PACK_LEN_SIZE - 1) > read_bytes_ + buf_size_){
		printf("push data faile, buffer overflow!\n");
		return false;
	}
	recv_pos += (len + PACK_LEN_SIZE);
	if (recv_pos > buf_end_){
		recv_pos = buffer_ + PACK_LEN_SIZE;
	}else if (recv_pos + len > buf_end_){ 
		*(uint16_t*)(recv_pos - PACK_LEN_SIZE) = END_FLAG;
		recv_pos = buffer_  + PACK_LEN_SIZE;
	}
	write_bytes_ += (len + PACK_LEN_SIZE);
#ifdef USE_LOCK
	if (write_bytes_ > read_bytes_){
		condition_.notify_one();
	}
#endif
	return true;
}




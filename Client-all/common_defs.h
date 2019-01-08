#ifndef COMMON_DEFS_H_
#define COMMON_DEFS_H_

#define DEFAULT_CELL_SIZE 491
#define MAX_CELL_SIZE 900


enum ProcClientType{
    CELL_SPLITTER = 1,
    MSG_RECEIVER = 2,
    VIDEO_RECEIVER = 3,
    STREAM_SERVER = 4,
    FILE_RECEIVER = 5,
    FILE_MANAGER = 6,
    
    CLIENT_TYPE_NONE = 7
};

//signals used 
enum SignalType{ //signal type between module's communication with  monitor server
    REGISTER = 1,   //register to monitor server
    UNREGISTER = 2, //unregister from monitor server
    REGISTER_SUCCEEDED = 3,
    REGISTER_FAILED = 4,
    EXCEPTION = 5,  //exception report to monitor server
    TERMINATE = 6,   //monitor server tell modules to terminate process
    OTHER = 7   //other information , string type
};

enum SecurityParam{
    SEC_LEVEL_POS = 9,
    SEC_HEAD_LEN = 11
};

enum ServiceType{
    FILE_CONFIG_SERVICE = 0x00,
    FILE_SERVICE = 0x01,
    VIDEO_SERVICE = 0x02,
    IPSTREAM_SERVICE = 0x03,
    MESSAGE_SERVICE = 0x04
};

#endif

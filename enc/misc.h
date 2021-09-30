#ifndef MISC_H_
#define MISC_H_


#define LOGSTR_SIZE 512

#define err(fmt, ...)													\
	do {																\
		char logstr[LOGSTR_SIZE];										\
		int ret = 0;													\
        sprintf(logstr, "Error: " fmt "", ##__VA_ARGS__);				\
        ocall_log(&ret, logstr);										\
    } while (0)
#define warn(fmt, ...)													\
	do {																\
		char logstr[LOGSTR_SIZE];										\
		int ret = 0;													\
        sprintf(logstr, "Warning: " fmt "", ##__VA_ARGS__);				\
        ocall_log(&ret, logstr);										\
    } while (0)
#define info(fmt, ...)													\
	do {																\
		char logstr[LOGSTR_SIZE];										\
		int ret = 0;													\
        sprintf(logstr, "" fmt "", ##__VA_ARGS__);						\
        ocall_log(&ret, logstr);										\
    } while (0)
#ifdef DEBUG
#define dbg(fmt, ...)													\
	do {																\
		char logstr[LOGSTR_SIZE];										\
		int ret = 0;													\
        sprintf(logstr, ":%s:%d: " fmt "", __func__, __LINE__, ##__VA_ARGS__); \
        ocall_log(&ret, logstr);										\
    } while (0)
#else
#define dbg(fmt, ...) do { } while (0)
#endif


#endif // MISC_H_

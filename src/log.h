#ifndef EYRE_LOG_INCLUDE
#define EYRE_LOG_INCLUDE



void printPointer(void* value);

void printInt(int value);

void printString(char* value);

void println(char* format, ...);



// Always results in termination, logged to stderr
void eyreError_(char* format, const char* file, int line, ...);

// Always logged
void eyreLogWarning_(char* format, const char* file, int line, ...);

// Important debug messages
void eyreLogDebug_(char* format, const char* file, int line, ...);

// Unimportant debug messages
void eyreLogInfo_(char* format, const char* file, int line, ...);

// Only for targeted debugging
void eyreLogTrace_(char* format, const char* file, int line, ...);



#define eyreError(format, ...) eyreError_(format, __FILE__, __LINE__, ##__VA_ARGS__)

#define eyreLogWarning(format, ...) eyreLogWarning_(format, __FILE__, __LINE__, ##__VA_ARGS__)

#if defined(EYRE_LOG_DEBUG) | defined(EYRE_LOG_ALL)
#define eyreLogDebug(format, ...) eyreLogDebug_(format, __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define eyreLogDebug(format, ...)
#endif

#if defined(EYRE_LOG_INFO) | defined(EYRE_LOG_ALL)
#define eyreLogInfo(format, ...) eyreLogInfo_(format, __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define eyreLogInfo(format, ...)
#endif

#if defined(EYRE_LOG_TRACE) | defined(EYRE_LOG_ALL)
#define eyreLogTrace(format, ...) eyreLogTrace_(format, __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define eyreLogTrace(format, ...)
#endif



#endif
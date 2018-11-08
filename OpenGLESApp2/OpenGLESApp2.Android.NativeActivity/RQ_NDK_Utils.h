#ifndef __RQ_NDK_Utils__
#define __RQ_NDK_Utils__

#include <stdint.h>
#include <time.h>

#include <android/asset_manager.h> 
#include <android/asset_manager_jni.h>

#include "android_fopen.h"

int64_t timespec2ms64(struct timespec *pt);
int64_t timespec2us64(struct timespec *pt);
void *getAssetFileToBuffer(android_app* state, const char *pFileName, size_t &size);

//Usage example
//int64_t xxval = timespec2ms64(&t0);


#endif

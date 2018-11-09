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


template<typename Tptr_type> class RQTCAutoDelete {
 public:
 Tptr_type *managed_pointer;
 RQAutoDelete(Tptr_type *_managed_pointer = NULL) {
  managed_pointer = _managed_pointer;
 }
 ~RQAutoDelete() {
  if(managed_pointer==NULL)
   return;
  delete managed_pointer;
  managed_pointer = NULL;
 }
};

#define IF_NULL_DELETE(PARAM) if( NULL != PARAM ) { delete PARAM; PARAM = NULL; }
#define IF_NULL_DELETE_ARRAY(PARAM) if( NULL != PARAM ) { delete [] PARAM; PARAM = NULL; }



#endif

#pragma once

#include <jni.h>
#include <SLES/OpenSLES_Android.h>
#include <sys/types.h>
#include <cassert>
#include <cstring>


#include "IFAudioUtils.h"

namespace IFAudioSLES {

class AudioPlayer {
 // buffer queue player interfaces
 SLObjectItf outputMixObjectItf_;
 SLObjectItf playerObjectItf_;
 SLPlayItf playItf_;
 SLAndroidSimpleBufferQueueItf playBufferQueueItf_;

 SampleFormat sampleInfo_;
 AudioQueue *freeQueue_;       // user
 AudioQueue *playQueue_;       // user
 AudioQueue *devShadowQueue_;  // owner

 ENGINE_CALLBACK callback_;
 void *ctx_;
 sample_buf silentBuf_;
#ifdef ENABLE_LOG
 AndroidLog *logFile_;
#endif
 std::mutex stopMutex_;

public:
 explicit AudioPlayer(SampleFormat *sampleFormat, SLEngineItf engine);
 ~AudioPlayer();
 void SetBufQueue(AudioQueue *playQ, AudioQueue *freeQ);
 SLresult Start(void);
 void Stop(void);
 void ProcessSLCallback(SLAndroidSimpleBufferQueueItf bq);
 uint32_t dbgGetDevBufCount(void);
 void RegisterCallback(ENGINE_CALLBACK cb, void *ctx);
};



class AudioRecorder {
 SLObjectItf recObjectItf_;
 SLRecordItf recItf_;
 SLAndroidSimpleBufferQueueItf recBufQueueItf_;

 SampleFormat sampleInfo_;
 AudioQueue *freeQueue_;       // user
 AudioQueue *recQueue_;        // user
 AudioQueue *devShadowQueue_;  // owner
 uint32_t audioBufCount;

 ENGINE_CALLBACK callback_;
 void *ctx_;

public:
 explicit AudioRecorder(SampleFormat *, SLEngineItf engineEngine);
 ~AudioRecorder();
 SLboolean Start(void);
 SLboolean Stop(void);
 void SetBufQueues(AudioQueue *freeQ, AudioQueue *recQ);
 void ProcessSLCallback(SLAndroidSimpleBufferQueueItf bq);
 void RegisterCallback(ENGINE_CALLBACK cb, void *ctx);
 int32_t dbgGetDevBufCount(void);

};




}
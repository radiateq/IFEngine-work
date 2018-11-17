#include "IFAudioSLES.h"

#include <cstring>
#include <cstdlib>

using namespace IFAudioSLES;
namespace IFAudioSLES {
/*
 * Called by OpenSL SimpleBufferQueue for every audio buffer played
 * directly pass thru to our handler.
 * The regularity of this callback from openSL/Android System affects
 * playback continuity. If it does not callback in the regular time
 * slot, you are under big pressure for audio processing[here we do
 * not do any filtering/mixing]. Callback from fast audio path are
 * much more regular than other audio paths by my observation. If it
 * very regular, you could buffer much less audio samples between
 * recorder and player, hence lower latency.
 */
void bqPlayerCallback(SLAndroidSimpleBufferQueueItf bq, void *ctx) {
 (static_cast<AudioPlayer *>(ctx))->ProcessSLCallback(bq);
}
void AudioPlayer::ProcessSLCallback(SLAndroidSimpleBufferQueueItf bq) {
#ifdef ENABLE_LOG
 logFile_->logTime();
#endif
 std::lock_guard<std::mutex> lock(stopMutex_);

 // retrieve the finished device buf and put onto the free queue
 // so recorder could re-use it
 sample_buf *buf;
 if (!devShadowQueue_->front(&buf)) {
  /*
   * This should not happen: we got a callback,
   * but we have no buffer in deviceShadowedQueue
   * we lost buffers this way...(ERROR)
   */
  if (callback_) {
   uint32_t count;
   callback_(ctx_, ENGINE_SERVICE_MSG_RETRIEVE_DUMP_BUFS, &count);
  }
  return;
 }
 devShadowQueue_->pop();

 if (buf != &silentBuf_) {
  buf->size_ = 0;
  freeQueue_->push(buf);

  if (!playQueue_->front(&buf)) {
#ifdef ENABLE_LOG
   logFile_->log("%s", "====Warning: running out of the Audio buffers");
#endif
   return;
  }

  devShadowQueue_->push(buf);
  (*bq)->Enqueue(bq, buf->buf_, buf->size_);
  playQueue_->pop();
  return;
 }

 if (playQueue_->size() < PLAY_KICKSTART_BUFFER_COUNT) {
  (*bq)->Enqueue(bq, buf->buf_, buf->size_);
  devShadowQueue_->push(&silentBuf_);
  return;
 }

 assert(PLAY_KICKSTART_BUFFER_COUNT <=
  (DEVICE_SHADOW_BUFFER_QUEUE_LEN - devShadowQueue_->size()));
 for (int32_t idx = 0; idx < PLAY_KICKSTART_BUFFER_COUNT; idx++) {
  playQueue_->front(&buf);
  playQueue_->pop();
  devShadowQueue_->push(buf);
  (*bq)->Enqueue(bq, buf->buf_, buf->size_);
 }
}

AudioPlayer::AudioPlayer(SampleFormat *sampleFormat, SLEngineItf slEngine)
 : freeQueue_(nullptr),
 playQueue_(nullptr),
 devShadowQueue_(nullptr),
 callback_(nullptr) {
 SLresult result;
 assert(sampleFormat);
 sampleInfo_ = *sampleFormat;

 result = (*slEngine)
  ->CreateOutputMix(slEngine, &outputMixObjectItf_, 0, NULL, NULL);
 SLASSERT(result);

 // realize the output mix
 result =
  (*outputMixObjectItf_)->Realize(outputMixObjectItf_, SL_BOOLEAN_FALSE);
 SLASSERT(result);

 // configure audio source
 SLDataLocator_AndroidSimpleBufferQueue loc_bufq = {
     SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, DEVICE_SHADOW_BUFFER_QUEUE_LEN };

 SLAndroidDataFormat_PCM_EX format_pcm;
 ConvertToSLSampleFormat(&format_pcm, &sampleInfo_);
 SLDataSource audioSrc = { &loc_bufq, &format_pcm };

 // configure audio sink
 SLDataLocator_OutputMix loc_outmix = { SL_DATALOCATOR_OUTPUTMIX,
                                       outputMixObjectItf_ };
 SLDataSink audioSnk = { &loc_outmix, NULL };
 /*
  * create fast path audio player: SL_IID_BUFFERQUEUE and SL_IID_VOLUME
  * and other non-signal processing interfaces are ok.
  */
 SLInterfaceID ids[2] = { SL_IID_BUFFERQUEUE, SL_IID_VOLUME };
 SLboolean req[2] = { SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE };
 result = (*slEngine)->CreateAudioPlayer(
  slEngine, &playerObjectItf_, &audioSrc, &audioSnk,
  sizeof(ids) / sizeof(ids[0]), ids, req);
 SLASSERT(result);

 // realize the player
 result = (*playerObjectItf_)->Realize(playerObjectItf_, SL_BOOLEAN_FALSE);
 SLASSERT(result);

 // get the play interface
 result = (*playerObjectItf_)
  ->GetInterface(playerObjectItf_, SL_IID_PLAY, &playItf_);
 SLASSERT(result);

 // get the buffer queue interface
 result = (*playerObjectItf_)
  ->GetInterface(playerObjectItf_, SL_IID_BUFFERQUEUE,
   &playBufferQueueItf_);
 SLASSERT(result);

 // register callback on the buffer queue
 result = (*playBufferQueueItf_)
  ->RegisterCallback(playBufferQueueItf_, bqPlayerCallback, this);
 SLASSERT(result);

 result = (*playItf_)->SetPlayState(playItf_, SL_PLAYSTATE_STOPPED);
 SLASSERT(result);

 // create an empty queue to track deviceQueue
 devShadowQueue_ = new AudioQueue(DEVICE_SHADOW_BUFFER_QUEUE_LEN);
 assert(devShadowQueue_);

 silentBuf_.cap_ = (format_pcm.containerSize >> 3) * format_pcm.numChannels *
  sampleInfo_.framesPerBuf_;
 silentBuf_.buf_ = new uint8_t[silentBuf_.cap_];
 memset(silentBuf_.buf_, 0, silentBuf_.cap_);
 silentBuf_.size_ = silentBuf_.cap_;

#ifdef ENABLE_LOG
 std::string name = "play";
 logFile_ = new AndroidLog(name);
#endif
}

AudioPlayer::~AudioPlayer() {
 std::lock_guard<std::mutex> lock(stopMutex_);

 // destroy buffer queue audio player object, and invalidate all associated
 // interfaces
 if (playerObjectItf_ != NULL) {
  (*playerObjectItf_)->Destroy(playerObjectItf_);
 }
 // Consume all non-completed audio buffers
 sample_buf *buf = NULL;
 while (devShadowQueue_->front(&buf)) {
  buf->size_ = 0;
  devShadowQueue_->pop();
  if (buf != &silentBuf_) {
   freeQueue_->push(buf);
  }
 }
 delete devShadowQueue_;

 while (playQueue_->front(&buf)) {
  buf->size_ = 0;
  playQueue_->pop();
  freeQueue_->push(buf);
 }

 // destroy output mix object, and invalidate all associated interfaces
 if (outputMixObjectItf_) {
  (*outputMixObjectItf_)->Destroy(outputMixObjectItf_);
 }

 delete[] silentBuf_.buf_;
}

void AudioPlayer::SetBufQueue(AudioQueue *playQ, AudioQueue *freeQ) {
 playQueue_ = playQ;
 freeQueue_ = freeQ;
}

SLresult AudioPlayer::Start(void) {
 SLuint32 state;
 SLresult result = (*playItf_)->GetPlayState(playItf_, &state);
 if (result != SL_RESULT_SUCCESS) {
  return SL_BOOLEAN_FALSE;
 }
 if (state == SL_PLAYSTATE_PLAYING) {
  return SL_BOOLEAN_TRUE;
 }

 result = (*playItf_)->SetPlayState(playItf_, SL_PLAYSTATE_STOPPED);
 SLASSERT(result);

 result =
  (*playBufferQueueItf_)
  ->Enqueue(playBufferQueueItf_, silentBuf_.buf_, silentBuf_.size_);
 SLASSERT(result);
 devShadowQueue_->push(&silentBuf_);

 result = (*playItf_)->SetPlayState(playItf_, SL_PLAYSTATE_PLAYING);
 SLASSERT(result);
 return SL_BOOLEAN_TRUE;
}

void AudioPlayer::Stop(void) {
 SLuint32 state;

 SLresult result = (*playItf_)->GetPlayState(playItf_, &state);
 SLASSERT(result);

 if (state == SL_PLAYSTATE_STOPPED) return;

 std::lock_guard<std::mutex> lock(stopMutex_);

 result = (*playItf_)->SetPlayState(playItf_, SL_PLAYSTATE_STOPPED);
 SLASSERT(result);
 (*playBufferQueueItf_)->Clear(playBufferQueueItf_);

#ifdef ENABLE_LOG
 if (logFile_) {
  delete logFile_;
  logFile_ = nullptr;
 }
#endif
}

void AudioPlayer::RegisterCallback(ENGINE_CALLBACK cb, void *ctx) {
 callback_ = cb;
 ctx_ = ctx;
}

uint32_t AudioPlayer::dbgGetDevBufCount(void) {
 return (devShadowQueue_->size());
}





/*
 * bqRecorderCallback(): called for every buffer is full;
 *                       pass directly to handler
 */
void bqRecorderCallback(SLAndroidSimpleBufferQueueItf bq, void *rec) {
 (static_cast<AudioRecorder *>(rec))->ProcessSLCallback(bq);
}

void AudioRecorder::ProcessSLCallback(SLAndroidSimpleBufferQueueItf bq) {
#ifdef ENABLE_LOG
 recLog_->logTime();
#endif
 assert(bq == recBufQueueItf_);
 sample_buf *dataBuf = NULL;
 devShadowQueue_->front(&dataBuf);
 devShadowQueue_->pop();
 dataBuf->size_ = dataBuf->cap_;  // device only calls us when it is really
                                  // full

 callback_(ctx_, ENGINE_SERVICE_MSG_RECORDED_AUDIO_AVAILABLE, dataBuf);
 recQueue_->push(dataBuf);

 sample_buf *freeBuf;
 while (freeQueue_->front(&freeBuf) && devShadowQueue_->push(freeBuf)) {
  freeQueue_->pop();
  SLresult result = (*bq)->Enqueue(bq, freeBuf->buf_, freeBuf->cap_);
  SLASSERT(result);
 }

 ++audioBufCount;

 // should leave the device to sleep to save power if no buffers
 if (devShadowQueue_->size() == 0) {
  (*recItf_)->SetRecordState(recItf_, SL_RECORDSTATE_STOPPED);
 }
}

AudioRecorder::AudioRecorder(SampleFormat *sampleFormat, SLEngineItf slEngine)
 : freeQueue_(nullptr),
 recQueue_(nullptr),
 devShadowQueue_(nullptr),
 callback_(nullptr) {
 SLresult result;
 sampleInfo_ = *sampleFormat;
 SLAndroidDataFormat_PCM_EX format_pcm;
 ConvertToSLSampleFormat(&format_pcm, &sampleInfo_);

 // configure audio source
 SLDataLocator_IODevice loc_dev = { SL_DATALOCATOR_IODEVICE,
                                   SL_IODEVICE_AUDIOINPUT,
                                   SL_DEFAULTDEVICEID_AUDIOINPUT, NULL };
 SLDataSource audioSrc = { &loc_dev, NULL };

 // configure audio sink
 SLDataLocator_AndroidSimpleBufferQueue loc_bq = {
     SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, DEVICE_SHADOW_BUFFER_QUEUE_LEN };

 SLDataSink audioSnk = { &loc_bq, &format_pcm };

 // create audio recorder
 // (requires the RECORD_AUDIO permission)
 const SLInterfaceID id[2] = { SL_IID_ANDROIDSIMPLEBUFFERQUEUE,
                              SL_IID_ANDROIDCONFIGURATION };
 const SLboolean req[2] = { SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE };
 result = (*slEngine)->CreateAudioRecorder(
  slEngine, &recObjectItf_, &audioSrc, &audioSnk,
  sizeof(id) / sizeof(id[0]), id, req);
 SLASSERT(result);

 // Configure the voice recognition preset which has no
 // signal processing for lower latency.
 SLAndroidConfigurationItf inputConfig;
 result = (*recObjectItf_)
  ->GetInterface(recObjectItf_, SL_IID_ANDROIDCONFIGURATION,
   &inputConfig);
 if (SL_RESULT_SUCCESS == result) {
  SLuint32 presetValue = SL_ANDROID_RECORDING_PRESET_VOICE_RECOGNITION;
  (*inputConfig)
   ->SetConfiguration(inputConfig, SL_ANDROID_KEY_RECORDING_PRESET,
    &presetValue, sizeof(SLuint32));
 }
 result = (*recObjectItf_)->Realize(recObjectItf_, SL_BOOLEAN_FALSE);
 SLASSERT(result);
 result =
  (*recObjectItf_)->GetInterface(recObjectItf_, SL_IID_RECORD, &recItf_);
 SLASSERT(result);

 result = (*recObjectItf_)
  ->GetInterface(recObjectItf_, SL_IID_ANDROIDSIMPLEBUFFERQUEUE,
   &recBufQueueItf_);
 SLASSERT(result);

 result = (*recBufQueueItf_)
  ->RegisterCallback(recBufQueueItf_, bqRecorderCallback, this);
 SLASSERT(result);

 devShadowQueue_ = new AudioQueue(DEVICE_SHADOW_BUFFER_QUEUE_LEN);
 assert(devShadowQueue_);
#ifdef ENABLE_LOG
 std::string name = "rec";
 recLog_ = new AndroidLog(name);
#endif
}

SLboolean AudioRecorder::Start(void) {
 if (!freeQueue_ || !recQueue_ || !devShadowQueue_) {
  //LOGE("====NULL poiter to Start(%p, %p, %p)", freeQueue_, recQueue_,
  // devShadowQueue_);
  return SL_BOOLEAN_FALSE;
 }
 audioBufCount = 0;

 SLresult result;
 // in case already recording, stop recording and clear buffer queue
 result = (*recItf_)->SetRecordState(recItf_, SL_RECORDSTATE_STOPPED);
 SLASSERT(result);
 result = (*recBufQueueItf_)->Clear(recBufQueueItf_);
 SLASSERT(result);

 for (int i = 0; i < RECORD_DEVICE_KICKSTART_BUF_COUNT; i++) {
  sample_buf *buf = NULL;
  if (!freeQueue_->front(&buf)) {
   //LOGE("=====OutOfFreeBuffers @ startingRecording @ (%d)", i);
   break;
  }
  freeQueue_->pop();
  assert(buf->buf_ && buf->cap_ && !buf->size_);

  result = (*recBufQueueItf_)->Enqueue(recBufQueueItf_, buf->buf_, buf->cap_);
  SLASSERT(result);
  devShadowQueue_->push(buf);
 }

 result = (*recItf_)->SetRecordState(recItf_, SL_RECORDSTATE_RECORDING);
 SLASSERT(result);

 return (result == SL_RESULT_SUCCESS ? SL_BOOLEAN_TRUE : SL_BOOLEAN_FALSE);
}

SLboolean AudioRecorder::Stop(void) {
 // in case already recording, stop recording and clear buffer queue
 SLuint32 curState;

 SLresult result = (*recItf_)->GetRecordState(recItf_, &curState);
 SLASSERT(result);
 if (curState == SL_RECORDSTATE_STOPPED) {
  return SL_BOOLEAN_TRUE;
 }
 result = (*recItf_)->SetRecordState(recItf_, SL_RECORDSTATE_STOPPED);
 SLASSERT(result);
 result = (*recBufQueueItf_)->Clear(recBufQueueItf_);
 SLASSERT(result);

#ifdef ENABLE_LOG
 recLog_->flush();
#endif

 return SL_BOOLEAN_TRUE;
}

AudioRecorder::~AudioRecorder() {
 // destroy audio recorder object, and invalidate all associated interfaces
 if (recObjectItf_ != NULL) {
  (*recObjectItf_)->Destroy(recObjectItf_);
 }

 if (devShadowQueue_) {
  sample_buf *buf = NULL;
  while (devShadowQueue_->front(&buf)) {
   devShadowQueue_->pop();
   freeQueue_->push(buf);
  }
  delete (devShadowQueue_);
 }
#ifdef ENABLE_LOG
 if (recLog_) {
  delete recLog_;
 }
#endif
}

void AudioRecorder::SetBufQueues(AudioQueue *freeQ, AudioQueue *recQ) {
 assert(freeQ && recQ);
 freeQueue_ = freeQ;
 recQueue_ = recQ;
}

void AudioRecorder::RegisterCallback(ENGINE_CALLBACK cb, void *ctx) {
 callback_ = cb;
 ctx_ = ctx;
}
int32_t AudioRecorder::dbgGetDevBufCount(void) {
 return devShadowQueue_->size();
}










}
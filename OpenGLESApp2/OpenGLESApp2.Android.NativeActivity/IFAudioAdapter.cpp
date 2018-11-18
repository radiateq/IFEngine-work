#include "IFAudioAdapter.h"

using namespace IFAudioSLES;

namespace IFAudioSLES {

 ICSLock EngineServiceBufferMutex;
 std::vector<sample_buf*> EngineServiceBuffer;

 void createSLEngine(
jint sampleRate, jint framesPerBuf,
 jlong delayInMs, jfloat decay) {
 SLresult result;
 memset(&engine, 0, sizeof(engine));

 engine.fastPathSampleRate_ = static_cast<SLmilliHertz>(sampleRate) * 1000;
 engine.fastPathFramesPerBuf_ = static_cast<uint32_t>(framesPerBuf);
 engine.sampleChannels_ = AUDIO_SAMPLE_CHANNELS;
 engine.bitsPerSample_ = SL_PCMSAMPLEFORMAT_FIXED_16;

 result = slCreateEngine(&engine.slEngineObj_, 0, NULL, 0, NULL, NULL);
 SLASSERT(result);

 result =
  (*engine.slEngineObj_)->Realize(engine.slEngineObj_, SL_BOOLEAN_FALSE);
 SLASSERT(result);

 result = (*engine.slEngineObj_)
  ->GetInterface(engine.slEngineObj_, SL_IID_ENGINE,
   &engine.slEngineItf_);
 SLASSERT(result);

 // compute the RECOMMENDED fast audio buffer size:
 //   the lower latency required
 //     *) the smaller the buffer should be (adjust it here) AND
 //     *) the less buffering should be before starting player AFTER
 //        receiving the recorder buffer
 //   Adjust the bufSize here to fit your bill [before it busts]
 uint32_t bufSize = engine.fastPathFramesPerBuf_ * engine.sampleChannels_ *
  engine.bitsPerSample_;
 bufSize = (bufSize + 7) >> 3;  // bits --> byte
 engine.bufCount_ = BUF_COUNT;
 engine.bufs_ = allocateSampleBufs(engine.bufCount_, bufSize);
 assert(engine.bufs_);

 engine.freeBufQueue_ = new AudioQueue(engine.bufCount_);
 engine.recBufQueue_ = new AudioQueue(engine.bufCount_);
 assert(engine.freeBufQueue_ && engine.recBufQueue_);
 for (uint32_t i = 0; i < engine.bufCount_; i++) {
  engine.freeBufQueue_->push(&engine.bufs_[i]);
 }

 engine.echoDelay_ = delayInMs;
 engine.echoDecay_ = decay;
 engine.delayEffect_ = new AudioDelay(
  engine.fastPathSampleRate_, engine.sampleChannels_, engine.bitsPerSample_,
  engine.echoDelay_, engine.echoDecay_);
 assert(engine.delayEffect_);

}

jboolean
configureEcho(
 jint delayInMs,
 jfloat decay) {
 engine.echoDelay_ = delayInMs;
 engine.echoDecay_ = decay;

 engine.delayEffect_->setDelayTime(delayInMs);
 engine.delayEffect_->setDecayWeight(decay);
 return JNI_FALSE;
}

jboolean createSLBufferQueueAudioPlayer() {
 SampleFormat sampleFormat;
 memset(&sampleFormat, 0, sizeof(sampleFormat));
 sampleFormat.pcmFormat_ = (uint16_t)engine.bitsPerSample_;
 sampleFormat.framesPerBuf_ = engine.fastPathFramesPerBuf_;

 // SampleFormat.representation_ = SL_ANDROID_PCM_REPRESENTATION_SIGNED_INT;
 sampleFormat.channels_ = (uint16_t)engine.sampleChannels_;
 sampleFormat.sampleRate_ = engine.fastPathSampleRate_;

 engine.player_ = new AudioPlayer(&sampleFormat, engine.slEngineItf_);
 assert(engine.player_);
 if (engine.player_ == nullptr) return JNI_FALSE;

 engine.player_->SetBufQueue(engine.recBufQueue_, engine.freeBufQueue_);
 engine.player_->RegisterCallback(EngineService, (void *)&engine);

 return JNI_TRUE;
}

void deleteSLBufferQueueAudioPlayer() {
 if (engine.player_) {
  delete engine.player_;
  engine.player_ = nullptr;
 }
}

jboolean createAudioRecorder() {
 SampleFormat sampleFormat;
 memset(&sampleFormat, 0, sizeof(sampleFormat));
 sampleFormat.pcmFormat_ = static_cast<uint16_t>(engine.bitsPerSample_);

 // SampleFormat.representation_ = SL_ANDROID_PCM_REPRESENTATION_SIGNED_INT;
 sampleFormat.channels_ = engine.sampleChannels_;
 sampleFormat.sampleRate_ = engine.fastPathSampleRate_;
 sampleFormat.framesPerBuf_ = engine.fastPathFramesPerBuf_;
 engine.recorder_ = new AudioRecorder(&sampleFormat, engine.slEngineItf_);
 if (!engine.recorder_) {
  return JNI_FALSE;
 }
 engine.recorder_->SetBufQueues(engine.freeBufQueue_, engine.recBufQueue_);
 engine.recorder_->RegisterCallback(EngineService, (void *)&engine);
 return JNI_TRUE;
}

 void deleteAudioRecorder() {
 if (engine.recorder_) delete engine.recorder_;

 engine.recorder_ = nullptr;
}

void startPlay() {
 engine.frameCount_ = 0;
 /*
  * start player: make it into waitForData state
  */
 if (SL_BOOLEAN_FALSE == engine.player_->Start()) {
//  LOGE("====%s failed", __FUNCTION__);
  return;
 }
 engine.recorder_->Start();
}
//
void stopPlay() {
 engine.recorder_->Stop();
 engine.player_->Stop();

 delete engine.recorder_;
 delete engine.player_;
 engine.recorder_ = NULL;
 engine.player_ = NULL;
}
//
void MainActivity_deleteSLEngine() {
 delete engine.recBufQueue_;
 delete engine.freeBufQueue_;
 releaseSampleBufs(engine.bufs_, engine.bufCount_);
 if (engine.slEngineObj_ != NULL) {
  (*engine.slEngineObj_)->Destroy(engine.slEngineObj_);
  engine.slEngineObj_ = NULL;
  engine.slEngineItf_ = NULL;
 }

 if (engine.delayEffect_) {
  delete engine.delayEffect_;
  engine.delayEffect_ = nullptr;
 }
}
//
//uint32_t dbgEngineGetBufCount(void) {
// uint32_t count = engine.player_->dbgGetDevBufCount();
// count += engine.recorder_->dbgGetDevBufCount();
// count += engine.freeBufQueue_->size();
// count += engine.recBufQueue_->size();
//
// LOGE(
//  "Buf Disrtibutions: PlayerDev=%d, RecDev=%d, FreeQ=%d, "
//  "RecQ=%d",
//  engine.player_->dbgGetDevBufCount(),
//  engine.recorder_->dbgGetDevBufCount(), engine.freeBufQueue_->size(),
//  engine.recBufQueue_->size());
// if (count != engine.bufCount_) {
//  LOGE("====Lost Bufs among the queue(supposed = %d, found = %d)", BUF_COUNT,
//   count);
// }
// return count;
//}
//
///*
// * simple message passing for player/recorder to communicate with engine
// */
bool EngineService(void *ctx, uint32_t msg, void *data) {
 assert(ctx == &engine);
 switch (msg) {
 case ENGINE_SERVICE_MSG_RETRIEVE_DUMP_BUFS: {
  //*(static_cast<uint32_t *>(data)) = dbgEngineGetBufCount();
  break;
 }
 case ENGINE_SERVICE_MSG_RECORDED_AUDIO_AVAILABLE: {
  // adding audio delay effect
  sample_buf *buf = static_cast<sample_buf *>(data);
  assert(engine.fastPathFramesPerBuf_ ==
   buf->size_ / engine.sampleChannels_ / (engine.bitsPerSample_ / 8));
  //engine.delayEffect_->process(reinterpret_cast<int16_t *>(buf->buf_),
  // engine.fastPathFramesPerBuf_);

  //TODO:  NEVER RELEASE BUFFERS BUT ALWAYS ALTERNATE BETWEEN TWO OR CYCLE THREW MORE
  //TODO: Determine sample frequency using jni
  //TODO whenever using audio functions make sure audio is initialized
  //TODO when creating/deleting, make sure to track if there is anything to delete or if we are running more than once
  sample_buf *new_buf = allocateSampleBufs(2, buf->size_);
  uint32_t allocSize = (buf->size_ + 3) & ~3;
  memcpy( new_buf->buf_, buf->buf_, allocSize);
  new_buf->cap_ = buf->cap_;
  new_buf->size_ = buf->size_;  
  EngineServiceBufferMutex.EnterAndLock();
  EngineServiceBuffer.push_back(new_buf);
  EngineServiceBufferMutex.LeaveAndUnlock();

  break;
 }
 default:
  assert(false);
  return false;
 }

 return true;
}
}
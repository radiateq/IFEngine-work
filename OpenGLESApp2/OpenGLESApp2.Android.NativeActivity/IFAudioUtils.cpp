#include "IFAudioUtils.h"

using namespace IFAudioSLES;
namespace IFAudioSLES {
void ConvertToSLSampleFormat(SLAndroidDataFormat_PCM_EX* pFormat,
 SampleFormat* pSampleInfo_) {
 assert(pFormat);
 memset(pFormat, 0, sizeof(*pFormat));

 pFormat->formatType = SL_DATAFORMAT_PCM;
 // Only support 2 channels
 // For channelMask, refer to wilhelm/src/android/channels.c for details
 if (pSampleInfo_->channels_ <= 1) {
  pFormat->numChannels = 1;
  pFormat->channelMask = SL_SPEAKER_FRONT_LEFT;
 }
 else {
  pFormat->numChannels = 2;
  pFormat->channelMask = SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT;
 }
 pFormat->sampleRate = pSampleInfo_->sampleRate_;

 pFormat->endianness = SL_BYTEORDER_LITTLEENDIAN;
 pFormat->bitsPerSample = pSampleInfo_->pcmFormat_;
 pFormat->containerSize = pSampleInfo_->pcmFormat_;

 /*
  * fixup for android extended representations...
  */
 pFormat->representation = pSampleInfo_->representation_;
 switch (pFormat->representation) {
 case SL_ANDROID_PCM_REPRESENTATION_UNSIGNED_INT:
  pFormat->bitsPerSample = SL_PCMSAMPLEFORMAT_FIXED_8;
  pFormat->containerSize = SL_PCMSAMPLEFORMAT_FIXED_8;
  pFormat->formatType = SL_ANDROID_DATAFORMAT_PCM_EX;
  break;
 case SL_ANDROID_PCM_REPRESENTATION_SIGNED_INT:
  pFormat->bitsPerSample =
   SL_PCMSAMPLEFORMAT_FIXED_16;  // supports 16, 24, and 32
  pFormat->containerSize = SL_PCMSAMPLEFORMAT_FIXED_16;
  pFormat->formatType = SL_ANDROID_DATAFORMAT_PCM_EX;
  break;
 case SL_ANDROID_PCM_REPRESENTATION_FLOAT:
  pFormat->bitsPerSample = SL_PCMSAMPLEFORMAT_FIXED_32;
  pFormat->containerSize = SL_PCMSAMPLEFORMAT_FIXED_32;
  pFormat->formatType = SL_ANDROID_DATAFORMAT_PCM_EX;
  break;
 case 0:
  break;
 default:
  assert(0);
 }
}





/*
 * Mixing Audio in integer domain to avoid FP calculation
 *   (FG * ( MixFactor * 16 ) + BG * ( (1.0f-MixFactor) * 16 )) / 16
 */
static const int32_t kFloatToIntMapFactor = 128;
static const uint32_t kMsPerSec = 1000;
/**
 * Constructor for AudioDelay
 * @param sampleRate
 * @param channelCount
 * @param format
 * @param delayTimeInMs
 */
AudioDelay::AudioDelay(int32_t sampleRate, int32_t channelCount,
                       SLuint32 format, size_t delayTimeInMs,
                       float decayWeight)
    : AudioFormat(sampleRate, channelCount, format),
      delayTime_(delayTimeInMs),
      decayWeight_(decayWeight) {
  feedbackFactor_ = static_cast<int32_t>(decayWeight_ * kFloatToIntMapFactor);
  liveAudioFactor_ = kFloatToIntMapFactor - feedbackFactor_;
  allocateBuffer();
}

/**
 * Destructor
 */
AudioDelay::~AudioDelay() {
  if (buffer_) delete static_cast<uint8_t*>(buffer_);
}

/**
 * Configure for delay time ( in miliseconds ), dynamically adjustable
 * @param delayTimeInMS in miliseconds
 * @return true if delay time is set successfully
 */
bool AudioDelay::setDelayTime(size_t delayTimeInMS) {
  if (delayTimeInMS == delayTime_) return true;

  std::lock_guard<std::mutex> lock(lock_);

  if (buffer_) {
    delete static_cast<uint8_t*>(buffer_);
    buffer_ = nullptr;
  }

  delayTime_ = delayTimeInMS;
  allocateBuffer();
  return buffer_ != nullptr;
}

/**
 * Internal helper function to allocate buffer for the delay
 *  - calculate the buffer size for the delay time
 *  - allocate and zero out buffer (0 means silent audio)
 *  - configure bufSize_ to be size of audioFrames
 */
void AudioDelay::allocateBuffer(void) {
  float floatDelayTime = (float)delayTime_ / kMsPerSec;
  float fNumFrames = floatDelayTime * (float)sampleRate_ / kMsPerSec;
  size_t sampleCount = static_cast<uint32_t>(fNumFrames + 0.5f) * channelCount_;

  uint32_t bytePerSample = format_ / 8;
  assert(bytePerSample <= 4 && bytePerSample);

  uint32_t bytePerFrame = channelCount_ * bytePerSample;

  // get bufCapacity in bytes
  bufCapacity_ = sampleCount * bytePerSample;
  bufCapacity_ =
      ((bufCapacity_ + bytePerFrame - 1) / bytePerFrame) * bytePerFrame;

  buffer_ = new uint8_t[bufCapacity_];
  assert(buffer_);

  memset(buffer_, 0, bufCapacity_);
  curPos_ = 0;

  // bufSize_ is in Frames ( not samples, not bytes )
  bufSize_ = bufCapacity_ / bytePerFrame;
}

size_t AudioDelay::getDelayTime(void) const { return delayTime_; }

/**
 * setDecayWeight(): set the decay factor
 * ratio: value of 0.0 -- 1.0f;
 *
 * the calculation is in integer ( not in float )
 * for performance purpose
 */
void AudioDelay::setDecayWeight(float weight) {
  if (weight > 0.0f && weight < 1.0f) {
    float feedback = (weight * kFloatToIntMapFactor + 0.5f);
    feedbackFactor_ = static_cast<int32_t>(feedback);
    liveAudioFactor_ = kFloatToIntMapFactor - feedbackFactor_;
  }
}

float AudioDelay::getDecayWeight(void) const { return decayWeight_; }

/**
 * process() filter live audio with "echo" effect:
 *   delay time is run-time adjustable
 *   decay time could also be adjustable, but not used
 *   in this sample, hardcoded to .5
 *
 * @param liveAudio is recorded audio stream
 * @param channelCount for liveAudio, must be 2 for stereo
 * @param numFrames is length of liveAudio in Frames ( not in byte )
 */
void AudioDelay::process(int16_t* liveAudio, int32_t numFrames) {
  if (feedbackFactor_ == 0 || bufSize_ < numFrames) {
    return;
  }

  if (!lock_.try_lock()) {
    return;
  }

  if (numFrames + curPos_ > bufSize_) {
    curPos_ = 0;
  }

  // process every sample
  int32_t sampleCount = channelCount_ * numFrames;
  int16_t* samples = &static_cast<int16_t*>(buffer_)[curPos_ * channelCount_];
  for (size_t idx = 0; idx < sampleCount; idx++) {
#if 1
    int32_t curSample =
        (samples[idx] * feedbackFactor_ + liveAudio[idx] * liveAudioFactor_) /
        kFloatToIntMapFactor;
    if (curSample > SHRT_MAX)
      curSample = SHRT_MAX;
    else if (curSample < SHRT_MIN)
      curSample = SHRT_MIN;
 
    liveAudio[idx] = samples[idx];
    samples[idx] = static_cast<int16_t>(curSample);
#else
    // Pure delay
    int16_t tmp = liveAudio[idx];
    liveAudio[idx] = samples[idx];
    samples[idx] = tmp;
#endif
  }

  curPos_ += numFrames;
  lock_.unlock();
}




void UpdateDeviceAudioProperties(ANativeActivity *activity, jint &_sample_rate, jint &_frames_per_buffer) {

 //Attach current string to java virtual machine
 JavaVM* lJavaVM = activity->vm;
 jobject lNativeActivity = activity->clazz;
 JNIEnv* env = activity->env;
 JavaVMAttachArgs lJavaVMAttachArgs;
 lJavaVMAttachArgs.version = JNI_VERSION_1_6;
 lJavaVMAttachArgs.name = "NativeThread";
 lJavaVMAttachArgs.group = NULL;
 jint lResult = lJavaVM->AttachCurrentThread(&env, &lJavaVMAttachArgs);
 if (lResult == JNI_ERR)
  return;


 //Get context of the applications
 jclass context = env->FindClass("android/content/Context");
 //Get field ID of AUDIO context
 jfieldID audioServiceField = env->GetStaticFieldID(context, "AUDIO_SERVICE", "Ljava/lang/String;");
 jstring jstr = (jstring)env->GetStaticObjectField(context, audioServiceField);
 jmethodID getSystemServiceID = env->GetMethodID(context, "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;");
 jobject audio_context = env->CallObjectMethod(lNativeActivity, getSystemServiceID, jstr);

 //Get audio manager
 jclass AudioManager = env->FindClass("android/media/AudioManager");
 jobject oAudioManager = env->AllocObject(AudioManager);
 //Get volume from audio manager
 //jmethodID getStreamVolume = env->GetMethodID(AudioManager, "getStreamVolume", "(I)I");
 //jint stream = 3; //MUSIC_STREAM = 3
 //int volume = env->CallIntMethod(audio_context, getStreamVolume, stream);
 //Get sample rate
 //Get getProperty method access
 jmethodID getStreamProperty = env->GetMethodID(AudioManager, "getProperty", "(Ljava/lang/String;)Ljava/lang/String;");
 //Get static string of audio manager
 jfieldID audioServiceProp = env->GetStaticFieldID(AudioManager, "PROPERTY_OUTPUT_SAMPLE_RATE", "Ljava/lang/String;");
 jstring AM_prop_in_str = (jstring)env->GetStaticObjectField(AudioManager, audioServiceProp);
 //Get property with ID of static strings value
 jstring AM_prop_str = (jstring)(env->CallObjectMethod(audio_context, getStreamProperty, AM_prop_in_str));
 //Now convert the Java String to C++ char array 
 char *cstring;
 const char* cstr = env->GetStringUTFChars(AM_prop_str, 0);
 cstring = new char[strnlen(cstr, 1024) + 1];
 strncpy(cstring, cstr, strnlen(cstr, 1024) + 1);
 _sample_rate = atoi(cstring);
 //Cleanup
 delete[] cstring;
 //
 env->DeleteLocalRef(AM_prop_in_str);
 env->ReleaseStringUTFChars(AM_prop_str, cstr);
 env->DeleteLocalRef(AM_prop_str);
 //Get buffers
 audioServiceProp = env->GetStaticFieldID(AudioManager, "PROPERTY_OUTPUT_FRAMES_PER_BUFFER", "Ljava/lang/String;");
 AM_prop_in_str = (jstring)env->GetStaticObjectField(AudioManager, audioServiceProp);
 AM_prop_str = (jstring)(env->CallObjectMethod(audio_context, getStreamProperty, AM_prop_in_str));
 cstr = env->GetStringUTFChars(AM_prop_str, 0);
 cstring = new char[strnlen(cstr, 1024) + 1];
 strncpy(cstring, cstr, strnlen(cstr, 1024) + 1);
 _frames_per_buffer = atoi(cstring);
 //Cleanup
 delete[] cstring;
 //
 env->DeleteLocalRef(AM_prop_in_str);
 env->ReleaseStringUTFChars(AM_prop_str, cstr);
 env->DeleteLocalRef(AM_prop_str);


 lJavaVM->DetachCurrentThread();

 ///////////////////TEMPLATE CODE START
 //JavaVM* lJavaVM = Cube_Test_Update_User_Data.state->activity->vm;
 //JNIEnv* lJNIEnv = Cube_Test_Update_User_Data.state->activity->env;

 //JavaVMAttachArgs lJavaVMAttachArgs;
 //lJavaVMAttachArgs.version = JNI_VERSION_1_6;
 //lJavaVMAttachArgs.name = "NativeThread";
 //lJavaVMAttachArgs.group = NULL;

 //jint lResult = lJavaVM->AttachCurrentThread(&lJNIEnv, &lJavaVMAttachArgs);
 //if (lResult == JNI_ERR)
 // return;


 ////jobject lNativeActivity = Cube_Test_Update_User_Data.state->activity->clazz;
 //jobject lNativeActivity = Cube_Test_Update_User_Data.state->activity->clazz;

 //jclass ClassNativeActivityTest = lJNIEnv->GetObjectClass(Cube_Test_Update_User_Data.state->activity->clazz);
 //jclass ClassNativeActivity;
 //ClassNativeActivity = lJNIEnv->FindClass("Com/Sourcejni/Audioutils/Sourcejni");
 //if(ClassNativeActivity == NULL ){
 // return;
 //}
 //jfieldID fid = lJNIEnv->GetFieldID(ClassNativeActivity, "nativeSampleRate", "Ljava/lang/String;");
 //jstring jstr = (jstring)lJNIEnv-> GetObjectField(lNativeActivity, fid);
 //const char *nativeSampleRate = lJNIEnv->GetStringUTFChars(jstr, NULL);
 ////jmethodID _method = lJNIEnv->GetMethodID(ClassNativeActivity, "SendNotification", "()V");
 ////lJNIEnv->CallVoidMethod(lNativeActivity, _method);

 //lJavaVM->DetachCurrentThread();
///////////////////TEMPLATE CODE STOP
}





}
#pragma once

#include "IFAudioSLES.h"

#include <IF_General_Utils.h>
/*
 * Copyright 2015 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

namespace IFAudioSLES {

struct EchoAudioEngine {//EchoAudioEngine is original code I stole from but not related to project in general
 SLmilliHertz fastPathSampleRate_;
 //uint32_t fastPathFramesPerBuf_;
 uint16_t sampleChannels_;
 uint16_t bitsPerSample_;

 jint sample_rate;
 jint frames_per_buffer;

 SLObjectItf slEngineObj_;
 SLEngineItf slEngineItf_;

 AudioRecorder *recorder_;
 AudioPlayer *player_;
 AudioQueue *freeBufQueue_;  // Owner of the queue
 AudioQueue *recBufQueue_;   // Owner of the queue

 sample_buf *bufs_;
 uint32_t bufCount_;
 uint32_t frameCount_;

 sample_buf *record_buffer;
 uint32_t record_buffer_count;
 uint32_t record_buffer_write_pointer;
 uint32_t record_buffer_frame_size;

 int64_t echoDelay_;
 float echoDecay_;
 AudioDelay *delayEffect_;

 bool audio_engine_playing;
};
extern bool audio_engine_created;

extern EchoAudioEngine engine;

extern IFGeneralUtils::ICSLock EngineServiceBufferMutex;
extern std::queue<uint32_t> EngineServiceBuffer;
bool EngineService(void *ctx, uint32_t msg, void *data);

void createSLEngine(
 ANativeActivity *activity,
 int64_t echoDelay_, float echoDecay_);

jboolean configureEcho(
 jint delayInMs,
 jfloat decay);

jboolean createSLBufferQueueAudioPlayer();

void deleteSLBufferQueueAudioPlayer();

jboolean createAudioRecorder();

void deleteAudioRecorder();

void startPlay();

void stopPlay();

void deleteSLEngine();


void BuildAudioEngine(ANativeActivity *);
void TearDownAudioEngine();

}


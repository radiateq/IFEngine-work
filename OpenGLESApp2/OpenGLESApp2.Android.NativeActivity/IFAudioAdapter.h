#pragma once

#include "IFAudioSLES.h"

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

struct EchoAudioEngine {
 SLmilliHertz fastPathSampleRate_;
 uint32_t fastPathFramesPerBuf_;
 uint16_t sampleChannels_;
 uint16_t bitsPerSample_;

 SLObjectItf slEngineObj_;
 SLEngineItf slEngineItf_;

 AudioRecorder *recorder_;
 AudioPlayer *player_;
 AudioQueue *freeBufQueue_;  // Owner of the queue
 AudioQueue *recBufQueue_;   // Owner of the queue

 sample_buf *bufs_;
 uint32_t bufCount_;
 uint32_t frameCount_;
 int64_t echoDelay_;
 float echoDecay_;
 AudioDelay *delayEffect_;
};
static EchoAudioEngine engine;

bool EngineService(void *ctx, uint32_t msg, void *data);

void createSLEngine(
 jint sampleRate, jint framesPerBuf,
 jlong delayInMs, jfloat decay);

jboolean configureEcho(
 jint delayInMs,
 jfloat decay);

jboolean createSLBufferQueueAudioPlayer();

void deleteSLBufferQueueAudioPlayer();

jboolean createAudioRecorder();

void deleteAudioRecorder();

void startPlay();

void stopPlay();

void MainActivity_deleteSLEngine();


}


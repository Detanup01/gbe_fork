/* Copyright (C) 2019 Mr Goldberg
   This file is part of the Goldberg Emulator

   The Goldberg Emulator is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 3 of the License, or (at your option) any later version.

   The Goldberg Emulator is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the Goldberg Emulator; if not, see
   <http://www.gnu.org/licenses/>.  */

#ifndef VOICECHAT_INCLUDE_H
#define VOICECHAT_INCLUDE_H

#include "base.h"
#include <opus/opus.h>
#include <portaudio.h>

#define SAMPLE_RATE 48000
#define CHANNELS 1
#define FRAME_SIZE 960 // 20ms @ 48kHz
#define MAX_ENCODED_SIZE 4000
#define MAX_DECODED_SIZE (FRAME_SIZE * 2 * sizeof(int16_t)) // for stereo
#define DEFAULT_BITRATE 32000

struct VoicePacket {
    uint64_t userId;
    std::vector<uint8_t> encoded;
};

class VoiceChat
{
    std::atomic<bool> isRecording{ false };
    std::atomic<bool> isPlaying{ false };

    std::mutex inputMutex;
    std::condition_variable inputCond;
    std::queue<std::vector<uint8_t>> encodedQueue;

    std::mutex playbackQueueMutex;

    std::queue<VoicePacket> playbackQueue;

    std::mutex decoderMapMutex;
    std::unordered_map<uint64_t, OpusDecoder*> decoderMap;

    OpusEncoder* encoder = nullptr;
    PaStream* inputStream = nullptr;
    PaStream* outputStream = nullptr;
    static int inputCallback(const void* input, void*, unsigned long frameCount,
        const PaStreamCallbackTimeInfo*, PaStreamCallbackFlags, void*);
    static int outputCallback(const void*, void* output, unsigned long frameCount,
        const PaStreamCallbackTimeInfo*, PaStreamCallbackFlags, void*);

public:
    bool InitVoiceSystem();

    void ShutdownVoiceSystem();

    bool StartVoiceRecording();

    void StopVoiceRecording();

    bool StartVoicePlayback();

    void StopVoicePlayback();

    EVoiceResult GetAvailableVoice(uint32_t* pcbCompressed);

    EVoiceResult GetVoice(bool bWantCompressed, void* pDestBuffer, uint32_t cbDestBufferSize, uint32_t* nBytesWritten);

    EVoiceResult DecompressVoice(const void* pCompressed, uint32_t cbCompressed,
        void* pDestBuffer, uint32_t cbDestBufferSize, uint32_t* nBytesWritten,
        uint32_t nDesiredSampleRate);

    void QueueIncomingVoice(uint64_t userId, const uint8_t* data, size_t len);
};

#endif // VOICECHAT_INCLUDE_H

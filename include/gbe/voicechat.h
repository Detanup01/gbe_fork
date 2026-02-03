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

// recording: how many mic samples are recorded in 1 second
// playback: ???
#define SAMPLE_RATE 48000
// mic/playback channels, steam only support mono mic channels
// https://partner.steamgames.com/doc/api/ISteamUser#DecompressVoice
// "The output data is raw single-channel 16-bit PCM audio. The decoder supports any sample rate from 11025 to 48000"
#define CHANNELS_RECORDING 1
// stereo output
#define CHANNELS_PLAYBACK 2
// https://partner.steamgames.com/doc/api/ISteamUser#GetVoice
// "It is recommended that you pass in an 8 kilobytes or larger destination buffer for compressed audio"
#define MAX_ENCODED_SIZE 8192
// how many mic samples to buffer (internally by Port Audio) before firing our mic callback
// >>> sample time = (1/48000) = 0.02ms
// >>> 20ms (desired callback rate) / 0.02ms (sample time) = 960 frames
#define FRAME_SIZE 960
// https://opus-codec.org/docs/html_api/group__opusdecoder.html#ga1a8b923c1041ad4976ceada237e117ba
// "[out] 	pcm 	opus_int16*: Output signal (interleaved if 2 channels). length is frame_size*channels*sizeof(opus_int16)"
// "[in] 	frame_size 	Number of samples per channel of available space in *pcm, if less than the maximum frame size (120ms) some frames can not be decoded"
// so we have to account for the worst case scenario which is a max of 120ms frame size
// >>> sample time = (1/48000) = 0.02ms
// >>> 120ms (worst callback rate) / 0.02ms (sample time) = 5760 frames
// >>> 5760 frames (worst case) / 960 frames (our case) = 6
#define MAX_FRAME_SIZE (FRAME_SIZE * 6)
#define MAX_DECODED_RECORDING_SIZE (MAX_FRAME_SIZE * CHANNELS_RECORDING)
#define MAX_DECODED_PLAYBACK_SIZE  (MAX_FRAME_SIZE * CHANNELS_PLAYBACK)

struct VoicePacket {
    uint64_t userId = 0;
    std::vector<uint8_t> encoded;
};

class VoiceChat
{
    // is PortAudio lib initialized
    std::atomic<bool> isSystemInited{ false };

    // --- recording
    std::atomic<bool> isRecording{ false };
    std::recursive_mutex inputMutex;
    std::queue<std::vector<uint8_t>> encodedQueue;
    OpusEncoder* encoder = nullptr;
    PaStream* inputStream = nullptr;
    // --- recording

    // --- playback
    std::atomic<bool> isPlaying{ false };
    std::recursive_mutex playbackQueueMutex;
    std::queue<VoicePacket> playbackQueue;
    std::recursive_mutex decoderMapMutex;
    std::unordered_map<uint64_t, OpusDecoder*> decoderMap; // TODO do we need a decoder for each user?
    PaStream* outputStream = nullptr;
    // --- playback

    void cleanupVoiceRecordingInternal();
    void cleanupPlaybackInternal();

    // recording callback
    static int inputCallback(const void* input, void*, unsigned long frameCount,
        const PaStreamCallbackTimeInfo*, PaStreamCallbackFlags, void*);

    // playback callback
    static int outputCallback(const void*, void* output, unsigned long frameCount,
        const PaStreamCallbackTimeInfo*, PaStreamCallbackFlags, void*);

public:
    VoiceChat() = default;
    ~VoiceChat();

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

    void QueueAudioPlayback(uint64_t userId, const uint8_t* data, size_t len);

    bool IsVoiceSystemInitialized() const;
    
    bool IsRecordingActive() const;
    
    bool IsPlaybackActive() const;
};

#endif // VOICECHAT_INCLUDE_H

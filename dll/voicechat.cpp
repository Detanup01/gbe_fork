#include "dll/voicechat.h"

bool VoiceChat::InitVoiceSystem() {
    static std::atomic<int> initCount{ 0 };
    if (initCount++ == 0) {
        if (Pa_Initialize() != paNoError) {
            PRINT_DEBUG("PortAudio initialization failed");
            return false;
        }
    }
    isRecording = false;
    isPlaying = false;
    encoder = nullptr;
    inputStream = nullptr;
    outputStream = nullptr;
    return true;
}

void VoiceChat::ShutdownVoiceSystem() {
    static std::atomic<int> initCount{ 1 };
    if (--initCount == 0) {
        Pa_Terminate();
    }
}

int VoiceChat::inputCallback(const void* input, void*, unsigned long frameCount,
    const PaStreamCallbackTimeInfo*, PaStreamCallbackFlags, void* data) {
    VoiceChat* chat = static_cast<VoiceChat*>(data);
    if (!input || frameCount != FRAME_SIZE || !chat->isRecording.load()) return paContinue;

    std::vector<uint8_t> encoded(MAX_ENCODED_SIZE);
    int len = opus_encode(chat->encoder, static_cast<const int16_t*>(input), frameCount,
        encoded.data(), MAX_ENCODED_SIZE);
    if (len > 0) {
        encoded.resize(len);
        {
            std::lock_guard<std::mutex> lock(chat->inputMutex);
            chat->encodedQueue.push(std::move(encoded));
        }
        chat->inputCond.notify_one();
    }
    else {
        PRINT_DEBUG("Opus encoding failed: %d", len);
    }
    return paContinue;
}

int VoiceChat::outputCallback(const void*, void* output, unsigned long frameCount,
    const PaStreamCallbackTimeInfo*, PaStreamCallbackFlags, void* data) {
    VoiceChat* chat = static_cast<VoiceChat*>(data);
    int16_t* out = static_cast<int16_t*>(output);
    memset(out, 0, frameCount * sizeof(int16_t) * 2); // support stereo output

    std::lock_guard<std::mutex> lock(chat->playbackQueueMutex);
    size_t mixCount = 0;

    while (!chat->playbackQueue.empty()) {
        VoicePacket pkt = chat->playbackQueue.front();
        chat->playbackQueue.pop();

        OpusDecoder* decoder = nullptr;
        {
            std::lock_guard<std::mutex> dlock(chat->decoderMapMutex);
            decoder = chat->decoderMap[pkt.userId];
            if (!decoder) {
                int err = 0;
                decoder = opus_decoder_create(SAMPLE_RATE, CHANNELS, &err);
                if (err != OPUS_OK || !decoder) continue;
                chat->decoderMap[pkt.userId] = decoder;
            }
        }

        int16_t tempBuffer[FRAME_SIZE] = { 0 };
        int decoded = opus_decode(decoder, pkt.encoded.data(), pkt.encoded.size(), tempBuffer, frameCount, 0);
        if (decoded > 0) {
            for (int i = 0; i < decoded; ++i) {
                out[2 * i] += tempBuffer[i] / 2;     // left
                out[2 * i + 1] += tempBuffer[i] / 2; // right
            }
            ++mixCount;
        }
    }

    return paContinue;
}

bool VoiceChat::StartVoiceRecording() {
    if (isRecording.load()) return true;
    if (!InitVoiceSystem()) return false;

    int err = 0;
    encoder = opus_encoder_create(SAMPLE_RATE, CHANNELS, OPUS_APPLICATION_VOIP, &err);
    if (!encoder || err != OPUS_OK) {
        PRINT_DEBUG("Opus encoder create failed: %d", err);
        return false;
    }

    opus_encoder_ctl(encoder, OPUS_SET_BITRATE(DEFAULT_BITRATE));

    PaStreamParameters params{};
    params.device = Pa_GetDefaultInputDevice();
    if (params.device == paNoDevice) return false;
    params.channelCount = CHANNELS;
    params.sampleFormat = paInt16;
    params.suggestedLatency = Pa_GetDeviceInfo(params.device)->defaultLowInputLatency;
    params.hostApiSpecificStreamInfo = nullptr;

    PaError paErr = Pa_OpenStream(&inputStream, &params, nullptr, SAMPLE_RATE, FRAME_SIZE,
        paClipOff, inputCallback, this);
    if (paErr != paNoError) {
        PRINT_DEBUG("Failed to open input stream: %s", Pa_GetErrorText(paErr));
        return false;
    }

    isRecording.store(true);
    Pa_StartStream(inputStream);
    return true;
}

void VoiceChat::StopVoiceRecording() {
    if (!isRecording.exchange(false)) return;
    if (inputStream) {
        Pa_StopStream(inputStream);
        Pa_CloseStream(inputStream);
        inputStream = nullptr;
    }
    if (encoder) {
        opus_encoder_destroy(encoder);
        encoder = nullptr;
    }
    ShutdownVoiceSystem();
}

bool VoiceChat::StartVoicePlayback() {
    if (isPlaying.load()) return true;
    if (!InitVoiceSystem()) return false;

    PaStreamParameters params{};
    params.device = Pa_GetDefaultOutputDevice();
    if (params.device == paNoDevice) return false;
    params.channelCount = 2; // stereo output
    params.sampleFormat = paInt16;
    params.suggestedLatency = Pa_GetDeviceInfo(params.device)->defaultLowOutputLatency;
    params.hostApiSpecificStreamInfo = nullptr;

    PaError paErr = Pa_OpenStream(&outputStream, nullptr, &params, SAMPLE_RATE, FRAME_SIZE,
        paClipOff, outputCallback, nullptr);
    if (paErr != paNoError) {
        PRINT_DEBUG("Failed to open output stream: %s", Pa_GetErrorText(paErr));
        return false;
    }

    isPlaying.store(true);
    Pa_StartStream(outputStream);
    return true;
}

void VoiceChat::StopVoicePlayback() {
    if (!isPlaying.exchange(false)) return;
    if (outputStream) {
        Pa_StopStream(outputStream);
        Pa_CloseStream(outputStream);
        outputStream = nullptr;
    }

    std::lock_guard<std::mutex> lock(decoderMapMutex);
    for (auto& [id, decoder] : decoderMap) {
        opus_decoder_destroy(decoder);
    }
    decoderMap.clear();

    ShutdownVoiceSystem();
}

EVoiceResult VoiceChat::GetAvailableVoice(uint32_t* pcbCompressed) {
    if (!pcbCompressed) return k_EVoiceResultNotInitialized;
    std::lock_guard<std::mutex> lock(inputMutex);

    if (!isRecording.load()) return k_EVoiceResultNotRecording;
    if (encodedQueue.empty()) return k_EVoiceResultNoData;

    *pcbCompressed = static_cast<uint32_t>(encodedQueue.front().size());
    return k_EVoiceResultOK;
}

EVoiceResult VoiceChat::GetVoice(bool bWantCompressed, void* pDestBuffer, uint32_t cbDestBufferSize, uint32_t* nBytesWritten) {
    if (!pDestBuffer || !nBytesWritten) return k_EVoiceResultNotInitialized;

    std::unique_lock<std::mutex> lock(inputMutex);
    inputCond.wait_for(lock, std::chrono::milliseconds(20), [this] {
        return !this->encodedQueue.empty();
        });

    if (encodedQueue.empty()) return k_EVoiceResultNoData;

    auto buf = std::move(encodedQueue.front());
    encodedQueue.pop();
    lock.unlock();

    if (bWantCompressed) {
        if (cbDestBufferSize < buf.size()) return k_EVoiceResultBufferTooSmall;
        memcpy(pDestBuffer, buf.data(), buf.size());
        *nBytesWritten = static_cast<uint32_t>(buf.size());
        return k_EVoiceResultOK;
    }
    else {
        int err;
        OpusDecoder* tempDecoder = opus_decoder_create(SAMPLE_RATE, CHANNELS, &err);
        if (!tempDecoder || err != OPUS_OK) return k_EVoiceResultNotInitialized;

        int16_t* pcm = static_cast<int16_t*>(pDestBuffer);
        int samples = opus_decode(tempDecoder, buf.data(), static_cast<opus_int32>(buf.size()), pcm, FRAME_SIZE, 0);
        opus_decoder_destroy(tempDecoder);

        if (samples < 0) return k_EVoiceResultNotInitialized;

        uint32_t requiredSize = samples * CHANNELS * sizeof(int16_t);
        if (cbDestBufferSize < requiredSize) return k_EVoiceResultBufferTooSmall;

        *nBytesWritten = requiredSize;
        return k_EVoiceResultOK;
    }
}

EVoiceResult VoiceChat::DecompressVoice(const void* pCompressed, uint32_t cbCompressed,
    void* pDestBuffer, uint32_t cbDestBufferSize, uint32_t* nBytesWritten,
    uint32_t nDesiredSampleRate) {
    if (!pCompressed || !pDestBuffer || !nBytesWritten) return k_EVoiceResultNotInitialized;

    int err;
    OpusDecoder* tempDecoder = opus_decoder_create(nDesiredSampleRate, CHANNELS, &err);
    if (!tempDecoder || err != OPUS_OK) return k_EVoiceResultNotInitialized;

    int16_t* pcm = static_cast<int16_t*>(pDestBuffer);
    int samples = opus_decode(tempDecoder, static_cast<const uint8_t*>(pCompressed), cbCompressed, pcm, FRAME_SIZE, 0);
    opus_decoder_destroy(tempDecoder);

    if (samples < 0) return k_EVoiceResultNotInitialized;

    uint32_t bytesRequired = samples * CHANNELS * sizeof(int16_t);
    if (cbDestBufferSize < bytesRequired) return k_EVoiceResultBufferTooSmall;

    *nBytesWritten = bytesRequired;
    return k_EVoiceResultOK;
}

// Called externally (e.g., from network thread) to enqueue received voice
void VoiceChat::QueueIncomingVoice(uint64_t userId, const uint8_t* data, size_t len) {
    if (!data || len == 0) return;
    std::lock_guard<std::mutex> lock(playbackQueueMutex);
    playbackQueue.push({ userId, std::vector<uint8_t>(data, data + len) });
}

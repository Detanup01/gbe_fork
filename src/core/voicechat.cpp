#include "voicechat.h"


void VoiceChat::cleanupVoiceRecordingInternal()
{
    if (inputStream) {
        Pa_AbortStream(inputStream);
        Pa_CloseStream(inputStream);
        inputStream = nullptr;
        PRINT_DEBUG("Closed input stream");
    }

    if (encoder) {
        opus_encoder_destroy(encoder);
        encoder = nullptr;
        PRINT_DEBUG("Destroyed input encoder");
    }

    // this must be in a local scope (even without the lock)
    // so that the swapped/old buffer gets destroyed
    {
        std::lock_guard lock(inputMutex);

        std::queue<std::vector<uint8_t>> empty{};
        std::swap(encodedQueue, empty);
    }

    isRecording = false;
}

void VoiceChat::cleanupPlaybackInternal()
{
    if (outputStream) {
        Pa_AbortStream(outputStream);
        Pa_CloseStream(outputStream);
        outputStream = nullptr;
        PRINT_DEBUG("Closed output stream");
    }

    {
        std::lock_guard lock(decoderMapMutex);
        for (auto& [id, decoder] : decoderMap) {
            if (decoder) {
                opus_decoder_destroy(decoder);
            }
        }
        decoderMap.clear();
    }

    // this must be in a local scope (even without the lock)
    // so that the swapped/old buffer gets destroyed
    {
        std::lock_guard lock(playbackQueueMutex);

        std::queue<VoicePacket> empty{};
        std::swap(playbackQueue, empty);
    }

    isPlaying = false;
}

// https://www.portaudio.com/docs/v19-doxydocs/paex__record_8c_source.html
int VoiceChat::inputCallback(const void* input, void*, unsigned long frameCount,
    const PaStreamCallbackTimeInfo*, PaStreamCallbackFlags, void* data) {
    auto self_ref = reinterpret_cast<VoiceChat*>(data);
    if (!input || !self_ref->isRecording) return paContinue;

    std::vector<uint8_t> encoded(MAX_ENCODED_SIZE);
    int len = opus_encode(self_ref->encoder, reinterpret_cast<const int16_t*>(input), frameCount,
        encoded.data(), encoded.size());
    if (len > 0) {
        encoded.resize(len);
        {
            std::lock_guard lock(self_ref->inputMutex);
            self_ref->encodedQueue.emplace(std::move(encoded));
        }
    }
    else {
        PRINT_DEBUG("[X] Opus encoding failed: %s", opus_strerror(len));
    }
    return paContinue;
}

int VoiceChat::outputCallback(const void*, void* output, unsigned long frameCount /* frames per 1 channel! */,
    const PaStreamCallbackTimeInfo*, PaStreamCallbackFlags, void* data) {
    auto self_ref = reinterpret_cast<VoiceChat*>(data);
    auto out = reinterpret_cast<int16_t*>(output);

    unsigned long remainingFrames = frameCount;

    while (true) {
        if (remainingFrames <= 0) break;

        VoicePacket pkt{};
        {
            std::lock_guard lock(self_ref->playbackQueueMutex);

            if (self_ref->playbackQueue.empty()) break;

            pkt = std::move(self_ref->playbackQueue.front());
            self_ref->playbackQueue.pop();
        }


        OpusDecoder* decoder = nullptr;
        {
            std::lock_guard lock(self_ref->decoderMapMutex);

            auto it_decoder = self_ref->decoderMap.find(pkt.userId);
            if (self_ref->decoderMap.end() != it_decoder) {
                decoder = it_decoder->second;
            }
            else {
                int err = 0;
                // we must decompress using the same parameters used in StartVoicePlayback() when creating the encoder
                decoder = opus_decoder_create(SAMPLE_RATE, CHANNELS_PLAYBACK, &err);
                if (err != OPUS_OK || !decoder) {
                    PRINT_DEBUG("[X] Opus decoder create failed: %s", opus_strerror(err));
                    continue;
                }

                self_ref->decoderMap[pkt.userId] = decoder;
            }
        }

        auto pcm = std::vector<opus_int16>(MAX_DECODED_PLAYBACK_SIZE);
        int samplesPerChannel = opus_decode(decoder, (const unsigned char*)pkt.encoded.data(), (int)pkt.encoded.size(),
            pcm.data(), MAX_FRAME_SIZE, 0);
        if (samplesPerChannel < 0) {
            PRINT_DEBUG("[X] Opus decode failed: %s", opus_strerror(samplesPerChannel));
            break;
        }

        if ((unsigned long)samplesPerChannel > remainingFrames) {
            samplesPerChannel = remainingFrames;
        }
        // https://opus-codec.org/docs/html_api/group__opusdecoder.html#ga1a8b923c1041ad4976ceada237e117ba
        // "[out] 	pcm 	opus_int16*: Output signal (interleaved if 2 channels). length is frame_size*channels*sizeof(opus_int16)"
        uint32_t bytesRequired = samplesPerChannel * CHANNELS_PLAYBACK * sizeof(opus_int16);
        memcpy(out, pcm.data(), bytesRequired);

        // update the pointers
        remainingFrames -= (unsigned long)samplesPerChannel;
        out += samplesPerChannel * CHANNELS_PLAYBACK;
    }

    return paContinue;
}


// --- !!! ------ !!! ------ !!! ------ !!! ------ !!! ---
// --- !!! ------ !!! ------ !!! ------ !!! ------ !!! ---
// don't init PortAudio or any other external libraries in the constructor
// always do lazy initialization, this makes it less likely to encounter
// a crash because of these external libraries if the current game isn't
// even using the Steam recording feature
// --- !!! ------ !!! ------ !!! ------ !!! ------ !!! ---
// --- !!! ------ !!! ------ !!! ------ !!! ------ !!! ---

VoiceChat::~VoiceChat()
{
    cleanupVoiceRecordingInternal();
    cleanupPlaybackInternal();
    ShutdownVoiceSystem();
}

bool VoiceChat::InitVoiceSystem() {
    if (isSystemInited) return true;

    PaError paErr = Pa_Initialize();
    if (paErr != paNoError) {
        PRINT_DEBUG("[X] PortAudio initialization failed: %s", Pa_GetErrorText(paErr));
        return false;
    }

    isSystemInited = true;
    PRINT_DEBUG("Successfully initialized VoiceSystem!");
    return true;
}

void VoiceChat::ShutdownVoiceSystem() {
    if (!isSystemInited.exchange(false)) return;

    Pa_Terminate();
    PRINT_DEBUG("VoiceSystem Terminated!");
}

bool VoiceChat::StartVoiceRecording() {
    if (isRecording) return true;
    if (!isSystemInited) {
        PRINT_DEBUG("[X] VoiceSystem not initialized");
        return false;
    }

    int err = 0;
    encoder = opus_encoder_create(SAMPLE_RATE, CHANNELS_RECORDING, OPUS_APPLICATION_VOIP, &err);
    if (!encoder || err != OPUS_OK) {
        PRINT_DEBUG("[X] Opus decoder create failed: %s", opus_strerror(err));
        cleanupVoiceRecordingInternal();
        return false;
    }

    PaStreamParameters params{};
    params.device = Pa_GetDefaultInputDevice();
    if (params.device == paNoDevice) {
        PRINT_DEBUG("[X] Pa_GetDefaultInputDevice failed (no device)");
        cleanupVoiceRecordingInternal();
        return false;
    }

    params.channelCount = CHANNELS_RECORDING;
    params.sampleFormat = paInt16;
    params.suggestedLatency = Pa_GetDeviceInfo(params.device)->defaultLowInputLatency;
    params.hostApiSpecificStreamInfo = nullptr;

    PaError paErr = Pa_OpenStream(&inputStream, &params, nullptr, SAMPLE_RATE, FRAME_SIZE,
        paClipOff, inputCallback, this);
    if (paErr != paNoError) {
        PRINT_DEBUG("[X] Failed to open input stream: %s", Pa_GetErrorText(paErr));
        cleanupVoiceRecordingInternal();
        return false;
    }

    paErr = Pa_StartStream(inputStream);
    if (paErr != paNoError) {
        PRINT_DEBUG("[X] Failed to start input stream: %s", Pa_GetErrorText(paErr));
        cleanupVoiceRecordingInternal();
        return false;
    }

    isRecording = true;
    PRINT_DEBUG("Successfully started recording!");
    return true;
}

void VoiceChat::StopVoiceRecording() {
    if (!isRecording.exchange(false)) return;

    PRINT_DEBUG_ENTRY();
    cleanupVoiceRecordingInternal();
}

bool VoiceChat::StartVoicePlayback() {
    if (isPlaying) return true;
    if (!isSystemInited) {
        PRINT_DEBUG("[X] VoiceSystem not initialized");
        return false;
    }

    PaStreamParameters params{};
    params.device = Pa_GetDefaultOutputDevice();
    if (params.device == paNoDevice) {
        PRINT_DEBUG("[X] Pa_GetDefaultInputDevice failed (no device)");
        cleanupPlaybackInternal();
        return false;
    }

    params.channelCount = CHANNELS_PLAYBACK;
    params.sampleFormat = paInt16;
    params.suggestedLatency = Pa_GetDeviceInfo(params.device)->defaultLowOutputLatency;
    params.hostApiSpecificStreamInfo = nullptr;

    PaError paErr = Pa_OpenStream(&outputStream, nullptr, &params, SAMPLE_RATE, FRAME_SIZE,
        paClipOff, outputCallback, nullptr);
    if (paErr != paNoError) {
        PRINT_DEBUG("[X] Failed to open output stream: %s", Pa_GetErrorText(paErr));
        cleanupPlaybackInternal();
        return false;
    }

    paErr = Pa_StartStream(outputStream);
    if (paErr != paNoError) {
        PRINT_DEBUG("[X] Failed to start output stream: %s", Pa_GetErrorText(paErr));
        cleanupPlaybackInternal();
        return false;
    }

    isPlaying = true;
    PRINT_DEBUG("Successfully started playback!");
    return true;
}

void VoiceChat::StopVoicePlayback() {
    if (!isPlaying.exchange(false)) return;

    PRINT_DEBUG_ENTRY();
    cleanupPlaybackInternal();
}

EVoiceResult VoiceChat::GetAvailableVoice(uint32_t* pcbCompressed) {
    // init this early since some games completely ignore the return result and use this
    if (pcbCompressed) *pcbCompressed = 0;

    if (!isSystemInited) return k_EVoiceResultNotInitialized;
    if (!isRecording) return k_EVoiceResultNotRecording;
    if (!pcbCompressed) return k_EVoiceResultBufferTooSmall;

    std::lock_guard lock(inputMutex);

    if (encodedQueue.empty()) return k_EVoiceResultNoData;

    auto availableBytes = static_cast<uint32_t>(encodedQueue.front().size());
    *pcbCompressed = availableBytes;
    PRINT_DEBUG("available %u bytes of voice data", availableBytes);
    return k_EVoiceResultOK;
}

EVoiceResult VoiceChat::GetVoice(bool bWantCompressed, void* pDestBuffer, uint32_t cbDestBufferSize, uint32_t* nBytesWritten) {
    // init this early since some games completely ignore the return result and use this
    if (nBytesWritten) *nBytesWritten = 0;

    if (!isSystemInited) return k_EVoiceResultNotInitialized;
    if (!isRecording) return k_EVoiceResultNotRecording;
    if (!pDestBuffer || !nBytesWritten) return k_EVoiceResultBufferTooSmall;

    std::lock_guard lock(inputMutex);

    if (encodedQueue.empty()) return k_EVoiceResultNoData;

    auto& encodedVoice = encodedQueue.front();

    EVoiceResult ret = k_EVoiceResultOK;
    uint32_t actualWrittenBytes = 0;
    if (bWantCompressed) {
        if (cbDestBufferSize < encodedVoice.size()) {
            ret = k_EVoiceResultBufferTooSmall;
        }
        else {
            memcpy(pDestBuffer, encodedVoice.data(), encodedVoice.size());
            actualWrittenBytes = static_cast<uint32_t>(encodedVoice.size());
        }
    }
    else {
        ret = DecompressVoice(reinterpret_cast<const void*>(encodedVoice.data()), (uint32_t)encodedVoice.size(),
            pDestBuffer, cbDestBufferSize, &actualWrittenBytes, SAMPLE_RATE);
    }

    *nBytesWritten = actualWrittenBytes;

    if (k_EVoiceResultOK == ret) {
        encodedQueue.pop();
        PRINT_DEBUG("returned %u bytes of voice data", actualWrittenBytes);
    }
    else {
        PRINT_DEBUG("[X] Failed to get voice data <%i>", ret);
    }
    return ret;
}

EVoiceResult VoiceChat::DecompressVoice(const void* pCompressed, uint32_t cbCompressed,
    void* pDestBuffer, uint32_t cbDestBufferSize, uint32_t* nBytesWritten,
    uint32_t nDesiredSampleRate) {
    // init this early since some games completely ignore the return result and use this
    if (nBytesWritten) *nBytesWritten = 0;

    if (!pCompressed || !cbCompressed) return k_EVoiceResultNoData;

    int err{};
    // we must decompress using the same parameters used in StartVoiceRecording() when creating the encoder
    // so 'nDesiredSampleRate' is ignored on purpose here
    OpusDecoder* tempDecoder = opus_decoder_create(SAMPLE_RATE, CHANNELS_RECORDING, &err);
    if (!tempDecoder || err != OPUS_OK) {
        PRINT_DEBUG("[X] Opus decoder create failed: %s", opus_strerror(err));
        return k_EVoiceResultDataCorrupted;
    }

    auto pcm = std::vector<opus_int16>(MAX_DECODED_RECORDING_SIZE);
    int samplesPerChannel = opus_decode(tempDecoder, static_cast<const unsigned char*>(pCompressed), (int)cbCompressed,
        pcm.data(), MAX_FRAME_SIZE, 0);
    opus_decoder_destroy(tempDecoder);

    if (samplesPerChannel < 0) {
        PRINT_DEBUG("[X] Opus decode failed: %s", opus_strerror(samplesPerChannel));
        return k_EVoiceResultDataCorrupted;
    }

    // https://opus-codec.org/docs/html_api/group__opusdecoder.html#ga1a8b923c1041ad4976ceada237e117ba
    // "[out] 	pcm 	opus_int16*: Output signal (interleaved if 2 channels). length is frame_size*channels*sizeof(opus_int16)"
    uint32_t bytesRequired = samplesPerChannel * CHANNELS_RECORDING * sizeof(opus_int16);
    PRINT_DEBUG("required=%u bytes, buffer size=%u bytes", bytesRequired, cbDestBufferSize);
    // https://partner.steamgames.com/doc/api/ISteamUser#DecompressVoice
    // "nBytesWritten: Returns the number of bytes written to pDestBuffer,
    // or size of the buffer required to decompress the given data
    // if cbDestBufferSize is not large enough (and k_EVoiceResultBufferTooSmall is returned)."
    if (nBytesWritten) *nBytesWritten = bytesRequired;
    if (!pDestBuffer || cbDestBufferSize < bytesRequired) return k_EVoiceResultBufferTooSmall;

    memcpy(pDestBuffer, pcm.data(), bytesRequired);
    return k_EVoiceResultOK;
}

// Called externally (e.g., from network thread) to enqueue received voice
// We usually dont need this since it actually sends the voice data by SteamNetworking (or other) with GetVoice && DecompressVoice
void VoiceChat::QueueAudioPlayback(uint64_t userId, const uint8_t* data, size_t len) {
    if (!data || len == 0) return;

    std::lock_guard lock(playbackQueueMutex);
    playbackQueue.push({ userId, std::vector<uint8_t>(data, data + len) });
}

bool VoiceChat::IsVoiceSystemInitialized() const
{
    return isSystemInited;
}

bool VoiceChat::IsRecordingActive() const
{
    return isRecording;
}

bool VoiceChat::IsPlaybackActive() const
{
    return isPlaying;
}

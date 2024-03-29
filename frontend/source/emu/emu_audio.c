#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include <psp2/kernel/processmgr.h>
#include <psp2/audioout.h>
#include <speex/speex_resampler.h>

#include "emu/emu.h"
#include "utils.h"

#define AUDIO_MAX_VOLUME 0x8000
#define AUDIO_DEFAULT_SAMPLE_RATE 48000u
#define AUDIO_OUTPUT_COUNT 256u
#define AUDIO_BUFFER_LEN(out_len) (out_len << 8)

typedef struct
{
    SpeexResamplerState *resampler_state;
    int quality;
    uint32_t in_rate;
    uint32_t out_rate;
    const int16_t *in_data;
    uint32_t in_len;
    int16_t *out_data;
    uint32_t out_len;
} AudioResampler;

typedef struct
{
    int16_t *buf_data;
    size_t buf_len;
    size_t out_len;
    size_t read_pos;
    size_t write_pos;
} AudioBuffer;

typedef struct
{
    int okay;
    int pause;
    int stop;
    int stereo;
    int samples;
    uint32_t sample_rate;
    int left_vol;
    int right_vol;
    SceUID thread_id;
    int output_port;
    int need_resample;
    AudioResampler resampler;
    AudioBuffer audio_buffer;
    SceKernelLwMutexWork mutex;
    SceKernelLwCondWork cond;
} AudioState;

static AudioState audio_state = {0};

static void AudioResamplerDeinit(AudioResampler *resampler)
{
    if (resampler)
    {
        APP_LOG("[AUDIO] Audio resampler deinit...\n");

        if (resampler->resampler_state)
        {
            speex_resampler_destroy(resampler->resampler_state);
            resampler->resampler_state = NULL;
        }
        if (resampler->out_data)
        {
            free(resampler->out_data);
            resampler->out_data = NULL;
        }
        memset(resampler, 0, sizeof(AudioResampler));

        APP_LOG("[AUDIO] Audio resampler deinit OK!\n");
    }
}

static int AudioResamplerInit(AudioResampler *resampler)
{
    APP_LOG("[AUDIO] Audio resampler init...\n");

    if (!resampler)
        goto FAILED;

    if (resampler->resampler_state)
    {
        speex_resampler_set_rate(resampler->resampler_state, resampler->in_rate, resampler->out_rate);
        speex_resampler_set_quality(resampler->resampler_state, resampler->quality);
    }
    else
    {
        resampler->resampler_state = speex_resampler_init(1, resampler->in_rate, resampler->out_rate, resampler->quality, NULL);
    }
    if (!resampler->resampler_state)
        goto FAILED;
    // speex_resampler_skip_zeros(resampler->resampler_state);

    APP_LOG("[AUDIO] Audio resampler init OK!\n");
    return 0;

FAILED:
    APP_LOG("[AUDIO] Audio resampler init failed!\n");
    Emu_DeinitAudio();
    return -1;
}

static int AudioResamplerProcessInt(AudioResampler *resampler)
{
    return speex_resampler_process_int(resampler->resampler_state, 0, resampler->in_data, &(resampler->in_len), resampler->out_data, &(resampler->out_len));
}

void Emu_CleanAudio()
{
    if (audio_state.okay)
    {
        sceKernelLockLwMutex(&audio_state.mutex, 1, NULL);
        AudioBuffer *audio_buffer = &audio_state.audio_buffer;
        memset(audio_buffer->buf_data, 0, audio_buffer->buf_len * sizeof(int16_t));
        audio_buffer->read_pos = audio_buffer->write_pos = 0;
        sceKernelUnlockLwMutex(&audio_state.mutex, 1);
    }
}

void Emu_PauseAudio()
{
    audio_state.pause = 1;
}

void Emu_ResumeAudio()
{
    audio_state.pause = 0;
}

int Emu_SetAudioVolume(int left, int right)
{
    if (!audio_state.okay)
        return -1;

    audio_state.left_vol = left < AUDIO_MAX_VOLUME ? left : AUDIO_MAX_VOLUME;
    audio_state.right_vol = right < AUDIO_MAX_VOLUME ? right : AUDIO_MAX_VOLUME;
    int vols[2] = {audio_state.left_vol, audio_state.right_vol};

    return sceAudioOutSetVolume(audio_state.output_port, SCE_AUDIO_VOLUME_FLAG_L_CH | SCE_AUDIO_VOLUME_FLAG_R_CH, vols);
}

int Emu_SetAudioConfig(int samples, uint16_t freq, int stereo)
{
    if (!audio_state.okay)
        return -1;

    audio_state.samples = samples;
    audio_state.sample_rate = freq;
    audio_state.stereo = stereo;

    return sceAudioOutSetConfig(audio_state.output_port, samples, freq, stereo);
}

static int AudioOutput(void *buf)
{
    if (!audio_state.okay)
        return -1;

    return sceAudioOutOutput(audio_state.output_port, buf);
}

static void AudioSetSampleRate(uint16_t sample_rate)
{
    uint32_t sample_rates[] = {8000, 11025, 12000, 16000, 22050, 24000, 32000, 44100, 48000};
    int n_sample_rates = sizeof(sample_rates) / sizeof(uint32_t);
    int neglect = 50;

    int i;
    for (i = 0; i < n_sample_rates; i++)
    {
        if ((sample_rate >= sample_rates[i] - neglect) && (sample_rate <= sample_rates[i] + neglect))
            break;
    }

    if (i >= n_sample_rates)
    {
        audio_state.need_resample = 1;
        audio_state.sample_rate = AUDIO_DEFAULT_SAMPLE_RATE;

        AudioResampler *resampler = &(audio_state.resampler);
        resampler->quality = 1;
        resampler->in_rate = (uint32_t)core_system_av_info.timing.sample_rate;
        resampler->out_rate = audio_state.sample_rate;
        AudioResamplerInit(resampler);
        APP_LOG("[AUDIO] Audio need resample %u to %u\n", resampler->in_rate, resampler->out_rate);
    }
    else
    {
        audio_state.need_resample = 0;
        audio_state.sample_rate = sample_rates[i];
        APP_LOG("[AUDIO] Set audio sample rate: %d\n", audio_state.sample_rate);
    }
}

int Emu_UpdateAudioSampleRate(uint32_t sample_rate)
{
    APP_LOG("[AUDIO] Update audio sample rate: %u\n", sample_rate);

    if (!audio_state.okay || sample_rate == 0)
    {
        APP_LOG("[AUDIO] Update audio sample rate failed!\n");
        return -1;
    }

    AudioSetSampleRate(sample_rate);
    int ret = Emu_SetAudioConfig(audio_state.samples, audio_state.sample_rate, audio_state.stereo);

    APP_LOG("[AUDIO] Update audio sample rate OK!\n");
    return ret;
}

static int AudioThreadEntry(SceSize args, void *argp)
{
    APP_LOG("[AUDIO] Audio thread start run.\n");

    AudioBuffer *audio_buffer = &audio_state.audio_buffer;

    while (1)
    {
        sceKernelLockLwMutex(&audio_state.mutex, 1, NULL);
        while (!audio_state.stop && ((audio_buffer->write_pos - audio_buffer->read_pos) & (audio_buffer->buf_len - 1)) < audio_buffer->out_len)
            sceKernelWaitLwCond(&audio_state.cond, NULL);
        // printf("[AUDIO] read_pos: %u, write_pos: %u, written: %u\n", audio_buffer->read_pos, audio_buffer->write_pos, ((audio_buffer->write_pos - audio_buffer->read_pos) & (audio_buffer->buf_len - 1)));
        sceKernelUnlockLwMutex(&audio_state.mutex, 1);

        if (audio_state.stop)
            break;

        AudioOutput(audio_buffer->buf_data + audio_buffer->read_pos);
        audio_buffer->read_pos = (audio_buffer->read_pos + audio_buffer->out_len) % audio_buffer->buf_len;
    }

    APP_LOG("[AUDIO] Audio thread stop run.\n");
    sceKernelExitThread(0);
    return 0;
}

static void AudioOutputDeinit()
{
    APP_LOG("[AUDIO] Audio output deinit...\n");

    AudioBuffer *audio_buffer = &audio_state.audio_buffer;

    // Deinit audio out port
    if (audio_state.output_port >= 0)
    {
        sceAudioOutReleasePort(audio_state.output_port);
        audio_state.output_port = -1;
    }

    // Deinit audio buffer
    if (audio_buffer->buf_data)
    {
        free(audio_buffer->buf_data);
        audio_buffer->buf_data = NULL;
    }
    audio_buffer->buf_len = 0;
    audio_buffer->out_len = 0;
    audio_buffer->read_pos = audio_buffer->write_pos = 0;

    APP_LOG("[AUDIO] Audio output deinit OK!\n");
}

static int AudioOutputInit()
{
    APP_LOG("[AUDIO] Audio output init...\n");

    AudioBuffer *audio_buffer = &(audio_state.audio_buffer);

    audio_state.samples = AUDIO_OUTPUT_COUNT;

    // Init audio out port
    audio_state.output_port = sceAudioOutOpenPort(SCE_AUDIO_OUT_PORT_TYPE_VOICE, audio_state.samples,
                                                  audio_state.sample_rate, audio_state.stereo);
    if (audio_state.output_port < 0)
    {
        audio_state.output_port = -1;
        goto FAILED;
    }

    // Init audio buffer
    audio_buffer->out_len = audio_state.stereo ? audio_state.samples * 2 : audio_state.samples;
    audio_buffer->buf_len = AUDIO_BUFFER_LEN(audio_buffer->out_len);
    audio_buffer->buf_data = (int16_t *)calloc(audio_buffer->buf_len, sizeof(int16_t));
    if (!audio_buffer->buf_data)
        goto FAILED;
    audio_buffer->read_pos = audio_buffer->write_pos = 0;

    APP_LOG("[AUDIO] Audio output init OK!\n");
    return 0;

FAILED:
    APP_LOG("[AUDIO] Audio output init failed!\n");
    AudioOutputDeinit();
    return -1;
}

static void AudioThreadFinish()
{
    APP_LOG("[AUDIO] Audio thread finish...\n");

    audio_state.pause = 1;
    audio_state.stop = 1;

    if (audio_state.thread_id >= 0)
    {
        sceKernelLockLwMutex(&audio_state.mutex, 1, NULL);
        sceKernelSignalLwCond(&audio_state.cond);
        sceKernelUnlockLwMutex(&audio_state.mutex, 1);

        sceKernelWaitThreadEnd(audio_state.thread_id, NULL, NULL);
        sceKernelDeleteThread(audio_state.thread_id);
        audio_state.thread_id = -1;
    }

    APP_LOG("[AUDIO] Audio thread finish OK!\n");
}

static int AudioThreadStart()
{
    APP_LOG("[AUDIO] Audio thread start...\n");

    audio_state.pause = 1;
    audio_state.stop = 0;

    audio_state.thread_id = sceKernelCreateThread("emu_audio_thread", AudioThreadEntry, 0x10000100, 0x10000, 0, 0, NULL);
    if (audio_state.thread_id < 0)
        goto FAILED;

    if (sceKernelStartThread(audio_state.thread_id, 0, NULL) < 0)
        goto FAILED;

    APP_LOG("[AUDIO] Audio thread start OK!\n");
    return 0;

FAILED:
    APP_LOG("[AUDIO] Audio thread start failed!\n");
    AudioThreadFinish();
    return -1;
}

int Emu_InitAudio()
{
    APP_LOG("[AUDIO] Audio init...\n");

    if (audio_state.okay)
        Emu_DeinitAudio();

    audio_state.okay = 0;
    audio_state.pause = 1;
    audio_state.stop = 0;
    audio_state.stereo = 1;
    audio_state.output_port = -1;
    audio_state.thread_id = -1;
    audio_state.left_vol = AUDIO_MAX_VOLUME;
    audio_state.right_vol = AUDIO_MAX_VOLUME;

    AudioSetSampleRate(core_system_av_info.timing.sample_rate);
    sceKernelCreateLwMutex(&audio_state.mutex, "emu_audio_mutex", 2, 0, NULL);
    sceKernelCreateLwCond(&audio_state.cond, "emu_audio_cond", 0, &audio_state.mutex, NULL);

    if (AudioOutputInit() < 0)
        goto FAILED;

    if (AudioThreadStart() < 0)
        goto FAILED;

    audio_state.okay = 1;

    APP_LOG("[AUDIO] Audio init OK!\n");
    return 0;

FAILED:
    APP_LOG("[AUDIO] Audio init failed\n");
    Emu_DeinitAudio();
    return -1;
}

int Emu_DeinitAudio()
{
    APP_LOG("[AUDIO] Audio deinit...\n");

    audio_state.okay = 0;
    audio_state.pause = 1;
    audio_state.stop = 1;

    AudioThreadFinish(); // AudioThreadFinish must be doing before AudioOutputDeinit!
    AudioOutputDeinit();
    AudioResamplerDeinit(&audio_state.resampler);
    sceKernelDeleteLwMutex(&audio_state.mutex);
    sceKernelDeleteLwCond(&audio_state.cond);

    APP_LOG("[AUDIO] Audio deinit OK!\n");

    return 0;
}

size_t Retro_AudioSampleBatchCallback(const int16_t *data, size_t frames)
{
    if (!audio_state.okay || audio_state.pause)
        return frames;

    AudioResampler *resampler = &audio_state.resampler;
    AudioBuffer *audio_buffer = &audio_state.audio_buffer;
    const int16_t *in_data = data;
    uint32_t in_len = frames * 2;
    int16_t *out_data = audio_buffer->buf_data;
    uint32_t out_len = audio_buffer->buf_len;
    int write_pos = audio_buffer->write_pos;

    if (audio_state.need_resample && audio_state.resampler.resampler_state)
    {
        resampler->in_data = in_data;
        resampler->in_len = in_len;
        resampler->out_len = in_len * audio_state.sample_rate / core_system_av_info.timing.sample_rate;
        resampler->out_data = (int16_t *)malloc(resampler->out_len * sizeof(int16_t));
        if (resampler->out_data)
        {
            AudioResamplerProcessInt(resampler);
            in_data = resampler->out_data;
            in_len = resampler->out_len;
        }
    }

    int i;
    for (i = 0; i < in_len; i++)
    {
        out_data[write_pos] = in_data[i];
        if (++write_pos >= out_len)
            write_pos = 0;
    }

    sceKernelLockLwMutex(&audio_state.mutex, 1, NULL);
    audio_buffer->write_pos = write_pos;
    sceKernelSignalLwCond(&audio_state.cond);
    sceKernelUnlockLwMutex(&audio_state.mutex, 1);

    if (resampler->out_data)
    {
        free(resampler->out_data);
        resampler->out_data = NULL;
    }

    return frames;
}

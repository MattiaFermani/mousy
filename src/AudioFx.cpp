#include "AudioFx.h"
#include <cmath>
#include <QIODevice>

AudioFx& AudioFx::instance() {
    static AudioFx fx;
    return fx;
}

AudioFx::AudioFx() {
    m_format.setSampleRate(44100);
    m_format.setChannelCount(1);
    m_format.setSampleFormat(QAudioFormat::Int16);

    QAudioDevice device = QMediaDevices::defaultAudioOutput();
    if (!device.isNull()) {
        m_sink = new QAudioSink(device, m_format, this);
    }

    // Synthesize procedural sound effects
    m_hitPcm = generateTone(1100.0f, 650.0f, 0.08f, 0.45f, true);     // crisp click / hitmarker
    m_streakPcm = generateTone(880.0f, 1760.0f, 0.12f, 0.5f, true);   // rising chime
    m_missPcm = generateTone(160.0f, 70.0f, 0.15f, 0.4f, true);       // dull thud
    m_beepPcm = generateTone(440.0f, 440.0f, 0.07f, 0.35f, false);    // 3-2-1 beep
    m_startPcm = generateTone(880.0f, 880.0f, 0.14f, 0.45f, false);   // go! beep
}

AudioFx::~AudioFx() {
    if (m_sink) {
        m_sink->stop();
    }
}

QByteArray AudioFx::generateTone(float freqStart, float freqEnd, float durationSec, float volume, bool decay) {
    int totalSamples = static_cast<int>(44100 * durationSec);
    QByteArray pcm;
    pcm.resize(totalSamples * sizeof(qint16));
    qint16 *samples = reinterpret_cast<qint16*>(pcm.data());

    float phase = 0.0f;
    for (int i = 0; i < totalSamples; ++i) {
        float t = static_cast<float>(i) / totalSamples;
        float freq = freqStart + (freqEnd - freqStart) * t;
        phase += 2.0f * static_cast<float>(M_PI) * freq / 44100.0f;

        float amp = volume;
        if (decay) {
            amp *= (1.0f - t * t); // smooth decay
        }

        float s = std::sin(phase) * amp;
        samples[i] = static_cast<qint16>(std::clamp(s * 32767.0f, -32768.0f, 32767.0f));
    }

    return pcm;
}

void AudioFx::playPcm(const QByteArray &pcm) {
    if (!m_enabled || !m_sink || pcm.isEmpty()) return;

    QBuffer *buf = new QBuffer(this);
    buf->setData(pcm);
    buf->open(QIODevice::ReadOnly);

    m_sink->stop();
    m_sink->start(buf);

    connect(m_sink, &QAudioSink::stateChanged, this, [buf](QAudio::State state) {
        if (state == QAudio::IdleState || state == QAudio::StoppedState) {
            buf->deleteLater();
        }
    });
}

void AudioFx::playHit() {
    playPcm(m_hitPcm);
}

void AudioFx::playMiss() {
    playPcm(m_missPcm);
}

void AudioFx::playStreak() {
    playPcm(m_streakPcm);
}

void AudioFx::playCountdown(bool isStart) {
    playPcm(isStart ? m_startPcm : m_beepPcm);
}

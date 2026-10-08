#include "AudioFx.h"
#include <QMediaDevices>
#include <QAudioDevice>
#include <QDebug>
#include <cmath>
#include <algorithm>

AudioFx& AudioFx::instance() {
    static AudioFx fx;
    return fx;
}

AudioFx::AudioFx() {
    m_format.setSampleRate(44100);
    m_format.setChannelCount(1);
    m_format.setSampleFormat(QAudioFormat::Int16);

    // Pre-synthesize procedural sound effects in memory
    m_hitPcm    = generateTone(1100.0f, 650.0f, 0.08f, 0.40f, true);     // crisp hitmarker
    m_streakPcm = generateTone(880.0f,  1760.0f, 0.12f, 0.45f, true);    // rising combo chime
    m_missPcm   = generateTone(160.0f,  70.0f,  0.12f, 0.35f, true);     // dull thud
    m_beepPcm   = generateTone(440.0f,  440.0f, 0.07f, 0.30f, false);    // 3-2-1 countdown beep
    m_startPcm  = generateTone(880.0f,  880.0f, 0.14f, 0.40f, false);    // GO! start tone

    ensureSink();
}

AudioFx::~AudioFx() {
    if (m_sink) {
        m_sink->stop();
        delete m_sink;
        m_sink = nullptr;
        m_io = nullptr;
    }
}

void AudioFx::setEnabled(bool enabled) {
    m_enabled = enabled;
    if (!m_enabled && m_sink) {
        m_sink->reset();
    }
}

void AudioFx::ensureSink() {
    if (m_sink && m_io && m_io->isWritable()) return;

    try {
        QAudioDevice defaultDevice = QMediaDevices::defaultAudioOutput();
        if (defaultDevice.isNull()) {
            qDebug() << "[AudioFx] No default audio output device available.";
            return;
        }

        if (!m_sink) {
            m_sink = new QAudioSink(defaultDevice, m_format, this);
            m_sink->setBufferSize(64 * 1024);
        }

        if (!m_io || !m_io->isWritable() || m_sink->error() != QAudio::NoError) {
            m_sink->reset();
            m_io = m_sink->start();
        }
    } catch (...) {
        qWarning() << "[AudioFx] Exception caught while initializing audio sink.";
        m_sink = nullptr;
        m_io = nullptr;
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
            amp *= (1.0f - t * t); // smooth decay envelope
        }

        float s = std::sin(phase) * amp;
        samples[i] = static_cast<qint16>(std::clamp(s * 32767.0f, -32768.0f, 32767.0f));
    }

    return pcm;
}

void AudioFx::playPcm(const QByteArray &pcm) {
    if (!m_enabled || pcm.isEmpty()) return;

    ensureSink();

    if (!m_sink || !m_io || !m_io->isWritable()) return;

    if (m_sink->error() != QAudio::NoError) {
        m_sink->reset();
        m_io = m_sink->start();
    }

    if (m_io && m_io->isWritable()) {
        m_io->write(pcm.constData(), pcm.size());
    }
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

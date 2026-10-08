#ifndef AUDIOFX_H
#define AUDIOFX_H

#include <QObject>
#include <QByteArray>
#include <QAudioSink>
#include <QAudioFormat>
#include <QIODevice>

class AudioFx : public QObject {
    Q_OBJECT

public:
    static AudioFx& instance();

    void playHit();
    void playMiss();
    void playStreak();
    void playCountdown(bool isStart);
    void setEnabled(bool enabled);
    bool isEnabled() const { return m_enabled; }

private:
    AudioFx();
    ~AudioFx();
    AudioFx(const AudioFx&) = delete;
    AudioFx& operator=(const AudioFx&) = delete;

    bool m_enabled = true;
    QAudioFormat m_format;
    QAudioSink *m_sink = nullptr;
    QIODevice *m_io = nullptr;

    QByteArray m_hitPcm;
    QByteArray m_missPcm;
    QByteArray m_streakPcm;
    QByteArray m_beepPcm;
    QByteArray m_startPcm;

    void ensureSink();
    QByteArray generateTone(float freqStart, float freqEnd, float durationSec, float volume, bool decay);
    void playPcm(const QByteArray &pcm);
};

#endif // AUDIOFX_H

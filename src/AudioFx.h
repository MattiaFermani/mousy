#ifndef AUDIOFX_H
#define AUDIOFX_H

#include <QObject>
#include <QByteArray>
#include <QAudioSink>
#include <QMediaDevices>
#include <QAudioFormat>
#include <QBuffer>

class AudioFx : public QObject {
    Q_OBJECT

public:
    static AudioFx& instance();

    void playHit();
    void playMiss();
    void playStreak();
    void playCountdown(bool isStart);
    void setEnabled(bool enabled) { m_enabled = enabled; }
    bool isEnabled() const { return m_enabled; }

private:
    AudioFx();
    ~AudioFx();

    bool m_enabled = true;
    QAudioFormat m_format;
    QAudioSink *m_sink = nullptr;

    QByteArray m_hitPcm;
    QByteArray m_missPcm;
    QByteArray m_streakPcm;
    QByteArray m_beepPcm;
    QByteArray m_startPcm;

    QByteArray generateTone(float freqStart, float freqEnd, float durationSec, float volume, bool decay);
    void playPcm(const QByteArray &pcm);
};

#endif // AUDIOFX_H

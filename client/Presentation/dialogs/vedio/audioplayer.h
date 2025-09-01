// ===============================================
// audio/audio_player.h
// 音频播放器
// ===============================================

#pragma once

#include <QObject>
#include <QAudioOutput>
#include <QIODevice>
#include <QQueue>
#include <QMutex>
#include "protocol.h"

class AudioPlayer : public QObject {
    Q_OBJECT

public:
    explicit AudioPlayer(QObject* parent = nullptr);
    ~AudioPlayer();

    bool startPlayback();
    void stopPlayback();
    bool isPlaying() const { return m_isPlaying; }

    void processAudioData(const QByteArray& audioData, const AudioFormatInfo& format);

signals:
    void playbackStarted();
    void playbackStopped();
    void errorOccurred(const QString& error);

private slots:
    void onAudioOutputStateChanged(QAudio::State state);

private:
    QAudioOutput* m_audioOutput;
    QIODevice* m_audioDevice;
    bool m_isPlaying;
    AudioFormatInfo m_currentFormat;
    QQueue<QByteArray> m_audioQueue;
    QMutex m_queueMutex;

    void setupAudioOutput(const AudioFormatInfo& format);
    void writeAudioData(const QByteArray& data);
};

// ===============================================
// audio/audio_capture.h
// 音频捕获器
// ===============================================

#pragma once

#include <QObject>
#include <QAudioInput>
#include <QIODevice>
#include <QTimer>
#include <QMutex>
#include "protocol.h"

class AudioCapture : public QObject {
    Q_OBJECT

public:
    explicit AudioCapture(QObject* parent = nullptr);
    ~AudioCapture();

    bool startCapture(int sampleRate = 44100, int channelCount = 2, int sampleSize = 16);
    void stopCapture();
    bool isCapturing() const { return m_isCapturing; }

    void setRoomId(uint32_t roomId) { m_roomId = roomId; }

signals:
    void audioPackaged(const QByteArray& packet);
    void captureStarted();
    void captureStopped();
    void errorOccurred(const QString& error);

private slots:
    void onAudioDataReady();
    void processAudioBuffer();

private:
    QAudioInput* m_audioInput;
    QIODevice* m_audioDevice;
    QTimer* m_audioTimer;
    bool m_isCapturing;
    uint32_t m_roomId;
    AudioFormatInfo m_audioFormat;
    QByteArray m_audioBuffer;
    QMutex m_bufferMutex;

    void setupAudioFormat(int sampleRate, int channelCount, int sampleSize);
};

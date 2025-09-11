// ===============================================
// audio/audio_capture.cpp
// 音频捕获器实现
// ===============================================

#include "audiocapture.h"
#include <QAudioDeviceInfo>
#include <QDebug>
#include <QDateTime>

AudioCapture::AudioCapture(QObject* parent)
    : QObject(parent)
    , m_audioInput(nullptr)
    , m_audioDevice(nullptr)
    , m_isCapturing(false)
    , m_roomId(0)
{
    m_audioTimer = new QTimer(this);
    m_audioTimer->setInterval(50); // 20次/秒处理音频数据
    connect(m_audioTimer, &QTimer::timeout, this, &AudioCapture::processAudioBuffer);
}

AudioCapture::~AudioCapture() {
    stopCapture();
}

void AudioCapture::setupAudioFormat(int sampleRate, int channelCount, int sampleSize) {
    m_audioFormat.sampleRate = sampleRate;
    m_audioFormat.channelCount = channelCount;
    m_audioFormat.sampleSize = sampleSize;
    m_audioFormat.codec = "audio/pcm";
    m_audioFormat.sampleType = QAudioFormat::SignedInt;
}

bool AudioCapture::startCapture(int sampleRate, int channelCount, int sampleSize) {
    if (m_isCapturing) {
        stopCapture();
    }

    setupAudioFormat(sampleRate, channelCount, sampleSize);

    QAudioFormat format;
    format.setSampleRate(m_audioFormat.sampleRate);
    format.setChannelCount(m_audioFormat.channelCount);
    format.setSampleSize(m_audioFormat.sampleSize);
    format.setCodec(m_audioFormat.codec);
    format.setSampleType(m_audioFormat.sampleType);

    QAudioDeviceInfo info = QAudioDeviceInfo::defaultInputDevice();
    if (!info.isFormatSupported(format)) {
        format = info.nearestFormat(format);
        m_audioFormat.sampleRate = format.sampleRate();
        m_audioFormat.channelCount = format.channelCount();
        m_audioFormat.sampleSize = format.sampleSize();
    }

    m_audioInput = new QAudioInput(format, this);
    m_audioDevice = m_audioInput->start();

    if (!m_audioDevice) {
        emit errorOccurred("无法启动音频输入设备");
        return false;
    }

    connect(m_audioDevice, &QIODevice::readyRead, this, &AudioCapture::onAudioDataReady);

    m_isCapturing = true;
    m_audioTimer->start();

    emit captureStarted();
    qDebug() << "Audio capture started:"
             << m_audioFormat.sampleRate << "Hz,"
             << m_audioFormat.channelCount << "channels,"
             << m_audioFormat.sampleSize << "bits";

    return true;
}

void AudioCapture::stopCapture() {
    if (m_isCapturing) {
        m_audioTimer->stop();

        if (m_audioInput) {
            m_audioInput->stop();
            delete m_audioInput;
            m_audioInput = nullptr;
        }

        m_audioDevice = nullptr;
        m_isCapturing = false;

        QMutexLocker locker(&m_bufferMutex);
        m_audioBuffer.clear();

        emit captureStopped();
        qDebug() << "Audio capture stopped";
    }
}

void AudioCapture::onAudioDataReady() {
    if (m_audioDevice && m_isCapturing) {
        QByteArray newData = m_audioDevice->readAll();
        if (!newData.isEmpty()) {
            QMutexLocker locker(&m_bufferMutex);
            m_audioBuffer.append(newData);
        }
    }
}

void AudioCapture::processAudioBuffer() {
    QMutexLocker locker(&m_bufferMutex);

    if (m_audioBuffer.size() > 4096) { // 每4KB发送一次
        QByteArray audioData = m_audioBuffer;
        m_audioBuffer.clear();
        locker.unlock();

        // 打包音频数据
        QByteArray packet = ProtocolPackager::packAudioFrame(
            m_roomId,
            audioData,
            QDateTime::currentMSecsSinceEpoch(),
            m_audioFormat
        );

        emit audioPackaged(packet);
    }
}


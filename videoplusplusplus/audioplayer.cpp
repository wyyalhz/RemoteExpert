// ===============================================
// audio/audio_player.cpp
// 音频播放器实现
// ===============================================

#include "audioplayer.h"
#include <QAudioDeviceInfo>
#include <QDebug>

AudioPlayer::AudioPlayer(QObject* parent)
    : QObject(parent)
    , m_audioOutput(nullptr)
    , m_audioDevice(nullptr)
    , m_isPlaying(false)
{
}

AudioPlayer::~AudioPlayer() {
    stopPlayback();
}

bool AudioPlayer::startPlayback() {
    if (m_isPlaying) {
        stopPlayback();
    }
    return true;
}

void AudioPlayer::stopPlayback() {
    if (m_isPlaying && m_audioOutput) {
        m_audioOutput->stop();
        delete m_audioOutput;
        m_audioOutput = nullptr;
        m_audioDevice = nullptr;
        m_isPlaying = false;

        QMutexLocker locker(&m_queueMutex);
        m_audioQueue.clear();

        emit playbackStopped();
        qDebug() << "Audio playback stopped";
    }
}

void AudioPlayer::setupAudioOutput(const AudioFormatInfo& format) {
    if (m_audioOutput) {
        stopPlayback();
    }

    QAudioFormat audioFormat;
    audioFormat.setSampleRate(format.sampleRate);
    audioFormat.setChannelCount(format.channelCount);
    audioFormat.setSampleSize(format.sampleSize);
    audioFormat.setCodec(format.codec);
    audioFormat.setSampleType(format.sampleType);

    QAudioDeviceInfo info(QAudioDeviceInfo::defaultOutputDevice());
    if (!info.isFormatSupported(audioFormat)) {
        audioFormat = info.nearestFormat(audioFormat);
        qWarning() << "Requested audio format not supported, using nearest format";
    }

    m_audioOutput = new QAudioOutput(audioFormat, this);
    connect(m_audioOutput, &QAudioOutput::stateChanged, this, &AudioPlayer::onAudioOutputStateChanged);

    m_audioDevice = m_audioOutput->start();
    if (m_audioDevice) {
        m_isPlaying = true;
        m_currentFormat = format;
        emit playbackStarted();
        qDebug() << "Audio playback started with format:"
                 << format.sampleRate << "Hz,"
                 << format.channelCount << "channels,"
                 << format.sampleSize << "bits";
    } else {
        emit errorOccurred("无法启动音频输出设备");
    }
}

void AudioPlayer::processAudioData(const QByteArray& audioData, const AudioFormatInfo& format) {
    if (!m_isPlaying || !m_audioOutput) {
        // 第一次收到音频数据时启动播放
        setupAudioOutput(format);
    }

    if (m_isPlaying && m_audioDevice) {
        // 检查格式是否匹配
        if (format.sampleRate != m_currentFormat.sampleRate ||
            format.channelCount != m_currentFormat.channelCount ||
            format.sampleSize != m_currentFormat.sampleSize) {
            // 格式不匹配，重新设置
            setupAudioOutput(format);
        }

        // 写入音频数据
        writeAudioData(audioData);
    }
}

void AudioPlayer::writeAudioData(const QByteArray& data) {
    if (m_audioDevice && m_audioOutput->state() == QAudio::ActiveState) {
        qint64 bytesWritten = m_audioDevice->write(data);
        if (bytesWritten != data.size()) {
            qWarning() << "Audio write incomplete:" << bytesWritten << "of" << data.size() << "bytes";
        }
    }
}

void AudioPlayer::onAudioOutputStateChanged(QAudio::State state) {
    switch (state) {
    case QAudio::ActiveState:
        qDebug() << "Audio output active";
        break;
    case QAudio::SuspendedState:
        qDebug() << "Audio output suspended";
        break;
    case QAudio::StoppedState:
        qDebug() << "Audio output stopped";
        if (m_audioOutput && m_audioOutput->error() != QAudio::NoError) {
            emit errorOccurred(QString("音频输出错误: %1").arg(m_audioOutput->error()));
        }
        break;
    case QAudio::IdleState:
        // 数据播放完毕，正常状态
        break;
    }
}

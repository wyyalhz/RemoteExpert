#include "audio_pipe.h"
#include <cstring>

AudioPipe1::AudioPipe1(QObject *parent) : QIODevice(parent) {}
AudioPipe1::~AudioPipe1() { close(); }

void AudioPipe1::start() {
    QMutexLocker lock(&m_mutex);
    m_buffer.clear();
    m_running = true;
    open(QIODevice::ReadOnly);
}

void AudioPipe1::stop() {
    QMutexLocker lock(&m_mutex);
    m_running = false;
    close();
    m_buffer.clear();
}

void AudioPipe1::append(const QByteArray &data) {
    QMutexLocker lock(&m_mutex);
    m_buffer.append(data);
}

qint64 AudioPipe1::readData(char *data, qint64 maxlen) {
    QMutexLocker lock(&m_mutex);
    if (!m_running || m_buffer.isEmpty()) return 0;
    qint64 len = qMin<qint64>(maxlen, m_buffer.size());
    memcpy(data, m_buffer.constData(), size_t(len));
    m_buffer.remove(0, int(len));
    return len;
}

qint64 AudioPipe1::writeData(const char *, qint64) { return -1; }

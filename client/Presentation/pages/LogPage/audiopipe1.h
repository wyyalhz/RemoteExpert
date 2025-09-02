#ifndef AUDIOPIPE_H
#define AUDIOPIPE_H

#include <QIODevice>
#include <QMutex>
#include <QByteArray>

class AudioPipe1 : public QIODevice {
    Q_OBJECT
public:
    explicit AudioPipe1(QObject *parent = nullptr);
    ~AudioPipe1() override;

    void start();
    void stop();
    void append(const QByteArray &data);

    qint64 readData(char *data, qint64 maxlen) override;
    qint64 writeData(const char *data, qint64 len) override;

private:
    QMutex m_mutex;
    QByteArray m_buffer;
    bool m_running = false;
};

#endif // AUDIOPIPE_H

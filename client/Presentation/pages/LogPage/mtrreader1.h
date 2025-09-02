// MtrReader.h
#ifndef MTRREADER_H
#define MTRREADER_H

#include <QThread>
#include <QFile>
#include <QJsonObject>
#include <QAudioFormat>
#include <atomic>

struct FrameMeta {
    QString type; // "video" | "audio" | "text"
    QJsonObject json;
};

class MtrReader1 : public QThread {
    Q_OBJECT
public:
    explicit MtrReader1(QObject *parent = nullptr);
    ~MtrReader1() override;

    void openFile(const QString &path);
    void requestStop();
    void requestPause(bool p);
    void setSpeed(double s) { m_speed = s <= 0 ? 1.0 : s; }

signals:
    void headerRead(QJsonObject header);
    void trailerRead(QJsonObject trailer);

    // 增加流信息：streamName/streamType/isScreen
    void videoFrameReady(const QImage &img, quint64 ts, const QString &streamName, int streamType, bool isScreen);
    void audioChunkReady(const QByteArray &pcm, const QAudioFormat &fmt, quint64 ts);
    void textMessageReady(const QString &line, quint64 ts);

    void progress(quint64 lastTs);
    void errorOccured(const QString &msg);

protected:
    void run() override;

private:
    bool readExact(QFile &f, char *buf, qint64 len);
    bool readUint16LE(QFile &f, quint16 &out);
    bool readUint32LE(QFile &f, quint32 &out);
    bool readUint64LE(QFile &f, quint64 &out);

    bool parseMeta(const QByteArray &raw, FrameMeta &outMeta, QString &err);

    QString m_path;
    std::atomic_bool m_stop{false};
    std::atomic_bool m_pause{false};
    double m_speed = 1.0;
};

#endif // MTRREADER_H

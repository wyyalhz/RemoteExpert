
#include "mtrreader1.h"

#include <QJsonDocument>
#include <QImage>
#include <QElapsedTimer>
#include <QThread>
#include <QVariant>

MtrReader1::MtrReader1(QObject *parent) : QThread(parent) {}
MtrReader1::~MtrReader1() {
    requestStop();
    wait(500);
}

void MtrReader1::openFile(const QString &path) {
    m_path = path;
}

void MtrReader1::requestStop() {
    m_stop = true;
}

void MtrReader1::requestPause(bool p) {
    m_pause = p;
}

bool MtrReader1::readExact(QFile &f, char *buf, qint64 len) {
    qint64 got = 0;
    while (got < len) {
        qint64 n = f.read(buf + got, len - got);
        if (n <= 0) return false;
        got += n;
    }
    return true;
}

bool MtrReader1::readUint16LE(QFile &f, quint16 &out) {
    char b[2];
    if (!readExact(f, b, 2)) return false;
    out = (quint8)b[0] | (quint16((quint8)b[1]) << 8);
    return true;
}

bool MtrReader1::readUint32LE(QFile &f, quint32 &out) {
    char b[4];
    if (!readExact(f, b, 4)) return false;
    out = (quint32(quint8(b[0]))      ) |
          (quint32(quint8(b[1])) <<  8) |
          (quint32(quint8(b[2])) << 16) |
          (quint32(quint8(b[3])) << 24);
    return true;
}

bool MtrReader1::readUint64LE(QFile &f, quint64 &out) {
    char b[8];
    if (!readExact(f, b, 8)) return false;
    out = (quint64(quint8(b[0]))      ) |
          (quint64(quint8(b[1])) <<  8) |
          (quint64(quint8(b[2])) << 16) |
          (quint64(quint8(b[3])) << 24) |
          (quint64(quint8(b[4])) << 32) |
          (quint64(quint8(b[5])) << 40) |
          (quint64(quint8(b[6])) << 48) |
          (quint64(quint8(b[7])) << 56);
    return true;
}

bool MtrReader1::parseMeta(const QByteArray &raw, FrameMeta &outMeta, QString &err) {
    QJsonParseError pe{};
    QJsonDocument doc = QJsonDocument::fromJson(raw, &pe);
    if (pe.error != QJsonParseError::NoError || !doc.isObject()) {
        err = QString("Invalid JSON metadata: %1").arg(pe.errorString());
        return false;
    }
    outMeta.json = doc.object();
    outMeta.type = outMeta.json.value("type").toString();
    return true;
}

void MtrReader1::run() {
    m_stop = false;
    m_pause = false;

    QFile f(m_path);
    if (!f.open(QIODevice::ReadOnly)) {
        emit errorOccured(QString("Failed to open file: %1").arg(m_path));
        return;
    }

    // --- Read FileHeader ---
    quint32 headerLength = 0;
    if (!readUint32LE(f, headerLength)) {
        emit errorOccured("Corrupt header length");
        return;
    }
    if (headerLength == 0 || headerLength > (50u << 20)) {
        emit errorOccured("Unreasonable header length");
        return;
    }
    QByteArray headerRaw(headerLength, 0);
    if (!readExact(f, headerRaw.data(), headerRaw.size())) {
        emit errorOccured("Failed to read header JSON");
        return;
    }
    QJsonParseError pe{};
    QJsonDocument hdoc = QJsonDocument::fromJson(headerRaw, &pe);
    if (pe.error != QJsonParseError::NoError || !hdoc.isObject()) {
        emit errorOccured("Header JSON invalid");
        return;
    }
    QJsonObject headerObj = hdoc.object();
    emit headerRead(headerObj);

    // Timing base
    quint64 baseStart = static_cast<quint64>(headerObj.value("startTime").toDouble(0.0));
    bool haveBase = baseStart != 0;

    QElapsedTimer wall; wall.start();
    quint64 firstTs = 0; // first frame ts observed

    while (!m_stop && !f.atEnd()) {
        if (m_pause) { QThread::msleep(10); continue; }

        quint16 frameType = 0;
        quint64 timestamp = 0;
        quint32 metaLen = 0;
        quint32 dataLen = 0;

        if (!readUint16LE(f, frameType)) break; // likely EOF
        if (!readUint64LE(f, timestamp)) { emit errorOccured("EOF at timestamp"); break; }
        if (!readUint32LE(f, metaLen))   { emit errorOccured("EOF at metadataLength"); break; }
        if (metaLen > (20u << 20)) { emit errorOccured("Unreasonable metadata size"); break; }
        QByteArray metaRaw(metaLen, 0);
        if (!readExact(f, metaRaw.data(), metaRaw.size())) { emit errorOccured("EOF in metadata"); break; }
        if (!readUint32LE(f, dataLen)) { emit errorOccured("EOF at dataLength"); break; }
        if (dataLen > (200u << 20)) { emit errorOccured("Unreasonable data size"); break; }
        QByteArray dataRaw;
        if (dataLen) {
            dataRaw.resize(int(dataLen));
            if (!readExact(f, dataRaw.data(), dataRaw.size())) { emit errorOccured("EOF in data"); break; }
        }

        // Trailer? (frameType == 13)
        if (frameType == 13) {
            QString err; FrameMeta fm;
            if (!parseMeta(metaRaw, fm, err)) { emit errorOccured(err); break; }
            emit trailerRead(fm.json);
            break;
        }

        QString err; FrameMeta fm;
        if (!parseMeta(metaRaw, fm, err)) { emit errorOccured(err); break; }
        if (firstTs == 0) firstTs = timestamp;
        if (!haveBase) baseStart = firstTs; // fallback to first frame time

        // realtime pacing against wall clock
        qint64 logicalMs = qint64(double(timestamp - baseStart) / m_speed);
        qint64 nowMs = wall.elapsed();
        if (logicalMs > nowMs) QThread::msleep(quint32(qMin<qint64>(logicalMs - nowMs, 50)));

        if (fm.type == "video") {
            const QString streamName = fm.json.value("streamName").toString();
            const int streamType = fm.json.value("streamType").toInt(-1);
            const bool isScreen = fm.json.value("isScreen").toBool(false);

            QImage img;
            img.loadFromData(dataRaw, "JPG"); // assume JPEG still image
            if (!img.isNull()) emit videoFrameReady(img, timestamp, streamName, streamType, isScreen);
        } else if (fm.type == "audio") {
            QAudioFormat fmt; fmt.setCodec("audio/pcm");
            fmt.setSampleRate(fm.json.value("sampleRate").toInt(44100));
            fmt.setChannelCount(fm.json.value("channelCount").toInt(2));
            int ss = fm.json.value("sampleSize").toInt(16);
            fmt.setSampleSize(ss);
            fmt.setByteOrder(QAudioFormat::LittleEndian);
            fmt.setSampleType(ss == 8 ? QAudioFormat::UnSignedInt : QAudioFormat::SignedInt);
            emit audioChunkReady(dataRaw, fmt, timestamp);
        } else if (fm.type == "text") {
            QString user = fm.json.value("userName").toString();
            QString content = fm.json.value("content").toString();
            QString line = QString("[%1] %2: %3").arg(QString::number(timestamp), user, content);
            emit textMessageReady(line, timestamp);
        }

        emit progress(timestamp);
    }
}

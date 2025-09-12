// LogMainWindow.h
#ifndef LOGMAINWINDOW_H
#define LOGMAINWINDOW_H

#include <QMainWindow>
#include <QAudioOutput>
#include <QAudioDeviceInfo>
#include <QImage>
#include "mtr_reader.h"
#include "audio_pipe.h"

namespace Ui { class LogMainWindow; }

class LogMainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit LogMainWindow(QWidget *parent = nullptr);
    ~LogMainWindow();

protected:
    void resizeEvent(QResizeEvent *e) override;

private slots:
    void openFile();
    void play();
    void pause();
    void stopPlayback();

    void onHeader(const QJsonObject &obj);
    void onTrailer(const QJsonObject &obj);

    // 新签名：携带 streamName/streamType/isScreen
    void onVideoFrame(const QImage &img, quint64 ts, const QString &streamName, int streamType, bool isScreen);
    void onAudioChunk(const QByteArray &pcm, const QAudioFormat &fmt, quint64 ts);
    void onTextLine(const QString &line, quint64 ts);
    void onProgress(quint64 ts);
    void onError(const QString &msg);

private:
    void ensureAudio(const QAudioFormat &fmt);
    void renderFrames();
    bool isRemoteStream(const QString &name, int type, bool isScreen) const;

    Ui::LogMainWindow *ui;
    MtrReader1 *m_reader = nullptr;

    QScopedPointer<QAudioOutput> m_audioOut;
    QScopedPointer<AudioPipe1> m_pipe;
    QAudioFormat m_currentFmt;

    QString m_currentPath;
    quint64 m_headerStart = 0;
    quint64 m_trailerEnd = 0;

    // 分别缓存本地/远程最后一帧
    QImage m_lastLocal;
    QImage m_lastRemote;
};

#endif // LOGMAINWINDOW_H

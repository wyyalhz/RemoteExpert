#include "log_main_window.h"
#include "ui_log_main_window.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QPixmap>
#include <QDateTime>
#include <QResizeEvent>
#include <QVariant>
#include <QJsonDocument>

static QString msToHMS(quint64 ms) {
    quint64 s = ms / 1000; quint64 m = s / 60; quint64 h = m / 60;
    return QString::asprintf("%02llu:%02llu:%02llu", h, m % 60, s % 60);
}

LogMainWindow::LogMainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::LogMainWindow) {
    ui->setupUi(this);

    connect(ui->btnOpen,  &QPushButton::clicked, this, &LogMainWindow::openFile);
    connect(ui->btnPlay,  &QPushButton::clicked, this, &LogMainWindow::play);
    connect(ui->btnPause, &QPushButton::clicked, this, &LogMainWindow::pause);
    connect(ui->btnStop,  &QPushButton::clicked, this, &LogMainWindow::stopPlayback);
}

LogMainWindow::~LogMainWindow() {
    stopPlayback();
    delete ui;
}

void LogMainWindow::resizeEvent(QResizeEvent *e) {
    QMainWindow::resizeEvent(e);
    renderFrames();
}

void LogMainWindow::openFile() {
    QString path = QFileDialog::getOpenFileName(
        this, tr("Open .mtr file"), QString(), tr("Meeting Recordings (*.mtr)"));
    if (path.isEmpty()) return;
    stopPlayback();

    m_currentPath = path;
    ui->listText->clear();
    ui->txtTrailer->clear();
    ui->videoLocalLabel->setText("Local");
    ui->videoRemoteLabel->setText("Remote");
    m_lastLocal = QImage();
    m_lastRemote = QImage();

    m_reader = new MtrReader1(this);
    connect(m_reader, &MtrReader1::headerRead,       this, &LogMainWindow::onHeader);
    connect(m_reader, &MtrReader1::trailerRead,      this, &LogMainWindow::onTrailer);
    connect(m_reader, &MtrReader1::videoFrameReady,  this, &LogMainWindow::onVideoFrame);
    connect(m_reader, &MtrReader1::audioChunkReady,  this, &LogMainWindow::onAudioChunk);
    connect(m_reader, &MtrReader1::textMessageReady, this, &LogMainWindow::onTextLine);
    connect(m_reader, &MtrReader1::progress,         this, &LogMainWindow::onProgress);
    connect(m_reader, &MtrReader1::errorOccured,     this, &LogMainWindow::onError);

    m_reader->openFile(path);
    m_reader->start();
}

void LogMainWindow::play()   { if (m_reader) m_reader->requestPause(false); }
void LogMainWindow::pause()  { if (m_reader) m_reader->requestPause(true); }

void LogMainWindow::stopPlayback() {
    if (m_reader) {
        m_reader->requestStop();
        m_reader->wait(200);
        m_reader->deleteLater();
        m_reader = nullptr;
    }
    if (m_audioOut) { m_audioOut->stop(); m_audioOut.reset(); }
    if (m_pipe) { m_pipe->stop(); m_pipe.reset(); }
}

void LogMainWindow::onHeader(const QJsonObject &obj) {
    m_headerStart = static_cast<quint64>(obj.value("startTime").toDouble(0.0));
    QStringList ss;
    ss << QString("version=%1").arg(obj.value("version").toString());
    ss << QString("format=%1").arg(obj.value("format").toString());
    if (m_headerStart)
        ss << QString("startTime=%1")
                  .arg(QDateTime::fromMSecsSinceEpoch(
                           static_cast<qint64>(m_headerStart)).toString(Qt::ISODate));
    QJsonObject audioFmt = obj.value("audioFormat").toObject();
    ss << QString("audio %1 Hz, %2 ch, %3 bit")
              .arg(audioFmt.value("sampleRate").toInt())
              .arg(audioFmt.value("channels").toInt())
              .arg(audioFmt.value("sampleSize").toInt());
    ui->lblHeader->setText("Header: " + ss.join(" | "));
}

void LogMainWindow::onTrailer(const QJsonObject &obj) {
    m_trailerEnd = static_cast<quint64>(obj.value("endTime").toDouble(0.0));
    QJsonDocument d(obj);
    ui->txtTrailer->setPlainText(QString::fromUtf8(d.toJson(QJsonDocument::Indented)));
}

bool LogMainWindow::isRemoteStream(const QString &name, int type, bool) const {
    if (type >= 0) return (type >= 2);
    return name.contains(QStringLiteral("远程"));
}

void LogMainWindow::onVideoFrame(const QImage &img, quint64,
                                 const QString &streamName,
                                 int streamType, bool isScreen) {
    if (isRemoteStream(streamName, streamType, isScreen))
        m_lastRemote = img;
    else
        m_lastLocal = img;
    renderFrames();
}

void LogMainWindow::renderFrames() {
    if (!m_lastLocal.isNull()) {
        QPixmap pm = QPixmap::fromImage(m_lastLocal);
        pm = pm.scaled(ui->videoLocalLabel->size(),
                       Qt::KeepAspectRatio, Qt::SmoothTransformation);
        ui->videoLocalLabel->setPixmap(pm);
    }
    if (!m_lastRemote.isNull()) {
        QPixmap pm = QPixmap::fromImage(m_lastRemote);
        pm = pm.scaled(ui->videoRemoteLabel->size(),
                       Qt::KeepAspectRatio, Qt::SmoothTransformation);
        ui->videoRemoteLabel->setPixmap(pm);
    }
}

void LogMainWindow::ensureAudio(const QAudioFormat &fmt) {
    if (m_audioOut && fmt == m_currentFmt) return;
    if (m_audioOut) { m_audioOut->stop(); m_audioOut.reset(); }

    QAudioDeviceInfo dev = QAudioDeviceInfo::defaultOutputDevice();
    m_currentFmt = dev.isFormatSupported(fmt) ? fmt : dev.nearestFormat(fmt);

    m_pipe.reset(new AudioPipe1(this));
    m_pipe->start();

    m_audioOut.reset(new QAudioOutput(m_currentFmt, this));
    m_audioOut->start(m_pipe.data());
}

void LogMainWindow::onAudioChunk(const QByteArray &pcm,
                                 const QAudioFormat &fmt, quint64) {
    ensureAudio(fmt);
    if (m_pipe) m_pipe->append(pcm);
}

void LogMainWindow::onTextLine(const QString &line, quint64) {
    ui->listText->addItem(line);
    ui->listText->scrollToBottom();
}

void LogMainWindow::onProgress(quint64 ts) {
    if (m_headerStart && m_trailerEnd && m_trailerEnd >= m_headerStart) {
        quint64 cur = ts > m_headerStart ? (ts - m_headerStart) : 0;
        quint64 dur = m_trailerEnd - m_headerStart;
        ui->lblTime->setText(msToHMS(cur) + " / " + msToHMS(dur));
        ui->slider->setRange(0, static_cast<int>(dur));
        ui->slider->setValue(static_cast<int>(cur));
    }
}

void LogMainWindow::onError(const QString &msg) {
    QMessageBox::warning(this, tr("Error"), msg);
}

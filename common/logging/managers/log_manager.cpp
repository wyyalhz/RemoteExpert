#include "log_manager.h"
#include <QDebug>
#include <QDir>
#include <QStandardPaths>

// 静态成员初始化
LogManager* LogManager::instance_ = nullptr;
QMutex LogManager::mutex_;

// ConsoleLogger 实现
ConsoleLogger::ConsoleLogger(QObject *parent)
    : LoggerBase(parent)
{
}

void ConsoleLogger::outputLog(LogLevel level, const QString &formattedMessage)
{
    switch (level) {
        case LogLevel::DEBUG:
            qDebug().noquote() << formattedMessage;
            break;
        case LogLevel::INFO:
            qInfo().noquote() << formattedMessage;
            break;
        case LogLevel::WARNING:
            qWarning().noquote() << formattedMessage;
            break;
        case LogLevel::ERROR:
            qCritical().noquote() << formattedMessage;
            break;
        case LogLevel::CRITICAL:
            qCritical().noquote() << formattedMessage;
            break;
    }
}

// FileLogger 实现
FileLogger::FileLogger(const QString &logFilePath, QObject *parent)
    : LoggerBase(parent)
    , logFilePath_(logFilePath)
    , logFile_(nullptr)
    , logStream_(nullptr)
{
    setLogFilePath(logFilePath);
}

FileLogger::~FileLogger()
{
    if (logStream_) {
        delete logStream_;
        logStream_ = nullptr;
    }
    if (logFile_) {
        logFile_->close();
        delete logFile_;
        logFile_ = nullptr;
    }
}

void FileLogger::setLogFilePath(const QString &path)
{
    logFilePath_ = path;
    
    // 确保日志目录存在
    QFileInfo fileInfo(path);
    QDir dir = fileInfo.absoluteDir();
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    
    // 关闭之前的文件
    if (logStream_) {
        delete logStream_;
        logStream_ = nullptr;
    }
    if (logFile_) {
        logFile_->close();
        delete logFile_;
        logFile_ = nullptr;
    }
    
    // 创建新的文件
    if (!logFilePath_.isEmpty()) {
        logFile_ = new QFile(logFilePath_);
        if (logFile_->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
            logStream_ = new QTextStream(logFile_);
            logStream_->setCodec("UTF-8");
        }
    }
}

void FileLogger::outputLog(LogLevel level, const QString &formattedMessage)
{
    Q_UNUSED(level)
    if (logStream_ && logFile_ && logFile_->isOpen()) {
        *logStream_ << formattedMessage << "\n";
        logStream_->flush();
    }
}

// LogManager 实现
LogManager::LogManager(QObject *parent)
    : QObject(parent)
    , globalLogLevel_(LogLevel::INFO)
    , consoleLogger_(nullptr)
    , fileLogger_(nullptr)
{
}

LogManager::~LogManager()
{
    cleanup();
}

LogManager* LogManager::getInstance()
{
    if (instance_ == nullptr) {
        QMutexLocker locker(&mutex_);
        if (instance_ == nullptr) {
            instance_ = new LogManager();
        }
    }
    return instance_;
}

void LogManager::initialize(LogLevel level, const QString &logFilePath)
{
    globalLogLevel_ = level;
    logFilePath_ = logFilePath;
    
    // 创建控制台日志器
    if (!consoleLogger_) {
        consoleLogger_ = new ConsoleLogger(this);
        consoleLogger_->setLogLevel(level);
    }
    
    // 创建文件日志器（如果指定了文件路径）
    if (!logFilePath.isEmpty()) {
        if (!fileLogger_) {
            fileLogger_ = new FileLogger(logFilePath, this);
            fileLogger_->setLogLevel(level);
        }
    }
    
    info(LogModule::SYSTEM, LogLayer::BUSINESS, "LogManager", "日志系统初始化完成");
}

LoggerBase* LogManager::getLogger(LogModule module, LogLayer layer)
{
    QString key = generateLoggerKey(module, layer);
    
    QMutexLocker locker(&loggersMutex_);
    
    if (!loggers_.contains(key)) {
        loggers_[key] = createLogger(module, layer);
    }
    
    return loggers_[key];
}

QString LogManager::generateLoggerKey(LogModule module, LogLayer layer) const
{
    return QString("%1_%2").arg(static_cast<int>(module)).arg(static_cast<int>(layer));
}

LoggerBase* LogManager::createLogger(LogModule module, LogLayer layer)
{
    // 创建一个复合日志器，同时输出到控制台和文件
    class CompositeLogger : public LoggerBase
    {
    public:
        CompositeLogger(LogModule module, LogLayer layer, 
                       ConsoleLogger* console, FileLogger* file, QObject* parent = nullptr)
            : LoggerBase(parent)
            , consoleLogger_(console)
            , fileLogger_(file)
        {
            setModule(module);
            setLayer(layer);
        }
        
    protected:
        void outputLog(LogLevel level, const QString &formattedMessage) override
        {
            if (consoleLogger_) {
                consoleLogger_->outputLog(level, formattedMessage);
            }
            if (fileLogger_) {
                fileLogger_->outputLog(level, formattedMessage);
            }
        }
        
    private:
        ConsoleLogger* consoleLogger_;
        FileLogger* fileLogger_;
    };
    
    return new CompositeLogger(module, layer, consoleLogger_, fileLogger_, this);
}

// 直接调用日志方法实现 - 兼容客户端调用方式
void LogManager::debug(LogModule module, LogLayer layer, const QString &message)
{
    LoggerBase* logger = getLogger(module, layer);
    if (logger) {
        logger->debug(message);
    }
}

void LogManager::info(LogModule module, LogLayer layer, const QString &message)
{
    LoggerBase* logger = getLogger(module, layer);
    if (logger) {
        logger->info(message);
    }
}

void LogManager::warning(LogModule module, LogLayer layer, const QString &message)
{
    LoggerBase* logger = getLogger(module, layer);
    if (logger) {
        logger->warning(message);
    }
}

void LogManager::error(LogModule module, LogLayer layer, const QString &message)
{
    LoggerBase* logger = getLogger(module, layer);
    if (logger) {
        logger->error(message);
    }
}

void LogManager::critical(LogModule module, LogLayer layer, const QString &message)
{
    LoggerBase* logger = getLogger(module, layer);
    if (logger) {
        logger->critical(message);
    }
}

// 带上下文的日志方法实现 - 兼容客户端调用方式
void LogManager::debug(LogModule module, LogLayer layer, const QString &context, const QString &message)
{
    LoggerBase* logger = getLogger(module, layer);
    if (logger) {
        logger->debug(context, message);
    }
}

void LogManager::info(LogModule module, LogLayer layer, const QString &context, const QString &message)
{
    LoggerBase* logger = getLogger(module, layer);
    if (logger) {
        logger->info(context, message);
    }
}

void LogManager::warning(LogModule module, LogLayer layer, const QString &context, const QString &message)
{
    LoggerBase* logger = getLogger(module, layer);
    if (logger) {
        logger->warning(context, message);
    }
}

void LogManager::error(LogModule module, LogLayer layer, const QString &context, const QString &message)
{
    LoggerBase* logger = getLogger(module, layer);
    if (logger) {
        logger->error(context, message);
    }
}

void LogManager::critical(LogModule module, LogLayer layer, const QString &context, const QString &message)
{
    LoggerBase* logger = getLogger(module, layer);
    if (logger) {
        logger->critical(context, message);
    }
}

void LogManager::setGlobalLogLevel(LogLevel level)
{
    globalLogLevel_ = level;
    
    // 更新所有日志器的级别
    QMutexLocker locker(&loggersMutex_);
    for (auto logger : loggers_) {
        if (logger) {
            logger->setLogLevel(level);
        }
    }
    
    if (consoleLogger_) {
        consoleLogger_->setLogLevel(level);
    }
    if (fileLogger_) {
        fileLogger_->setLogLevel(level);
    }
}

void LogManager::setLogFilePath(const QString &path)
{
    logFilePath_ = path;
    
    if (fileLogger_) {
        fileLogger_->setLogFilePath(path);
    } else if (!path.isEmpty()) {
        fileLogger_ = new FileLogger(path, this);
        fileLogger_->setLogLevel(globalLogLevel_);
    }
}

void LogManager::cleanup()
{
    QMutexLocker locker(&loggersMutex_);
    
    // 清理所有日志器
    for (auto logger : loggers_) {
        if (logger) {
            delete logger;
        }
    }
    loggers_.clear();
    
    // 清理控制台和文件日志器
    if (consoleLogger_) {
        delete consoleLogger_;
        consoleLogger_ = nullptr;
    }
    if (fileLogger_) {
        delete fileLogger_;
        fileLogger_ = nullptr;
    }
}

#ifndef LOG_MANAGER_H
#define LOG_MANAGER_H

#include "logger_base.h"
#include <QObject>
#include <QMap>
#include <QMutex>
#include <QFile>
#include <QTextStream>

// 控制台日志输出器
class ConsoleLogger : public LoggerBase
{
    Q_OBJECT

public:
    explicit ConsoleLogger(QObject *parent = nullptr);
    void outputLog(LogLevel level, const QString &formattedMessage) override;
};

// 文件日志输出器
class FileLogger : public LoggerBase
{
    Q_OBJECT

public:
    explicit FileLogger(const QString &logFilePath, QObject *parent = nullptr);
    ~FileLogger();

    void setLogFilePath(const QString &path);
    void outputLog(LogLevel level, const QString &formattedMessage) override;

private:
    QString logFilePath_;
    QFile *logFile_;
    QTextStream *logStream_;
};

// 日志管理器 - 单例模式
class LogManager : public QObject
{
    Q_OBJECT

public:
    static LogManager* getInstance();
    
    // 初始化日志管理器
    void initialize(LogLevel level = LogLevel::INFO, 
                   const QString &logFilePath = QString());
    
    // 获取指定模块和层级的日志器
    LoggerBase* getLogger(LogModule module, LogLayer layer);
    
    // 直接调用日志方法 - 不使用宏
    void debug(LogModule module, LogLayer layer, const QString &message);
    void info(LogModule module, LogLayer layer, const QString &message);
    void warning(LogModule module, LogLayer layer, const QString &message);
    void error(LogModule module, LogLayer layer, const QString &message);
    void critical(LogModule module, LogLayer layer, const QString &message);
    
    // 带上下文的日志方法
    void debug(LogModule module, LogLayer layer, const QString &context, const QString &message);
    void info(LogModule module, LogLayer layer, const QString &context, const QString &message);
    void warning(LogModule module, LogLayer layer, const QString &context, const QString &message);
    void error(LogModule module, LogLayer layer, const QString &context, const QString &message);
    void critical(LogModule module, LogLayer layer, const QString &context, const QString &message);
    
    // 设置全局日志级别
    void setGlobalLogLevel(LogLevel level);
    
    // 设置日志文件路径
    void setLogFilePath(const QString &path);
    
    // 清理资源
    void cleanup();

private:
    explicit LogManager(QObject *parent = nullptr);
    ~LogManager();
    
    // 生成日志器键值
    QString generateLoggerKey(LogModule module, LogLayer layer) const;
    
    // 创建新的日志器
    LoggerBase* createLogger(LogModule module, LogLayer layer);

private:
    static LogManager* instance_;
    static QMutex mutex_;
    
    LogLevel globalLogLevel_;
    QString logFilePath_;
    
    QMap<QString, LoggerBase*> loggers_;
    ConsoleLogger* consoleLogger_;
    FileLogger* fileLogger_;
    
    QMutex loggersMutex_;
};

#endif // LOG_MANAGER_H

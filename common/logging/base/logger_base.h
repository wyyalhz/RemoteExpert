#ifndef LOGGER_BASE_H
#define LOGGER_BASE_H

#include <QObject>
#include <QString>
#include <QDateTime>

// 日志级别枚举
enum class LogLevel {
    DEBUG = 0,
    INFO = 1,
    WARNING = 2,
    ERROR = 3,
    CRITICAL = 4
};

// 模块枚举（合并客户端和服务端）
enum class LogModule {
    USER = 0,
    TICKET = 1,        // 客户端原有
    WORKORDER = 2,     // 服务端原有
    DATABASE = 3,
    NETWORK = 4,
    SYSTEM = 5,
    PRESENTATION = 6   // 客户端原有
};

// 层级枚举（合并客户端和服务端）
enum class LogLayer {
    DATA = 0,
    BUSINESS = 1,
    NETWORK = 2,
    PRESENTATION = 3,  // 客户端原有
    API = 4           // 服务端原有
};

// 日志基类
class LoggerBase : public QObject
{
    Q_OBJECT

public:
    explicit LoggerBase(QObject *parent = nullptr);
    virtual ~LoggerBase();

    // 设置日志级别
    void setLogLevel(LogLevel level) { currentLevel_ = level; }
    LogLevel getLogLevel() const { return currentLevel_; }

    // 设置模块和层级
    void setModule(LogModule module) { currentModule_ = module; }
    void setLayer(LogLayer layer) { currentLayer_ = layer; }

    // 日志输出接口 - 直接调用，不使用宏
    virtual void debug(const QString &message);
    virtual void info(const QString &message);
    virtual void warning(const QString &message);
    virtual void error(const QString &message);
    virtual void critical(const QString &message);

    // 带上下文的日志输出
    virtual void debug(const QString &context, const QString &message);
    virtual void info(const QString &context, const QString &message);
    virtual void warning(const QString &context, const QString &message);
    virtual void error(const QString &context, const QString &message);
    virtual void critical(const QString &context, const QString &message);

protected:
    // 获取模块名称
    QString getModuleName() const;
    
    // 获取层级名称
    QString getLayerName() const;
    
    // 获取日志级别名称
    QString getLevelName(LogLevel level) const;
    
    // 格式化日志消息 - 格式：[yyyy-MM-dd hh:mm:ss.zzz] [模块][层级] 上下文: 消息内容
    QString formatMessage(LogLevel level, const QString &context, const QString &message) const;
    
    // 实际输出日志的虚函数，由子类实现
    virtual void outputLog(LogLevel level, const QString &formattedMessage) = 0;

private:
    LogLevel currentLevel_;
    LogModule currentModule_;
    LogLayer currentLayer_;
};

#endif // LOGGER_BASE_H

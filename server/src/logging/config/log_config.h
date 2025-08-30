#ifndef LOG_CONFIG_H
#define LOG_CONFIG_H

#include "../base/logger_base.h"
#include <QString>

// 日志配置类
class LogConfig
{
public:
    // 日志级别配置
    static LogLevel getDefaultLogLevel();
    static void setDefaultLogLevel(LogLevel level);
    
    // 日志文件配置
    static QString getDefaultLogFilePath();
    static void setDefaultLogFilePath(const QString &path);
    
    // 日志格式配置
    static bool isConsoleOutputEnabled();
    static void setConsoleOutputEnabled(bool enabled);
    
    static bool isFileOutputEnabled();
    static void setFileOutputEnabled(bool enabled);
    
    // 日志文件轮转配置
    static int getMaxLogFileSize(); // MB
    static void setMaxLogFileSize(int sizeMB);
    
    static int getMaxLogFileCount();
    static void setMaxLogFileCount(int count);
    
    // 模块级别配置
    static LogLevel getModuleLogLevel(LogModule module);
    static void setModuleLogLevel(LogModule module, LogLevel level);
    
    // 层级级别配置
    static LogLevel getLayerLogLevel(LogLayer layer);
    static void setLayerLogLevel(LogLayer layer, LogLevel level);
    
    // 从配置文件加载配置
    static bool loadFromFile(const QString &configFilePath);
    
    // 保存配置到文件
    static bool saveToFile(const QString &configFilePath);

private:
    static LogLevel defaultLogLevel_;
    static QString defaultLogFilePath_;
    static bool consoleOutputEnabled_;
    static bool fileOutputEnabled_;
    static int maxLogFileSize_;
    static int maxLogFileCount_;
    
    static QMap<LogModule, LogLevel> moduleLogLevels_;
    static QMap<LogLayer, LogLevel> layerLogLevels_;
};

#endif // LOG_CONFIG_H

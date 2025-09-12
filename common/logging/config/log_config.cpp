#include "log_config.h"
#include <QSettings>
#include <QDir>
#include <QStandardPaths>

// 静态成员初始化
LogLevel LogConfig::defaultLogLevel_ = LogLevel::INFO;
QString LogConfig::defaultLogFilePath_ = "logs/application.log";
bool LogConfig::consoleOutputEnabled_ = true;
bool LogConfig::fileOutputEnabled_ = true;
int LogConfig::maxLogFileSize_ = 10; // 10MB
int LogConfig::maxLogFileCount_ = 5;
QMap<LogModule, LogLevel> LogConfig::moduleLogLevels_;
QMap<LogLayer, LogLevel> LogConfig::layerLogLevels_;

LogLevel LogConfig::getDefaultLogLevel()
{
    return defaultLogLevel_;
}

void LogConfig::setDefaultLogLevel(LogLevel level)
{
    defaultLogLevel_ = level;
}

QString LogConfig::getDefaultLogFilePath()
{
    return defaultLogFilePath_;
}

void LogConfig::setDefaultLogFilePath(const QString &path)
{
    defaultLogFilePath_ = path;
}

bool LogConfig::isConsoleOutputEnabled()
{
    return consoleOutputEnabled_;
}

void LogConfig::setConsoleOutputEnabled(bool enabled)
{
    consoleOutputEnabled_ = enabled;
}

bool LogConfig::isFileOutputEnabled()
{
    return fileOutputEnabled_;
}

void LogConfig::setFileOutputEnabled(bool enabled)
{
    fileOutputEnabled_ = enabled;
}

int LogConfig::getMaxLogFileSize()
{
    return maxLogFileSize_;
}

void LogConfig::setMaxLogFileSize(int sizeMB)
{
    maxLogFileSize_ = sizeMB;
}

int LogConfig::getMaxLogFileCount()
{
    return maxLogFileCount_;
}

void LogConfig::setMaxLogFileCount(int count)
{
    maxLogFileCount_ = count;
}

LogLevel LogConfig::getModuleLogLevel(LogModule module)
{
    if (moduleLogLevels_.contains(module)) {
        return moduleLogLevels_[module];
    }
    return defaultLogLevel_;
}

void LogConfig::setModuleLogLevel(LogModule module, LogLevel level)
{
    moduleLogLevels_[module] = level;
}

LogLevel LogConfig::getLayerLogLevel(LogLayer layer)
{
    if (layerLogLevels_.contains(layer)) {
        return layerLogLevels_[layer];
    }
    return defaultLogLevel_;
}

void LogConfig::setLayerLogLevel(LogLayer layer, LogLevel level)
{
    layerLogLevels_[layer] = level;
}

bool LogConfig::loadFromFile(const QString &configFilePath)
{
    QSettings settings(configFilePath, QSettings::IniFormat);
    
    // 加载基本配置
    int levelValue = settings.value("Logging/DefaultLevel", static_cast<int>(LogLevel::INFO)).toInt();
    defaultLogLevel_ = static_cast<LogLevel>(levelValue);
    
    defaultLogFilePath_ = settings.value("Logging/LogFilePath", "logs/application.log").toString();
    consoleOutputEnabled_ = settings.value("Logging/ConsoleOutput", true).toBool();
    fileOutputEnabled_ = settings.value("Logging/FileOutput", true).toBool();
    maxLogFileSize_ = settings.value("Logging/MaxFileSize", 10).toInt();
    maxLogFileCount_ = settings.value("Logging/MaxFileCount", 5).toInt();
    
    // 加载模块级别配置
    moduleLogLevels_.clear();
    settings.beginGroup("ModuleLevels");
    QStringList moduleKeys = settings.childKeys();
    for (const QString &key : moduleKeys) {
        bool ok;
        int moduleValue = key.toInt(&ok);
        if (ok) {
            LogModule module = static_cast<LogModule>(moduleValue);
            int levelValue = settings.value(key, static_cast<int>(defaultLogLevel_)).toInt();
            moduleLogLevels_[module] = static_cast<LogLevel>(levelValue);
        }
    }
    settings.endGroup();
    
    // 加载层级级别配置
    layerLogLevels_.clear();
    settings.beginGroup("LayerLevels");
    QStringList layerKeys = settings.childKeys();
    for (const QString &key : layerKeys) {
        bool ok;
        int layerValue = key.toInt(&ok);
        if (ok) {
            LogLayer layer = static_cast<LogLayer>(layerValue);
            int levelValue = settings.value(key, static_cast<int>(defaultLogLevel_)).toInt();
            layerLogLevels_[layer] = static_cast<LogLevel>(levelValue);
        }
    }
    settings.endGroup();
    
    return true;
}

bool LogConfig::saveToFile(const QString &configFilePath)
{
    QSettings settings(configFilePath, QSettings::IniFormat);
    
    // 保存基本配置
    settings.setValue("Logging/DefaultLevel", static_cast<int>(defaultLogLevel_));
    settings.setValue("Logging/LogFilePath", defaultLogFilePath_);
    settings.setValue("Logging/ConsoleOutput", consoleOutputEnabled_);
    settings.setValue("Logging/FileOutput", fileOutputEnabled_);
    settings.setValue("Logging/MaxFileSize", maxLogFileSize_);
    settings.setValue("Logging/MaxFileCount", maxLogFileCount_);
    
    // 保存模块级别配置
    settings.beginGroup("ModuleLevels");
    for (auto it = moduleLogLevels_.begin(); it != moduleLogLevels_.end(); ++it) {
        settings.setValue(QString::number(static_cast<int>(it.key())), 
                         static_cast<int>(it.value()));
    }
    settings.endGroup();
    
    // 保存层级级别配置
    settings.beginGroup("LayerLevels");
    for (auto it = layerLogLevels_.begin(); it != layerLogLevels_.end(); ++it) {
        settings.setValue(QString::number(static_cast<int>(it.key())), 
                         static_cast<int>(it.value()));
    }
    settings.endGroup();
    
    settings.sync();
    return settings.status() == QSettings::NoError;
}

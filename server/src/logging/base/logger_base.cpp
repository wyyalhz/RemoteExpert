#include "logger_base.h"
#include <QDebug>

LoggerBase::LoggerBase(QObject *parent)
    : QObject(parent)
    , currentLevel_(LogLevel::INFO)
    , currentModule_(LogModule::SYSTEM)
    , currentLayer_(LogLayer::BUSINESS)
{
}

LoggerBase::~LoggerBase()
{
}

void LoggerBase::debug(const QString &message)
{
    if (currentLevel_ <= LogLevel::DEBUG) {
        QString formattedMessage = formatMessage(LogLevel::DEBUG, "", message);
        outputLog(LogLevel::DEBUG, formattedMessage);
    }
}

void LoggerBase::info(const QString &message)
{
    if (currentLevel_ <= LogLevel::INFO) {
        QString formattedMessage = formatMessage(LogLevel::INFO, "", message);
        outputLog(LogLevel::INFO, formattedMessage);
    }
}

void LoggerBase::warning(const QString &message)
{
    if (currentLevel_ <= LogLevel::WARNING) {
        QString formattedMessage = formatMessage(LogLevel::WARNING, "", message);
        outputLog(LogLevel::WARNING, formattedMessage);
    }
}

void LoggerBase::error(const QString &message)
{
    if (currentLevel_ <= LogLevel::ERROR) {
        QString formattedMessage = formatMessage(LogLevel::ERROR, "", message);
        outputLog(LogLevel::ERROR, formattedMessage);
    }
}

void LoggerBase::critical(const QString &message)
{
    if (currentLevel_ <= LogLevel::CRITICAL) {
        QString formattedMessage = formatMessage(LogLevel::CRITICAL, "", message);
        outputLog(LogLevel::CRITICAL, formattedMessage);
    }
}

void LoggerBase::debug(const QString &context, const QString &message)
{
    if (currentLevel_ <= LogLevel::DEBUG) {
        QString formattedMessage = formatMessage(LogLevel::DEBUG, context, message);
        outputLog(LogLevel::DEBUG, formattedMessage);
    }
}

void LoggerBase::info(const QString &context, const QString &message)
{
    if (currentLevel_ <= LogLevel::INFO) {
        QString formattedMessage = formatMessage(LogLevel::INFO, context, message);
        outputLog(LogLevel::INFO, formattedMessage);
    }
}

void LoggerBase::warning(const QString &context, const QString &message)
{
    if (currentLevel_ <= LogLevel::WARNING) {
        QString formattedMessage = formatMessage(LogLevel::WARNING, context, message);
        outputLog(LogLevel::WARNING, formattedMessage);
    }
}

void LoggerBase::error(const QString &context, const QString &message)
{
    if (currentLevel_ <= LogLevel::ERROR) {
        QString formattedMessage = formatMessage(LogLevel::ERROR, context, message);
        outputLog(LogLevel::ERROR, formattedMessage);
    }
}

void LoggerBase::critical(const QString &context, const QString &message)
{
    if (currentLevel_ <= LogLevel::CRITICAL) {
        QString formattedMessage = formatMessage(LogLevel::CRITICAL, context, message);
        outputLog(LogLevel::CRITICAL, formattedMessage);
    }
}

QString LoggerBase::getModuleName() const
{
    switch (currentModule_) {
        case LogModule::USER: return "USER";
        case LogModule::WORKORDER: return "WORKORDER";
        case LogModule::DATABASE: return "DB";
        case LogModule::NETWORK: return "NET";
        case LogModule::SYSTEM: return "SYS";
        default: return "UNKNOWN";
    }
}

QString LoggerBase::getLayerName() const
{
    switch (currentLayer_) {
        case LogLayer::DATA: return "DATA";
        case LogLayer::BUSINESS: return "BUSINESS";
        case LogLayer::NETWORK: return "NETWORK";
        case LogLayer::API: return "API";
        default: return "UNKNOWN";
    }
}

QString LoggerBase::getLevelName(LogLevel level) const
{
    switch (level) {
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO: return "INFO";
        case LogLevel::WARNING: return "WARN";
        case LogLevel::ERROR: return "ERROR";
        case LogLevel::CRITICAL: return "CRITICAL";
        default: return "UNKNOWN";
    }
}

QString LoggerBase::formatMessage(LogLevel level, const QString &context, const QString &message) const
{
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
    QString moduleName = getModuleName();
    QString layerName = getLayerName();
    QString levelName = getLevelName(level);
    
    QString formattedMessage = QString("[%1] [%2][%3] %4")
        .arg(timestamp)
        .arg(moduleName)
        .arg(layerName);
    
    if (!context.isEmpty()) {
        formattedMessage += QString(" %1: %2").arg(context).arg(message);
    } else {
        formattedMessage += QString(" %1").arg(message);
    }
    
    return formattedMessage;
}

#ifndef DATAMODEL_H
#define DATAMODEL_H

#include <QObject>
#include <QVector>
#include <QDateTime>
#include <QtCharts/QLineSeries>

QT_CHARTS_USE_NAMESPACE

struct SensorData {
    QString name;
    double value;
    QString unit;
    QDateTime timestamp;
};

struct LogEntry {
    QDateTime timestamp;
    QString level;
    QString message;
};

struct FaultInfo {
    QString code;
    QString description;
    QString severity;
    QDateTime timestamp;
};

class DataModel : public QObject
{
    Q_OBJECT
public:
    explicit DataModel(QObject *parent = nullptr);

    // 传感器数据操作
    void addPressureData(const SensorData &data);
    void addTemperatureData(const SensorData &data);
    const QVector<SensorData>& pressureData() const;
    const QVector<SensorData>& temperatureData() const;
    void clearSensorData();

    // 日志操作
    void addLogEntry(const LogEntry &entry);
    const QVector<LogEntry>& logEntries() const;
    void clearLogEntries();

    // 故障信息操作
    void addFaultInfo(const FaultInfo &fault);
    const QVector<FaultInfo>& faultInfos() const;
    void clearFaultInfos();

    // 获取图表数据系列
    QLineSeries* pressureSeries();
    QLineSeries* temperatureSeries();

    void clearAllData();

signals:
    void pressureDataAdded();
    void temperatureDataAdded();
    void logEntryAdded();
    void faultInfoAdded();

private:
    QVector<SensorData> m_pressureData;
    QVector<SensorData> m_temperatureData;
    QVector<LogEntry> m_logEntries;
    QVector<FaultInfo> m_faultInfos;
    QLineSeries *m_pressureSeries;
    QLineSeries *m_temperatureSeries;
};

#endif // DATAMODEL_H

#include "data_model.h"
#include <QtCharts/QLineSeries>

DataModel::DataModel(QObject *parent) : QObject(parent),
    m_pressureSeries(new QLineSeries(this)),
    m_temperatureSeries(new QLineSeries(this))
{
    // 设置系列名称
    m_pressureSeries->setName("Pressure");
    m_temperatureSeries->setName("Temperature");

    // 设置系列颜色
    m_pressureSeries->setColor(Qt::blue);
    m_temperatureSeries->setColor(Qt::red);
}

// ================= 传感器数据操作 =================
void DataModel::addPressureData(const SensorData &data)
{
    m_pressureData.append(data);
    m_pressureSeries->append(data.timestamp.toMSecsSinceEpoch(), data.value);

    // 限制数据点数量(可选，防止内存占用过大)
    if (m_pressureSeries->count() > 1000) {
        m_pressureSeries->removePoints(0, m_pressureSeries->count() - 800);
    }

    emit pressureDataAdded();
}

void DataModel::addTemperatureData(const SensorData &data)
{
    m_temperatureData.append(data);
    m_temperatureSeries->append(data.timestamp.toMSecsSinceEpoch(), data.value);

    // 限制数据点数量(可选)
    if (m_temperatureSeries->count() > 1000) {
        m_temperatureSeries->removePoints(0, m_temperatureSeries->count() - 800);
    }

    emit temperatureDataAdded();
}

const QVector<SensorData>& DataModel::pressureData() const
{
    return m_pressureData;
}

const QVector<SensorData>& DataModel::temperatureData() const
{
    return m_temperatureData;
}

void DataModel::clearSensorData()
{
    m_pressureData.clear();
    m_temperatureData.clear();
    m_pressureSeries->clear();
    m_temperatureSeries->clear();
}

// ================= 日志操作 =================
void DataModel::addLogEntry(const LogEntry &entry)
{
    m_logEntries.append(entry);

    // 限制日志数量(可选)
    if (m_logEntries.size() > 500) {
        m_logEntries.removeFirst();
    }

    emit logEntryAdded();
}

const QVector<LogEntry>& DataModel::logEntries() const
{
    return m_logEntries;
}

void DataModel::clearLogEntries()
{
    m_logEntries.clear();
}

// ================= 故障信息操作 =================
void DataModel::addFaultInfo(const FaultInfo &fault)
{
    m_faultInfos.append(fault);

    // 限制故障信息数量(可选)
    if (m_faultInfos.size() > 100) {
        m_faultInfos.removeFirst();
    }

    emit faultInfoAdded();
}

const QVector<FaultInfo>& DataModel::faultInfos() const
{
    return m_faultInfos;
}

void DataModel::clearFaultInfos()
{
    m_faultInfos.clear();
}

// ================= 图表系列获取 =================
QLineSeries* DataModel::pressureSeries()
{
    return m_pressureSeries;
}

QLineSeries* DataModel::temperatureSeries()
{
    return m_temperatureSeries;
}

// ================= 清空所有数据 =================
void DataModel::clearAllData()
{
    clearSensorData();
    clearLogEntries();
    clearFaultInfos();
}

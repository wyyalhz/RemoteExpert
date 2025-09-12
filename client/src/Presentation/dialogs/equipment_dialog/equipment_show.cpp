#include "equipment_show.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QStandardItemModel>
#include <QtCharts/QDateTimeAxis>
#include <QtCharts/QValueAxis>
#include <QtCharts/QLineSeries>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMessageBox>
#include <QDebug>

EquipmentShow::EquipmentShow(QWidget *parent) : QWidget(parent)
{
    m_dataModel = new DataModel(this);
    m_serialPort = new QSerialPort(this);
    setupUI();
    setupConnections();
}

EquipmentShow::~EquipmentShow()
{
    closeSerialPort();
}

bool EquipmentShow::openSerialPort(const QString &portName, qint32 baudRate)
{
    closeSerialPort();

    m_serialPort->setPortName(portName);
    m_serialPort->setBaudRate(baudRate);
    m_serialPort->setDataBits(QSerialPort::Data8);
    m_serialPort->setParity(QSerialPort::NoParity);
    m_serialPort->setStopBits(QSerialPort::OneStop);
    m_serialPort->setFlowControl(QSerialPort::NoFlowControl);

    if (!m_serialPort->open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, "Error",
                            QString("Failed to open port %1: %2")
                            .arg(portName).arg(m_serialPort->errorString()));
        return false;
    }

    qDebug() << "Opened port:" << portName << "at" << baudRate << "baud";
    return true;
}

void EquipmentShow::closeSerialPort()
{
    if (m_serialPort->isOpen()) {
        m_serialPort->close();
    }
}

bool EquipmentShow::isSerialConnected() const
{
    return m_serialPort->isOpen();
}

void EquipmentShow::onSerialDataReceived() {
    static QByteArray buffer;

    QByteArray newData = m_serialPort->readAll();
    qDebug() << "Raw data (hex):" << newData.toHex();
    qDebug() << "Raw data (string):" << QString::fromUtf8(newData);

    // 处理转义字符（将\\n转换为真正的换行符）
    newData.replace("\\n", "\n");
    buffer += newData;

    // 查找真正的换行符
    int endPos = buffer.indexOf('\n');
    if (endPos != -1) {
        QByteArray completeData = buffer.left(endPos).trimmed();
        buffer.remove(0, endPos + 1);

        // 打印处理后的数据
        qDebug() << "Unescaped JSON:" << completeData;

        // 移除可能存在的转义字符
        completeData.replace("\\\"", "\"");

        parseSerialData(completeData);
    }
}

void EquipmentShow::parseSerialData(const QByteArray &data)
{
    qDebug() << "Attempting to parse:" << data;

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        qDebug() << "JSON parse error:" << parseError.errorString();
        return;
    }

    if (!doc.isObject()) {
        qDebug() << "Expected JSON object";
        return;
    }

    QJsonObject json = doc.object();
    QDateTime timestamp = QDateTime::currentDateTime();

    // 解析传感器数据
    if (json.contains("sensors") && json["sensors"].isArray()) {
        QJsonArray sensors = json["sensors"].toArray();
        for (const QJsonValue &value : sensors) {
            if (value.isObject()) {
                QJsonObject sensor = value.toObject();

                if (sensor.contains("name") && sensor.contains("value")) {
                    SensorData sd;
                    sd.name = sensor["name"].toString();
                    sd.value = sensor["value"].toDouble();
                    sd.unit = sensor.contains("unit") ? sensor["unit"].toString() : "";
                    sd.timestamp = timestamp;

                    if (sd.name == "Pressure") {
                        m_dataModel->addPressureData(sd);
                    } else if (sd.name == "Temperature") {
                        m_dataModel->addTemperatureData(sd);
                    }
                }
            }
        }
    }

    // 解析日志数据
    if (json.contains("logs") && json["logs"].isArray()) {
        QJsonArray logs = json["logs"].toArray();
        for (const QJsonValue &value : logs) {
            if (value.isObject()) {
                QJsonObject log = value.toObject();

                if (log.contains("level") && log.contains("message")) {
                    LogEntry entry;
                    entry.timestamp = timestamp;
                    entry.level = log["level"].toString();
                    entry.message = log["message"].toString();
                    m_dataModel->addLogEntry(entry);
                }
            }
        }
    }

    // 解析故障数据
    if (json.contains("faults") && json["faults"].isArray()) {
        QJsonArray faults = json["faults"].toArray();
        for (const QJsonValue &value : faults) {
            if (value.isObject()) {
                QJsonObject fault = value.toObject();

                if (fault.contains("code") && fault.contains("description") && fault.contains("severity")) {
                    FaultInfo fi;
                    fi.code = fault["code"].toString();
                    fi.description = fault["description"].toString();
                    fi.severity = fault["severity"].toString();
                    fi.timestamp = timestamp;
                    m_dataModel->addFaultInfo(fi);
                }
            }
        }
    }
}

void EquipmentShow::setupUI()
{
    // 主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(5, 5, 5, 5);

    // 主分割器
    m_mainSplitter = new QSplitter(Qt::Vertical, this);

    // 图表分割器
    QSplitter *chartSplitter = new QSplitter(Qt::Horizontal, this);

    // 压力图表
    m_pressureChartView = new QChartView(this);
    setupPressureChart();

    // 温度图表
    m_temperatureChartView = new QChartView(this);
    setupTemperatureChart();

    chartSplitter->addWidget(m_pressureChartView);
    chartSplitter->addWidget(m_temperatureChartView);
    chartSplitter->setStretchFactor(0, 1);
    chartSplitter->setStretchFactor(1, 1);

    // 数据表格
    m_dataTabs = new QTabWidget(this);
    setupTables();

    m_mainSplitter->addWidget(chartSplitter);
    m_mainSplitter->addWidget(m_dataTabs);
    m_mainSplitter->setStretchFactor(0, 2);
    m_mainSplitter->setStretchFactor(1, 1);

    mainLayout->addWidget(m_mainSplitter);
    setLayout(mainLayout);

    // 初始大小
    resize(1000, 700);
}

void EquipmentShow::setupPressureChart()
{
    QChart *chart = new QChart();
    chart->setTitle("Pressure (MPa)");

    // 设置坐标轴
    QDateTimeAxis *axisX = new QDateTimeAxis();
    axisX->setTitleText("Time");
    axisX->setFormat("hh:mm:ss");
    axisX->setRange(QDateTime::currentDateTime().addSecs(-60), QDateTime::currentDateTime());
    chart->addAxis(axisX, Qt::AlignBottom);

    QValueAxis *axisY = new QValueAxis();
    axisY->setTitleText("Value");
    axisY->setRange(0, 20); // 假设压力范围0-20MPa
    chart->addAxis(axisY, Qt::AlignLeft);

    m_pressureChartView->setChart(chart);
    m_pressureChartView->setRenderHint(QPainter::Antialiasing);
}

void EquipmentShow::setupTemperatureChart()
{
    QChart *chart = new QChart();
    chart->setTitle("Temperature (°C)");

    // 设置坐标轴
    QDateTimeAxis *axisX = new QDateTimeAxis();
    axisX->setTitleText("Time");
    axisX->setFormat("hh:mm:ss");
    axisX->setRange(QDateTime::currentDateTime().addSecs(-60), QDateTime::currentDateTime());
    chart->addAxis(axisX, Qt::AlignBottom);

    QValueAxis *axisY = new QValueAxis();
    axisY->setTitleText("Value");
    axisY->setRange(0, 100); // 假设温度范围0-100°C
    chart->addAxis(axisY, Qt::AlignLeft);

    m_temperatureChartView->setChart(chart);
    m_temperatureChartView->setRenderHint(QPainter::Antialiasing);
}

void EquipmentShow::setupTables()
{
    // 日志表格
    m_logTable = new QTableView(this);
    QStandardItemModel *logModel = new QStandardItemModel(0, 3, this);
    logModel->setHorizontalHeaderLabels({"Time", "Level", "Message"});
    m_logTable->setModel(logModel);
    m_logTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // 故障表格
    m_faultTable = new QTableView(this);
    QStandardItemModel *faultModel = new QStandardItemModel(0, 3, this);
    faultModel->setHorizontalHeaderLabels({"Code", "Description", "Severity"});
    m_faultTable->setModel(faultModel);
    m_faultTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    m_dataTabs->addTab(m_logTable, "Logs");
    m_dataTabs->addTab(m_faultTable, "Faults");
}

void EquipmentShow::setupConnections() {
    connect(m_dataModel, &DataModel::pressureDataAdded, this, &EquipmentShow::updatePressureChart);
    connect(m_dataModel, &DataModel::temperatureDataAdded, this, &EquipmentShow::updateTemperatureChart);
    connect(m_dataModel, &DataModel::logEntryAdded, this, &EquipmentShow::updateLogTable);
    connect(m_dataModel, &DataModel::faultInfoAdded, this, &EquipmentShow::updateFaultTable);
    connect(m_serialPort, &QSerialPort::readyRead, this, &EquipmentShow::onSerialDataReceived);
    connect(m_serialPort, &QSerialPort::errorOccurred, [this](QSerialPort::SerialPortError error) {
        if (error != QSerialPort::NoError) {
            qDebug() << "Serial error:" << m_serialPort->errorString();
        }
    });
}

// 数据接口实现
void EquipmentShow::addPressureData(double value, const QDateTime &timestamp)
{
    SensorData data;
    data.name = "Pressure";
    data.value = value;
    data.unit = "MPa";
    data.timestamp = timestamp;
    m_dataModel->addPressureData(data);
}

void EquipmentShow::addTemperatureData(double value, const QDateTime &timestamp)
{
    SensorData data;
    data.name = "Temperature";
    data.value = value;
    data.unit = "°C";
    data.timestamp = timestamp;
    m_dataModel->addTemperatureData(data);
}

void EquipmentShow::addLogEntry(const QString &level, const QString &message, const QDateTime &timestamp)
{
    LogEntry entry;
    entry.timestamp = timestamp;
    entry.level = level;
    entry.message = message;
    m_dataModel->addLogEntry(entry);
}

void EquipmentShow::addFaultInfo(const QString &code, const QString &description,
                                const QString &severity, const QDateTime &timestamp)
{
    FaultInfo fault;
    fault.code = code;
    fault.description = description;
    fault.severity = severity;
    fault.timestamp = timestamp;
    m_dataModel->addFaultInfo(fault);
}

void EquipmentShow::clearAllData()
{
    m_dataModel->clearAllData();
    updatePressureChart();
    updateTemperatureChart();
    updateLogTable();
    updateFaultTable();
}

void EquipmentShow::updatePressureChart()
{
    QChart *chart = m_pressureChartView->chart();
    QLineSeries *series = m_dataModel->pressureSeries();

    if (chart->series().isEmpty()) {
        chart->addSeries(series);
        series->attachAxis(chart->axisX());
        series->attachAxis(chart->axisY());
    }

    // 调整X轴范围显示最新60秒数据
    QDateTimeAxis *axisX = qobject_cast<QDateTimeAxis*>(chart->axisX());
    if (axisX) {
        QDateTime now = QDateTime::currentDateTime();
        axisX->setRange(now.addSecs(-60), now);
    }
}

void EquipmentShow::updateTemperatureChart()
{
    QChart *chart = m_temperatureChartView->chart();
    QLineSeries *series = m_dataModel->temperatureSeries();

    if (chart->series().isEmpty()) {
        chart->addSeries(series);
        series->attachAxis(chart->axisX());
        series->attachAxis(chart->axisY());
    }

    // 调整X轴范围显示最新60秒数据
    QDateTimeAxis *axisX = qobject_cast<QDateTimeAxis*>(chart->axisX());
    if (axisX) {
        QDateTime now = QDateTime::currentDateTime();
        axisX->setRange(now.addSecs(-60), now);
    }
}

void EquipmentShow::updateLogTable()
{
    QStandardItemModel *model = qobject_cast<QStandardItemModel*>(m_logTable->model());
    if (!model) return;

    const auto &logs = m_dataModel->logEntries();
    if (logs.isEmpty()) return;

    const LogEntry &entry = logs.last();
    QList<QStandardItem*> row;
    row << new QStandardItem(entry.timestamp.toString("hh:mm:ss"));
    row << new QStandardItem(entry.level);
    row << new QStandardItem(entry.message);

    // 设置不同级别日志的颜色
    if (entry.level == "ERROR") {
        foreach (QStandardItem *item, row) {
            item->setForeground(Qt::red);
            item->setFont(QFont("Arial", 9, QFont::Bold));
        }
    } else if (entry.level == "WARNING") {
        foreach (QStandardItem *item, row) {
            item->setForeground(Qt::darkYellow);
        }
    }

    model->appendRow(row);
    m_logTable->scrollToBottom();
}

void EquipmentShow::updateFaultTable()
{
    QStandardItemModel *model = qobject_cast<QStandardItemModel*>(m_faultTable->model());
    if (!model) return;

    const auto &faults = m_dataModel->faultInfos();
    if (faults.isEmpty()) return;

    const FaultInfo &fault = faults.last();
    QList<QStandardItem*> row;
    row << new QStandardItem(fault.code);
    row << new QStandardItem(fault.description);
    row << new QStandardItem(fault.severity);

    // 设置不同严重程度故障的颜色
    if (fault.severity == "CRITICAL") {
        foreach (QStandardItem *item, row) {
            item->setForeground(Qt::red);
            item->setFont(QFont("Arial", 9, QFont::Bold));
        }
    } else if (fault.severity == "HIGH") {
        foreach (QStandardItem *item, row) {
            item->setForeground(Qt::darkRed);
        }
    } else if (fault.severity == "MEDIUM") {
        foreach (QStandardItem *item, row) {
            item->setForeground(Qt::darkYellow);
        }
    }

    model->appendRow(row);
    m_faultTable->scrollToBottom();
}

void EquipmentShow::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    // 可以在这里添加响应窗口大小变化的逻辑
}

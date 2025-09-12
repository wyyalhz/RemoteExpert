#ifndef EQUIPMENTSHOW_H
#define EQUIPMENTSHOW_H

#include <QWidget>
#include <QSplitter>
#include <QTabWidget>
#include <QtCharts/QChartView>
#include <QSerialPort>
#include <QJsonDocument>
#include <QTableView>
#include "data_model.h"

QT_CHARTS_USE_NAMESPACE

class EquipmentShow : public QWidget
{
    Q_OBJECT
public:
    explicit EquipmentShow(QWidget *parent = nullptr);
    ~EquipmentShow();

    // 串口操作
    bool openSerialPort(const QString &portName, qint32 baudRate);
    void closeSerialPort();
    bool isSerialConnected() const;
    void parseSerialData(const QByteArray &data);

private slots:
    void onSerialDataReceived();
    void updatePressureChart();
    void updateTemperatureChart();
    void updateLogTable();
    void updateFaultTable();

private:
    DataModel *m_dataModel;
    QSerialPort *m_serialPort;

    // UI组件
    QSplitter *m_mainSplitter;
    QChartView *m_pressureChartView;
    QChartView *m_temperatureChartView;
    QTabWidget *m_dataTabs;
    QTableView *m_logTable;
    QTableView *m_faultTable;

    void setupUI();
    void setupConnections();
    void setupCharts();
    void setupTables();

    void setupPressureChart();
    void setupTemperatureChart();
    void addPressureData(double value, const QDateTime &timestamp);
    void addTemperatureData(double value, const QDateTime &timestamp);
    void addLogEntry(const QString &level, const QString &message, const QDateTime &timestamp);
    void addFaultInfo(const QString &code, const QString &description, const QString &severity, const QDateTime &timestamp);
    void clearAllData();
    void resizeEvent(QResizeEvent *event);
};

#endif // EQUIPMENTSHOW_H

/**********************************************************
Author: Qt君
微信公众号: Qt君(首发)
QQ群: 732271126
LICENSE: MIT
**********************************************************/
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDebug>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QTimer>
#include <QFile>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void serialUpdateData(double value);

private slots:
    void on_clearButton_clicked();
    void on_sendButton_clicked();
    void on_openButton_clicked();

    void onReadData();
    void onUpdateChart();
    void onCreateTestData();
    //void onTestClicked(bool status);
    void onUpdateChart(QStringList list);

    //void on_m_testBtn_clicked();

    void on_edit_pushButton_clicked();

    double getDataRate(int index,double data);//获取数据比率函数

    void on_saveData_pushButton_clicked();

private:
    Ui::MainWindow *ui;
    QSerialPort m_serialPort;

    /* 定义两个可变数组存放绘图的坐标数据 */
    QVector<double> m_xs;
    QVector<double> m_ys;

    QVector <QVector<double>> m_xs_list;
    QVector <QVector<double>> m_ys_list;

    int m_xLength;
    double m_serialUpdateData;

    QTimer m_testTimer;
    QTimer m_updateChartTimer;
    int chAmount = 0;  //通道个数，第一次收到的数据个数就是通道个数
    int postionIndex = 0; //接收到数据次数，串口没接收到一次数据加一
    QMap<QString,QString> channelNameList;
    QVector<QColor> colorList = { QColor("#90EE90"),QColor("#FF6100"),QColor("#802A2A"),QColor("#FF0000"),QColor("#B0171F"),QColor("#00FFFF"),QColor("#0000FF"),QColor("#A020F0")} ;
};

#endif // MAINWINDOW_H

/**********************************************************
Author: Qt君
微信公众号: Qt君(首发)
QQ群: 732271126
LICENSE: MIT
**********************************************************/
#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_xLength(0)
{
    ui->setupUi(this);

    /* 向绘图区域QCustomPlot添加一条曲线 */
//    ui->qCustomPlot->addGraph();

    ui->qCustomPlot->setInteractions(QCP::iRangeDrag|QCP::iRangeZoom|QCP::iSelectPlottables);//可以缩放可以滚动点击选择


     ui->qCustomPlot->axisRect()->insetLayout()->setInsetAlignment(0,Qt::AlignTop|Qt::AlignRight);


    m_updateChartTimer.start(1000);

//    connect(&m_updateChartTimer, SIGNAL(timeout()), this, SLOT(onUpdateChart()));
    connect(&m_testTimer, SIGNAL(timeout()), this, SLOT(onCreateTestData()));
//    connect(ui->m_testBtn, SIGNAL(clicked(bool)), this, SLOT(onTestClicked(bool)));


    /* 连接数据信号槽 */
    connect(&m_serialPort, SIGNAL(readyRead()), this, SLOT(onReadData()));

    connect(ui->edit_pushButton, SIGNAL(click),this,SLOT(editChannelName()));


    /* 设置坐标轴标签名称 */
    ui->qCustomPlot->xAxis->setLabel("次数");
//    ui->qCustomPlot->yAxis->setLabel("温度");

    /* 设置坐标轴显示范围 */
    ui->qCustomPlot->xAxis->setRange(0, 100);
    ui->qCustomPlot->yAxis->setRange(0, 100);

    /* 查找可用的串口 */
    foreach(const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
        ui->PortBox->addItem(info.portName());
    }

    /* 设置波特率下拉菜单默认显示第三项(9600) */
    ui->BaudBox->setCurrentIndex(3);

    /* 关闭发送按钮的使能 */
    ui->sendButton->setEnabled(false);

    qDebug()<<"界面初始化成功！";
}

MainWindow::~MainWindow()
{
    delete ui;
}

/* 清空接收窗口 */
void MainWindow::on_clearButton_clicked()
{
    ui->textEdit->clear();
}

/* 发送数据 */
void MainWindow::on_sendButton_clicked()
{
    m_serialPort.write(ui->textEdit_2->toPlainText().toLatin1());
}

/* 读取接收到的数据 */
void MainWindow::onReadData()
{
    QByteArray buf;
    buf = m_serialPort.readAll();
    m_serialUpdateData = buf.toDouble();

    if(! buf.isEmpty()) {
        QString str = ui->textEdit->toPlainText();
        QString eStr = tr(buf);
        eStr.remove(QRegExp("[\\[\\]]"));
        QStringList strList = eStr.split(",");  //把纯文本数据处理成数组
        chAmount = strList.size();
        onUpdateChart(strList);

        str += tr(buf);
        ui->textEdit->clear();
        ui->textEdit->append(str);

    }

    buf.clear();
}

void MainWindow::onUpdateChart()
{
    if (m_xLength > 10) {
        m_xLength = 0;
        m_xs.clear();
        m_ys.clear();
    }

    m_xs.append(m_xLength);
    m_ys.append(m_serialUpdateData);
    m_serialUpdateData = 0;

    ui->qCustomPlot->replot();
//    qDebug() << m_xs << m_ys;
    ui->qCustomPlot->graph(0)->setData(m_xs, m_ys);

    m_xLength++;
}

void MainWindow::onUpdateChart(QStringList list)
{
    ui->qCustomPlot->legend->setVisible(true);


    for(int j=0;j<chAmount;j++){
        for (int i = 0; i < list.size(); i++) {
            double inData;
            QVector<double> ydata;
            QVector <double> xdata;
            //第一次收到数据之间添加到数组中
            if(postionIndex==0 && i==j){
                ui->qCustomPlot->addGraph();
                inData = getDataRate(i,list[i].toDouble());
                ydata.append(inData);
                m_ys_list.append(ydata);
                xdata.append(postionIndex);
                m_xs_list.append(xdata);
            }
            //            非第一次收到数据取出数据然后把数据添加进去
            if(i==j&& postionIndex!=0){
                //当数据大于4000条的时候就开始删除前面的数据
                 inData = getDataRate(i,list[i].toDouble());
                 if(m_ys_list[j].size()>=4000){
                     m_ys_list[j].pop_front();
                 }
                m_ys_list[j].append(inData);
                m_xs_list[j].append(postionIndex);
            }

        }
    }
//    ui->qCustomPlot->graph(0)->setName("ch");
    for(int k=0;k<list.size();k++)
    {
        QPen pen(colorList[k], 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        ui->qCustomPlot->graph(k)->setData(m_xs_list[k], m_ys_list[k]);
        ui->qCustomPlot->graph(k)->setName("ch0"+QString::number(k+1));
        ui->qCustomPlot->graph(k)->setPen(pen);

    }

    postionIndex++;
    //x轴数据动态移动
    double lower = 0;
    double upper = 100;
    if(postionIndex>100){
        lower = postionIndex-100;
        upper = postionIndex;
    }
    ui->qCustomPlot->xAxis->setRange(lower,upper);
    ui->qCustomPlot->replot();

}

void MainWindow::onCreateTestData()
{
    m_serialUpdateData = rand() % 100 + 10;
}

//void MainWindow::onTestClicked(bool status)
//{
//    if (m_testTimer.isActive()) {
//        m_testTimer.stop();
//        ui->m_testBtn->setText("开启");
//    }
//    else {
//        m_testTimer.start();
//        ui->m_testBtn->setText("关闭");
//        m_serialUpdateData = 0;
//    }
//}

void MainWindow::on_openButton_clicked()
{
    if(ui->openButton->text()==tr("打开串口")) {
        if (m_serialPort.isOpen()) {
            m_serialPort.close();
            m_serialPort.clear();
        }

        /* 设置串口名 */
        m_serialPort.setPortName(ui->PortBox->currentText());

        /* 打开串口 */
        m_serialPort.open(QIODevice::ReadWrite);

        /* 设置波特率 */
        m_serialPort.setBaudRate(ui->BaudBox->currentText().toInt());

        /* 设置数据位数 */
        if (ui->BitNumBox->currentIndex() == 8) {
            m_serialPort.setDataBits(QSerialPort::Data8);
        }

        /* 设置奇偶校验 */
        if (ui->ParityBox->currentIndex() == 0) {
            m_serialPort.setParity(QSerialPort::NoParity);
        }

        /* 设置停止位 */
        switch(ui->StopBox->currentIndex()) {
            case 1: m_serialPort.setStopBits(QSerialPort::OneStop); break;
            case 2: m_serialPort.setStopBits(QSerialPort::TwoStop); break;
            default: break;
        }

        /* 设置流控制 */
        m_serialPort.setFlowControl(QSerialPort::NoFlowControl);

        /* 关闭设置菜单使能 */
        ui->PortBox->setEnabled(false);
        ui->BaudBox->setEnabled(false);
        ui->BitNumBox->setEnabled(false);
        ui->ParityBox->setEnabled(false);
        ui->StopBox->setEnabled(false);
        ui->openButton->setText(tr("关闭串口"));
        ui->sendButton->setEnabled(true);
        /*打开串口后就不能设置比率 */
        ui->mu01_lineEdit->setDisabled(true);
        ui->mu02_lineEdit->setDisabled(true);
        ui->mu03_lineEdit->setDisabled(true);
        ui->mu04_lineEdit->setDisabled(true);
        ui->mu05_lineEdit->setDisabled(true);
        ui->mu06_lineEdit->setDisabled(true);
        ui->mu07_lineEdit->setDisabled(true);
        ui->mu08_lineEdit->setDisabled(true);

    }
    else {
        /* 关闭串口 */
        m_serialPort.close();
        m_serialPort.clear();

        /* 恢复设置使能 */
        ui->PortBox->setEnabled(true);
        ui->BaudBox->setEnabled(true);
        ui->BitNumBox->setEnabled(true);
        ui->ParityBox->setEnabled(true);
        ui->StopBox->setEnabled(true);
        ui->openButton->setText(tr("打开串口"));
        ui->sendButton->setEnabled(false);
        /* 关闭串口后可以修改 */
        ui->mu01_lineEdit->setDisabled(false);
        ui->mu02_lineEdit->setDisabled(false);
        ui->mu03_lineEdit->setDisabled(false);
        ui->mu04_lineEdit->setDisabled(false);
        ui->mu05_lineEdit->setDisabled(false);
        ui->mu06_lineEdit->setDisabled(false);
        ui->mu07_lineEdit->setDisabled(false);
        ui->mu08_lineEdit->setDisabled(false);
    }
}

void MainWindow::on_edit_pushButton_clicked()
{

    //修改通道名称
    QString ch01 = ui->ch01_lineEdit->text().toUtf8().data();
    QString ch02 = ui->ch02_lineEdit->text().toUtf8().data();
    QString ch03 = ui->ch03_lineEdit->text().toUtf8().data();
    QString ch04 = ui->ch04_lineEdit->text().toUtf8().data();
    QString ch05 = ui->ch05_lineEdit->text().toUtf8().data();
    QString ch06 = ui->ch06_lineEdit->text().toUtf8().data();
    QString ch07 = ui->ch07_lineEdit->text().toUtf8().data();
    QString ch08 = ui->ch08_lineEdit->text().toUtf8().data();
    if(ch01!=""){
        channelNameList.insert("ch01",ch01);
    }
    if(ch02!=""){
        channelNameList.insert("ch02",ch02);
    }
    if(ch03!=""){
        channelNameList.insert("ch03",ch03);
    }
    if(ch04!=""){
        channelNameList.insert("ch04",ch04);
    }
    if(ch05!=""){
        channelNameList.insert("ch05",ch05);
    }
    if(ch06!=""){
        channelNameList.insert("ch06",ch06);
    }
    if(ch07!=""){
        channelNameList.insert("ch07",ch07);
    }
    if(ch08!=""){
        channelNameList.insert("ch08",ch08);
    }

    for (int i = 0; i < chAmount; ++i) {
        if(i==0 && ch01!="")
        {
            qDebug() << ch01;
            ui->qCustomPlot->graph(i)->setName(ch01);
        }
        if(i==1 && ch02!="")
        {
            ui->qCustomPlot->graph(i)->setName(ch02);
        }

        if(i==2 && ch03!="")
        {
            ui->qCustomPlot->graph(i)->setName(ch03);
        }
        if(i==3 && ch04!="")
        {
            ui->qCustomPlot->graph(i)->setName(ch04);
        }
        if(i==4 && ch05!="")
        {
            ui->qCustomPlot->graph(i)->setName(ch05);
        }
        if(i==5 && ch06!="")
        {
            ui->qCustomPlot->graph(i)->setName(ch06);
        }
        if(i==7 && ch08!="")
        {
            ui->qCustomPlot->graph(i)->setName(ch07);
        }
    }


     ui->qCustomPlot->replot();
}

double MainWindow::getDataRate(int index,double data)
{
    double rate = 1;
    double outData=0;

    //倍率
    QString mu01 = ui->mu01_lineEdit->text().toUtf8().data();
    QString mu02 = ui->mu02_lineEdit->text().toUtf8().data();
    QString mu03 = ui->mu03_lineEdit->text().toUtf8().data();
    QString mu04 = ui->mu04_lineEdit->text().toUtf8().data();
    QString mu05 = ui->mu05_lineEdit->text().toUtf8().data();
    QString mu06 = ui->mu06_lineEdit->text().toUtf8().data();
    QString mu07 = ui->mu07_lineEdit->text().toUtf8().data();
    QString mu08 = ui->mu08_lineEdit->text().toUtf8().data();

    if(index==0 && mu01!=1){
        rate = mu01.toDouble();
        outData = rate*data;
    }
    if(index==1 && mu02!=1){
        rate = mu02.toDouble();
        outData = rate*data;
    }
    if(index==2 && mu03!=1){
        rate = mu03.toDouble();
        outData = rate*data;
    }
    if(index==3 && mu04!=1){
        rate = mu04.toDouble();
        outData = rate*data;
    }
    if(index==4 && mu05!=1){
        rate = mu05.toDouble();
        outData = rate*data;
    }
    if(index==5 && mu06!=1){
        rate = mu06.toDouble();
        outData = rate*data;
    }
    if(index==6 && mu07!=1){
        rate = mu07.toDouble();
        outData = rate*data;
    }
    if(index==7 && mu08!=1){
        rate = mu08.toDouble();
        outData = rate*data;
    }

    return outData;
}


//保存数据
void MainWindow::on_saveData_pushButton_clicked()
{
    QString filename = QApplication::applicationDirPath()+QDateTime::currentDateTime().toString("yyyy-MM-dd")+".txt";
    QFile file(filename);
    file.open(QIODevice::WriteOnly);
    QString addInfo =ui->textEdit->toPlainText()+"\n";
    char *pAddinfo;
    QByteArray qt_byte=addInfo.toUtf8();
    pAddinfo=qt_byte.data();
    file.write(pAddinfo);
    bool exist = QFile::exists(filename);
    if(exist){
        ui->label_file_address->setText(filename);
    }
}

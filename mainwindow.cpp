#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //刷新串口
    on_refreshSerialButton_clicked();

    //增加波特率
    ui->bandRateComboBox->clear();
    ui->bandRateComboBox->addItem("9600",QSerialPort::Baud9600);
    ui->bandRateComboBox->addItem("38400",QSerialPort::Baud38400);
    ui->bandRateComboBox->addItem("57600",QSerialPort::Baud57600);
    ui->bandRateComboBox->addItem("115200",QSerialPort::Baud115200);
    ui->bandRateComboBox->setCurrentIndex(3);

    //增加时间间隔
    ui->intervalTimeComboBox->clear();
    ui->intervalTimeComboBox->addItem("200",200);
    ui->intervalTimeComboBox->addItem("500",500);
    ui->intervalTimeComboBox->addItem("1000",1000);
    ui->intervalTimeComboBox->setCurrentIndex(0);

    nIndexAutorun = 0;
    nIndexInit = 0;
    nIndexManual = 0;

    nSizeAutorun = 0;
    nSizeInit = 0;
    nSizeManual = 0;

    isSendInit = false;
    isSendAutorun = false;
    isSendManual = false;
    isShowTimeFlag = true;

    connect(&serialPort,SIGNAL(readyRead()),this,SLOT(on_serialPort_readyRead()));

    //增加系统标置位
    ui->statusBar->showMessage(GetCurrentTime() + " - " + QString::fromLocal8Bit("系统初始化完毕!"),1000);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_connectSerialButton_clicked()
{
    //设置串口
    serialPort.setPortName(ui->portNameComboBox->currentText());
    serialPort.setBaudRate(static_cast<QSerialPort::BaudRate>(ui->bandRateComboBox->currentData().toInt()));
    serialPort.setDataBits(QSerialPort::Data8);
    serialPort.setParity(QSerialPort::NoParity);
    serialPort.setStopBits(QSerialPort::OneStop);

    //尝试打开串口
    if(serialPort.open(QIODevice::ReadWrite))
    {
        ui->bandRateComboBox->setDisabled(true);
        ui->portNameComboBox->setDisabled(true);
        ui->disconnectSerialButton->setEnabled(true);
        ui->intervalTimeComboBox->setDisabled(true);
        ui->connectSerialButton->setDisabled(true);
        ui->refreshSerialButton->setDisabled(true);
        ui->statusBar->showMessage(GetCurrentTime() + " - " + QString::fromLocal8Bit("串口打开成功!"),1000);
    }
    else
    {
        ui->statusBar->showMessage(GetCurrentTime() + " - " + QString::fromLocal8Bit("串口打开失败!"),1000);
    }
}

void MainWindow::on_disconnectSerialButton_clicked()
{
    //关闭
    serialPort.close();

    //如果还打开,报错
    if(serialPort.isOpen())
    {
        ui->statusBar->showMessage(GetCurrentTime() + " - " + QString::fromLocal8Bit("串口关闭失败!"),1000);
        return;
    }

    ui->bandRateComboBox->setEnabled(true);
    ui->portNameComboBox->setEnabled(true);
    ui->connectSerialButton->setEnabled(true);
    ui->disconnectSerialButton->setDisabled(true);
    ui->intervalTimeComboBox->setEnabled(true);
    ui->refreshSerialButton->setEnabled(true);
    ui->statusBar->showMessage(GetCurrentTime() + " - " + QString::fromLocal8Bit("串口打开成功!"),1000);


}

void MainWindow::on_refreshSerialButton_clicked()
{
    //清除原有内容
    ui->portNameComboBox->clear();

    //添加现有串口
    const auto infos = QSerialPortInfo::availablePorts();
    //如果是空的就退出
    if(infos.isEmpty())
    {
        ui->portNameComboBox->addItem("None");
        return;
    }

    //非空就添加串口
    for(const QSerialPortInfo &info: infos)
    {
        ui->portNameComboBox->addItem(info.portName());
    }
}

void MainWindow::on_chooseInitPathButton_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(this,QString::fromLocal8Bit("选择文件"),currentPath.path(),tr("* txt"));
    if(filePath.isEmpty())
    {
        return;
    }

    sendInitData.clear();
    QFile file(filePath);
    QString line;
    if(file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        //没到底就一直读
        while(!file.atEnd())
        {
            line = QString(file.readLine());
            int nIndexJ = line.indexOf('J');
            if(nIndexJ != -1)
            {
                line.remove("\r");
                line.remove("\n");
                line.remove(0,nIndexJ);
                sendInitData.append(line);
            }

        }
        nSizeInit = sendInitData.size();
        ui->txLineEdit->setText(sendInitData.at(0));
        ui->initNumLabel->setText(QString::fromLocal8Bit("初始路径共%1个点").arg(nSizeInit));
    }

}

void MainWindow::on_choosManualPathButton_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(this,QString::fromLocal8Bit("选择文件"),currentPath.path(),tr("* txt"));
    if(filePath.isEmpty())
    {
        return;
    }

    sendManualData.clear();
    QFile file(filePath);
    QString line;
    if(file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        //没到底就一直读
        while(!file.atEnd())
        {
            line = QString(file.readLine());
            int nIndexJ = line.indexOf('J');
            if(nIndexJ != -1)
            {
                line.remove("\r");
                line.remove("\n");
                line.remove(0,nIndexJ);
                sendManualData.append(line);
            }

        }
        nSizeManual = sendManualData.size();
        ui->manualNumLabel->setText(QString::fromLocal8Bit("人工路径共%1个点").arg(nSizeManual));
    }
}

void MainWindow::on_chooseAutorunPathButton_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(this,QString::fromLocal8Bit("选择文件"),currentPath.path(),tr("* txt"));
    if(filePath.isEmpty())
    {
        return;
    }

    sendAutorunData.clear();
    QFile file(filePath);
    QString line;
    if(file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        //没到底就一直读
        while(!file.atEnd())
        {
            line = QString(file.readLine());
            int nIndexJ = line.indexOf('J');
            if(nIndexJ != -1)
            {
                line.remove("\r");
                line.remove("\n");
                line.remove(0,nIndexJ);
                sendAutorunData.append(line);
            }

        }
        nSizeAutorun = sendAutorunData.size();
        ui->autorunNumLabel->setText(QString::fromLocal8Bit("自动路径共%1个点").arg(nSizeAutorun));
    }
}

void MainWindow::on_intervalTimeComboBox_activated(int index)
{

}

void MainWindow::on_continuousTxInitButton_toggled(bool checked)
{
    //判断串口是否打开
    if(serialPort.isOpen() == false)
    {
        ui->statusBar->showMessage(GetCurrentTime() + " - " + QString::fromLocal8Bit("请先打开串口!"),1000);
        ui->continuousTxInitButton->setChecked(false);
        return;
    }

    //判断是否有足够的点
    if(nSizeInit == 0 )
    {
        ui->statusBar->showMessage(GetCurrentTime() + " - " + QString::fromLocal8Bit("没有足够的点!"),1000);
        ui->continuousTxInitButton->setChecked(false);
        return ;
    }

    //按下
    if(checked == true)
    {
        nTimerIDInit = startTimer(ui->intervalTimeComboBox->currentData().toInt());
        isSendInit= true;
    }
    //松开
    else
    {
        killTimer(nTimerIDInit);
        isSendInit = false;
    }
}

void MainWindow::on_continuousTxManualButton_toggled(bool checked)
{
    //判断串口是否打开
    if(serialPort.isOpen() == false)
    {
        ui->statusBar->showMessage(GetCurrentTime() + " - " + QString::fromLocal8Bit("请先打开串口!"),1000);
        ui->continuousTxManualButton->setChecked(false);
        return;
    }

    //判断是否有足够的点
    if(nSizeManual == 0 )
    {
        ui->statusBar->showMessage(GetCurrentTime() + " - " + QString::fromLocal8Bit("没有足够的点!"),1000);
        ui->continuousTxManualButton->setChecked(false);
        return ;
    }

    //按下
    if(checked == true)
    {
        nTimerIDManual = startTimer(ui->intervalTimeComboBox->currentData().toInt());
        isSendManual= true;
    }
    //松开
    else
    {
        killTimer(nTimerIDManual);
        isSendManual = false;
    }
}

void MainWindow::on_continuousTxAutorunButton_toggled(bool checked)
{
    //判断串口是否打开
    if(serialPort.isOpen() == false)
    {
        ui->statusBar->showMessage(GetCurrentTime() + " - " + QString::fromLocal8Bit("请先打开串口!"),1000);
        ui->continuousTxAutorunButton->setChecked(false);
        return;
    }

    //判断是否有足够的点
    if(nSizeAutorun == 0 )
    {
        ui->statusBar->showMessage(GetCurrentTime() + " - " + QString::fromLocal8Bit("没有足够的点!"),1000);
        ui->continuousTxAutorunButton->setChecked(false);
        return ;
    }

    //按下
    if(checked == true)
    {
        nTimerIDAutorun = startTimer(ui->intervalTimeComboBox->currentData().toInt());
        isSendAutorun= true;
    }
    //松开
    else
    {
        killTimer(nTimerIDAutorun);
        isSendAutorun = false;
    }

}

void MainWindow::on_clearRxTxButton_clicked()
{
    //ui->txLineEdit->clear();

    ui->rxTextEdit->clear();
    ui->rxTextEdit->document()->clear();
}

void MainWindow::on_singleTxButton_clicked()
{
    //判断串口是否打开
    if(serialPort.isOpen() == false)
    {
        ui->statusBar->showMessage(GetCurrentTime() + " - " + QString::fromLocal8Bit("请先打开串口"),1000);
        return;
    }

    serialPort.write(ui->txLineEdit->text().toLatin1().append("\r\n"));
}

void MainWindow::on_serialPort_readyRead()
{
    QTextCursor textCursor = ui->rxTextEdit->textCursor();
    textCursor.movePosition(QTextCursor::End);
    ui->rxTextEdit->setTextCursor(textCursor);

    QString readData = QString(serialPort.readAll());
    isShowTimeFlag ? readData = readData.replace("\r\n",GetCurrentTime() + " - \r\n") : readData;

    ui->rxTextEdit->insertPlainText(QString(serialPort.readAll()));
}

void MainWindow::timerEvent(QTimerEvent *event)
{
    //判断是不是初始
    if(event->timerId() == nTimerIDInit && isSendInit == true)
    {
        //有就发送
        if(nIndexInit < nSizeInit)
        {
            //发送数据
            ui->statusBar->showMessage(GetCurrentTime() + " - " + QString::fromLocal8Bit("当前发送第%1个").arg(nIndexInit),1000);
            serialPort.write(sendInitData.at(nIndexInit).toLatin1().append("\r\n"));

            //如果是最后一个
            if(nIndexInit == nSizeInit-1)
            {
                //复位清零
                nIndexInit = 0;
                ui->txLineEdit->clear();

                emit ui->continuousTxInitButton->toggled(false);
                ui->continuousTxInitButton->setChecked(false);
            }
            //不是最后一个就把下一个坐标放在文本上
            else
            {
                nIndexInit ++;
                ui->txLineEdit->setText(sendInitData.at(nIndexInit));
            }
        }
        else
        {
            ui->statusBar->showMessage(GetCurrentTime() + " - " + QString::fromLocal8Bit("发送完毕"),1000);
        }
    }


    //判断是不是自动
    if(event->timerId() == nTimerIDAutorun && isSendAutorun == true)
    {
        //有就发送
        if(nIndexAutorun < nSizeAutorun)
        {
            //发送数据
            ui->statusBar->showMessage(GetCurrentTime() + " - " + QString::fromLocal8Bit("当前发送第%1个").arg(nIndexAutorun),1000);

            serialPort.write(sendAutorunData.at(nIndexAutorun).toLatin1().append("\r\n"));

            //如果是最后一个
            if(nIndexAutorun == nSizeAutorun-1)
            {
                //复位清零
                nIndexAutorun = 0;
                ui->txLineEdit->clear();

                emit ui->continuousTxAutorunButton->toggled(false);
                ui->continuousTxAutorunButton->setChecked(false);
            }
            //不是最后一个就把下一个坐标放在文本上
            else
            {
                nIndexAutorun ++;
                ui->txLineEdit->setText(sendAutorunData.at(nIndexAutorun));
            }
        }
    }

    //判断是不是人工
    if(event->timerId() == nTimerIDManual && isSendManual == true)
    {
        //有就发送
        if(nIndexManual < nSizeManual)
        {
            //发送数据
            ui->statusBar->showMessage(GetCurrentTime() + " - " + QString::fromLocal8Bit("当前发送第%1个").arg(nIndexManual),1000);

            serialPort.write(sendManualData.at(nIndexManual).toLatin1().append("\r\n"));

            //如果是最后一个
            if(nIndexManual == nSizeManual-1)
            {
                //复位清零
                nIndexManual = 0;
                ui->txLineEdit->clear();

                emit ui->continuousTxManualButton->toggled(false);
                ui->continuousTxManualButton->setChecked(false);
            }
            //不是最后一个就把下一个坐标放在文本上
            else
            {
                nIndexManual ++;
                ui->txLineEdit->setText(sendManualData.at(nIndexManual));
            }
        }
    }

}

void MainWindow::on_resetInitButton_clicked()
{
    nIndexInit = 0;
    ui->statusBar->showMessage(GetCurrentTime() + " - " + QString::fromLocal8Bit("初始路径复位成功"),1000);
}

void MainWindow::on_resetAutorunButton_clicked()
{
    nIndexAutorun = 0;
    ui->statusBar->showMessage(GetCurrentTime() + " - " + QString::fromLocal8Bit("自动路径复位成功"),1000);
}

void MainWindow::on_resetManualButton_clicked()
{
    nIndexManual = 0;
    ui->statusBar->showMessage(GetCurrentTime() + " - " + QString::fromLocal8Bit("人工路径复位成功"),1000);
}

void MainWindow::on_isShowTimeCheckBox_stateChanged(int arg1)
{
    (arg1 == 0) ? isShowTimeFlag = false : isShowTimeFlag = true;
}



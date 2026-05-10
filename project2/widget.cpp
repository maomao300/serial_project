#include "widget.h"
#include "ui_widget.h"
#include <QSerialPortInfo>
#include <QDebug>
#include <QMessageBox>
#include <QFileDialog>
#include <QTime>
#include <QThread>
#include <QFileDialog>
Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    this->setLayout(ui->gridLayout_All);

    //控制参数初始化
    write_size_total=0;
    read_size_total=0;
    buttonindex=0;
    serialstatus =false;
    //控件初始化
    ui->pushButton_send->setEnabled(false);
    ui->checkBox_sendintime->setEnabled(false);
    ui->checkBox_sendnewh->setEnabled(false);
    ui->checkBox_sendhex->setEnabled(false);
    ui->pushButton_Save->setEnabled(false);
    ui->pushButton_import->setEnabled(false);
    ui->pushButton_reset->setEnabled(false);



    //定时器，定时发送
    timer=new QTimer(this);
    connect(timer,&QTimer::timeout,[=](){
        on_pushButton_send_clicked();
    });

    //定时器，每100ms刷新日期和时间
    QTimer* getsystimetime=new QTimer(this);
    getsystimetime->start(100);
    connect(getsystimetime,&QTimer::timeout,this,&Widget::refresh_time);

    //定时器，多文本处的循环定时发送
    circletime=new QTimer(this);
    connect(circletime,&QTimer::timeout,this,&Widget::circlesend_handle);

    //窗口加入一个串口控制对象
    serialport =new QSerialPort(this);
    //启动后更新加载系统中的可用串口号
    serial_Num_refresh();

    //串口读数据
    connect(serialport,&QSerialPort::readyRead,this,&Widget::serial_Ready_to_readData);
    //可用串口号的更新
    connect(ui->comboBox_Num,&mycombox::refresh,this,&Widget::serial_Num_refresh);
    //当前串口状态
    ui->label_sendstatus->setText(ui->comboBox_Num->currentText()+"NoOpen!");



    //串口控件初始化
    ui->comboBox_Data->setCurrentIndex(3);
    ui->comboBox_Rate->setCurrentIndex(6);



    
    //多文本的button控件、checkbox控件、lineEdit控件
    for(int i=1;i<=9;i++)
    {
        //button控件
        QString buttonname=QString("pushButton_%1").arg(i);
        QPushButton* button=findChild<QPushButton*>(buttonname);
        if(button)
        {
            button->setProperty("buttonid",i);
            buttons.append(button);
            connect(button,SIGNAL(clicked()),this,SLOT(Muitext_button_clicked()));
        }

        //checkbox控件
        QString checkboxname=QString("checkBox_%1").arg(i);
        QCheckBox* checkbox=findChild<QCheckBox*>(checkboxname);
        if(checkbox)
        {
            checkboxs.append(checkbox);
        }

        //lineEdit控件
        QString lineeditname=QString("lineEdit_%1").arg(i);
        QLineEdit* lineedit=findChild<QLineEdit*>(lineeditname);
        if(lineedit)
        {
            lineedits.append(lineedit);
        }
    }

}

Widget::~Widget()
{
    delete ui;
}


void Widget::on_pushButton_close_or_open_clicked()
{
    if(!serialstatus)
    {
        //    1.选择串口号
        serialport->setPortName(ui->comboBox_Num->currentText());
        //    2.设置波特率
        serialport->setBaudRate(ui->comboBox_Rate->currentText().toInt());
        //    3.设置数据位
        serialport->setDataBits(QSerialPort:: DataBits(ui->comboBox_Data->currentText().toInt()));
        //    4.设置校验位
        switch (ui->comboBox_Jiaoyan->currentIndex()) {
        case 0:
            serialport->setParity(QSerialPort::NoParity);
            break;
        case 1:
            serialport->setParity(QSerialPort::EvenParity);
            break;
        case 2:
            serialport->setParity(QSerialPort::MarkParity);
            break;
        case 3:
            serialport->setParity(QSerialPort::OddParity);
            break;
        case 4:
            serialport->setParity(QSerialPort::SpaceParity);
            break;
        default:
            break;
        }
        //    5.设置停止位
        switch (ui->comboBox_Stop->currentIndex()) {
        case 0:
            serialport->setStopBits(QSerialPort::OneStop);
            break;
        case 1:
            serialport->setStopBits(QSerialPort::OneAndHalfStop);
            break;
        case 2:
            serialport->setStopBits(QSerialPort::TwoStop);
            break;
        case 3:
            serialport->setStopBits(QSerialPort::UnknownStopBits);
            break;
        default:
            break;

        }


        //    6.设置流控
        if(ui->comboBox_Flowcon->currentText()=="None")
        {
            serialport->setFlowControl(QSerialPort::NoFlowControl);
        }

        //   7.打开串口
        if(serialport->open(QIODevice::ReadWrite))
        {
            ui->comboBox_Num->setEnabled(false);
            ui->comboBox_Rate->setEnabled(false);
            ui->comboBox_Data->setEnabled(false);
            ui->comboBox_Jiaoyan->setEnabled(false);
            ui->comboBox_Stop->setEnabled(false);
            ui->comboBox_Flowcon->setEnabled(false);
            ui->pushButton_close_or_open->setText("关闭串口");
            ui->pushButton_send->setEnabled(true);
            ui->lineEdit_sendcontext->setEnabled(true);
            ui->checkBox_sendintime->setEnabled(true);
            ui->checkBox_sendnewh->setEnabled(true);
            ui->checkBox_sendhex->setEnabled(true);
            ui->pushButton_Save->setEnabled(true);
            ui->pushButton_import->setEnabled(true);
            ui->pushButton_reset->setEnabled(true);
            serialstatus=true;
            ui->label_sendstatus->setText(ui->comboBox_Num->currentText()+"isOpen!");
        }
        else
        {
            QMessageBox msgbox;
            msgbox.setWindowTitle("串口打开错误");
            msgbox.setText("打开失败，串口可能被占用或已拔出");
            msgbox.exec();
        }
    }
    else
    {
        serialport->close();
        ui->pushButton_close_or_open->setText("打开串口");
        ui->comboBox_Num->setEnabled(true);
        ui->comboBox_Rate->setEnabled(true);
        ui->comboBox_Data->setEnabled(true);
        ui->comboBox_Jiaoyan->setEnabled(true);
        ui->comboBox_Stop->setEnabled(true);
        ui->comboBox_Flowcon->setEnabled(true);
        ui->pushButton_send->setEnabled(false);
        ui->checkBox_sendintime->setEnabled(false);
        ui->checkBox_sendintime->setCheckState(Qt::Unchecked);
        ui->lineEdit_sendtinum->setEnabled(true);
        ui->lineEdit_sendcontext->setEnabled(true);
        ui->checkBox_sendnewh->setEnabled(false);
        ui->checkBox_sendhex->setEnabled(false);
        ui->pushButton_Save->setEnabled(false);
        ui->pushButton_import->setEnabled(false);
        ui->pushButton_reset->setEnabled(false);
        timer->stop();
        serialstatus=false;
        ui->label_sendstatus->setText(ui->comboBox_Num->currentText()+"isClose!");
    }
}

void Widget::on_pushButton_send_clicked()
{
    int writesize=0;
    QMessageBox msgbox;
    const char*senddata=ui->lineEdit_sendcontext->text().toLocal8Bit().constData();
    if(ui->checkBox_sendhex->isChecked())
    {
        QString temp=ui->lineEdit_sendcontext->text();
        QByteArray temparray=temp.toUtf8();
        //判断是否是偶数，每两个十六进制字符对应一个字节
        if(temparray.size()%2!=0)
        {
            ui->label_sendstatus->setText("Input Error");
            msgbox.setWindowTitle("提示");
            msgbox.setIcon(QMessageBox::Warning);
            msgbox.setText("输入格式错误");
            msgbox.exec();
            return;
        }

        //判断是否是16进制
        for(char c:temparray)
        {
            if(!std::isxdigit(c))
            {
                ui->label_sendstatus->setText("Input Error");
                msgbox.setWindowTitle("提示");
                msgbox.setIcon(QMessageBox::Warning);
                msgbox.setText("输入格式错误");
                msgbox.exec();
                return;
            }
        }
        if(ui->checkBox_sendnewh->isChecked())
        {
            temparray.append("\r\n");
        }
        //转成16进制发送
        QByteArray tempsend=QByteArray::fromHex(temparray);

        writesize=serialport->write(tempsend);

    }
    else{
        if(ui->checkBox_sendnewh->isChecked())
        {
            QByteArray qtemp(senddata,strlen(senddata));
            qtemp.append("\r\n");
            writesize=serialport->write(qtemp);
        }
        else
            writesize=serialport->write(senddata);
    }

    if(writesize==-1)
    {
        ui->label_sendstatus->setText("SendError!");
    }
    else
    {
        write_size_total+=writesize;
        ui->label_sendstatus->setText("SendOk!");
        ui->label_sendsize->setText("sent:"+QString::number(write_size_total));
        if(strcmp(senddata,sendback.toStdString().c_str())!=0)
        {
            ui->textEditHsRecord->append(senddata);
            sendback=QString::fromUtf8(senddata);
        }

    }

}

void Widget::serial_Ready_to_readData()
{
    QString serialRevdata = serialport->readAll();
    if(serialRevdata!=NULL)
    {
        if(ui->checkBox_nexth->isChecked())serialRevdata.append("\r\n");
        if(ui->checkBox_shex->isChecked())
        {
            QByteArray data=serialRevdata.toUtf8().toHex().toUpper();
            serialRevdata =QString::fromUtf8(data);
            QString olddata=ui->textEditRev->toPlainText();
            ui->textEditRev->setText(olddata+serialRevdata);//旧的，勾选了HEX，读出来就是HEX，将旧的和新收到的数据转成HEX，进行拼接
        }
        else
        {
            if(ui->checkBox_savetime->checkState()==Qt::Unchecked)
            {
                ui->textEditRev->insertPlainText(serialRevdata);
            }
            else if(ui->checkBox_savetime->checkState()==Qt::Checked)
            {
                getTime();
                ui->textEditRev->insertPlainText("【"+mytime+"】"+serialRevdata);
            }
        }
        read_size_total+=serialRevdata.size();
        ui->label_revsize->setText("received:"+QString::number(read_size_total));
        ui->textEditRev->moveCursor(QTextCursor::End);
        ui->textEditRev->ensureCursorVisible();

    }
}

void Widget::on_checkBox_sendintime_clicked(bool checked)
{
    if(checked)
    {
        ui->lineEdit_sendtinum->setEnabled(false);
        ui->lineEdit_sendcontext->setEnabled(false);
        ui->pushButton_send->setEnabled(false);
        timer->start(ui->lineEdit_sendtinum->text().toInt());
    }
    else{
        timer->stop();
        ui->lineEdit_sendtinum->setEnabled(true);
        ui->lineEdit_sendcontext->setEnabled(true);
        ui->pushButton_send->setEnabled(true);
    }
}

void Widget::on_pushButton_reclear_clicked()
{
    ui->textEditRev->clear();
}

void Widget::on_pushButton_resave_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"),
                                                    "D:/Project/QTproject",
                                                    tr("Text (*.txt)"));
    if(fileName!=NULL)
    {
        QFile file(fileName);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
            return;

        QTextStream out(&file);
        out <<ui->textEditRev->toPlainText();
        file.close();
    }
}

void Widget::refresh_time()
{
    getTime();
    ui->label_time->setText(mytime);
}

void Widget::getTime()
{
    QDateTime currentTime=QDateTime::currentDateTime();
    QDate data=currentTime.date();
    int year=data.year();
    int month=data.month();
    int day=data.day();

    QTime time=currentTime.time();
    int hour=time.hour();
    int minute=time.minute();
    int second=time.second();

    mytime=QString("%1-%2-%3  %4:%5:%6")
            .arg(year,2,10,QChar('0'))
            .arg(month,2,10,QChar('0'))
            .arg(day,2,10,QChar('0'))
            .arg(hour,2,10,QChar('0'))
            .arg(minute,2,10,QChar('0'))
            .arg(second,2,10,QChar('0'));

}

void Widget::on_checkBox_shex_clicked(bool checked)
{
    if(checked)
    {
        //读取textEdit的上的内容
        QString temp=ui->textEditRev->toPlainText();
        //转换为hex
        QByteArray qtemp=temp.toUtf8();
        qtemp=qtemp.toHex();
        temp=QString::fromUtf8(qtemp);
        QString lastshow;
        for(int i=0;i<qtemp.size();i+=2)
        {
            lastshow+=temp.mid(i,2)+" ";
        }
        //显示
        ui->textEditRev->setText(lastshow.toUpper());
    }
    else
    {
        QString temp=ui->textEditRev->toPlainText();
        QByteArray qtemp=temp.toUtf8();
        qtemp=QByteArray::fromHex(qtemp);
        ui->textEditRev->setText(QString::fromUtf8(qtemp));
    }
    ui->textEditRev->moveCursor(QTextCursor::End);
    ui->textEditRev->ensureCursorVisible();

}

void Widget::on_pushButton_htable_clicked(bool checked)
{
    if(checked)
    {
        ui->pushButton_htable->setText("拓展面板");
        ui->groupBoxMuText->hide();
    }
    else
    {
        ui->pushButton_htable->setText("隐藏面板");
        ui->groupBoxMuText->show();
    }
}

void Widget::on_pushButton_hhistory_clicked(bool checked)
{
    if(checked)
    {
        ui->pushButton_hhistory->setText("显示历史");
        ui->groupBoxHsRecord->hide();
    }
    else
    {
        ui->pushButton_hhistory->setText("隐藏历史");
        ui->groupBoxHsRecord->show();
    }
}

void Widget::serial_Num_refresh()
{
    ui->comboBox_Num->clear();
    QList<QSerialPortInfo> serialList=QSerialPortInfo::availablePorts();
    for(QSerialPortInfo serialportinfo:serialList)
    {
        ui->comboBox_Num->addItem(serialportinfo.portName());
    }
    ui->label_sendstatus->setText("Refresh!");
}

void Widget::Muitext_button_clicked()
{
    QPushButton* button=qobject_cast<QPushButton*>(sender());
    if(button)
    {
        int num=button->property("buttonid").toInt();

        QString checkboxname=QString("checkBox_%1").arg(num);
        QCheckBox* checkbox=findChild<QCheckBox*>(checkboxname);
        if(checkbox)
        {
            ui->checkBox_sendhex->setChecked(checkbox->isChecked());
        }

        QString lineeditname=QString("lineEdit_%1").arg(num);
        QLineEdit* lineedit=findChild<QLineEdit*>(lineeditname);
        if(lineedit)
        {
            if(lineedit->text().size()<=0)
            {
                return;
            }
            ui->lineEdit_sendcontext->setText(lineedit->text());
        }
        on_pushButton_send_clicked();
    }
}

void Widget::on_checkBox_send_clicked(bool checked)
{
    if(checked)
    {
        circletime->start(ui->spinBox->text().toInt());
        ui->spinBox->setEnabled(false);
    }
    else
    {
        ui->spinBox->setEnabled(true);
        circletime->stop();
    }

}

void Widget::circlesend_handle()
{
    if(buttonindex<buttons.size())
    {
        QPushButton* tempbutton=buttons[buttonindex];
        emit tempbutton->clicked();
        buttonindex++;
    }
    else
        buttonindex=0;
}

void Widget::on_pushButton_Save_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("保存文件"),
                               "D:/Project/QTproject",
                               tr("Text(*.txt)"));
        if(fileName!=NULL)
        {
            QFile file(fileName);
                 if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
                     return;

                 QTextStream out(&file);
                 for(int i=0;i<checkboxs.size();i++)
                 {
                     out<<checkboxs[i]->isChecked()<<","<<lineedits[i]->text()<<'\n';
                 }
                 file.close();
        }
}



void Widget::on_pushButton_import_clicked()
{
    int i=0;
    QString fileName = QFileDialog::getOpenFileName(this,
         tr("打开文件"), "D:/Project/QTproject", tr("Text(*.txt)"));
    if(fileName!=NULL)
    {
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
            return;
        QTextStream in(&file);
        while (!in.atEnd()&&i<9) {
            QString line = in.readLine();
            QStringList parts=line.split(",");
            if(parts.count()==2)
            {
                 checkboxs[i]->setChecked(parts[0].toInt());
                 lineedits[i]->setText(parts[1]);
            }
            i++;
        }
        file.close();
    }
}

void Widget::on_pushButton_reset_clicked()
{
    QMessageBox msgbox;
    msgbox.setWindowTitle("提示");
    msgbox.setIcon(QMessageBox::Question);
    msgbox.setText("重置列表不可逆，是否重置列表？");
    QPushButton * yesbutton=msgbox.addButton("是",QMessageBox::YesRole);
    QPushButton * nobutton=msgbox.addButton("否",QMessageBox::NoRole);
    msgbox.exec();
    if(msgbox.clickedButton()==yesbutton)
    {
        for(int i=0;i<checkboxs.size();i++)
        {
            lineedits[i]->clear();
            checkboxs[i]->setChecked(false);
        }
    }
    if(msgbox.clickedButton()==nobutton)
    {

    }
}

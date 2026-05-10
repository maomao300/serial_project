#ifndef WIDGET_H
#define WIDGET_H

#include <QCheckBox>
#include <QPushButton>
#include <QSerialPort>
#include <QTimer>
#include <QWidget>
#include "mycombox.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();


private slots:

    void on_pushButton_close_or_open_clicked();
    void on_pushButton_send_clicked();
    void serial_Ready_to_readData();

    void on_checkBox_sendintime_clicked(bool checked);

    void on_pushButton_reclear_clicked();

    void on_pushButton_resave_clicked();
    void refresh_time();

    void on_checkBox_shex_clicked(bool checked);

    void on_pushButton_htable_clicked(bool checked);

    void on_pushButton_hhistory_clicked(bool checked);
    void serial_Num_refresh();
    void Muitext_button_clicked();

    void on_checkBox_send_clicked(bool checked);
    void circlesend_handle();

    void on_pushButton_Save_clicked();

    void on_pushButton_import_clicked();

    void on_pushButton_reset_clicked();

private:
    Ui::Widget *ui;
    QSerialPort* serialport;
    QTimer* timer;
    int write_size_total;
    int read_size_total;
    QString sendback;
    bool serialstatus;
    QString mytime;
    QList<QPushButton*>buttons;
    QList<QCheckBox*>checkboxs;
    QList<QLineEdit*>lineedits;
    int buttonindex;
    QTimer* circletime;
    void getTime();
};
#endif // WIDGET_H

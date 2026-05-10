#ifndef MYCOMBOX_H
#define MYCOMBOX_H

#include <QComboBox>



class mycombox : public QComboBox
{
    Q_OBJECT
public:
    mycombox(QWidget* parent);
private slots:
    void mousePressEvent(QMouseEvent *e) override;
signals:
    void refresh();
};

#endif // MYCOMBOX_H

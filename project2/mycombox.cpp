#include "mycombox.h"

#include <QMouseEvent>


mycombox::mycombox(QWidget *parent):QComboBox(parent)
{

}

void mycombox::mousePressEvent(QMouseEvent *e)
{
    if(e->button()==Qt::LeftButton )
    {
        emit  refresh();
    }
    QComboBox::mousePressEvent(e);
}

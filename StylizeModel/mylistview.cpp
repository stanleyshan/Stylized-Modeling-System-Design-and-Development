#include "mylistview.h"

#include <QDebug>
#include <QtMath>
#include <math.h>

MyListView::MyListView(QSize size)
{
    model = new QStandardItemModel();

    //never shows a scroll bar
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    currentValue = 0;
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setWrapping(false);
    setMovement(QListView::Static);
    setViewMode(QListView::IconMode);
    setIconSize(size);
    setLineWidth(0);
    setSpacing(15);
    setModel(model);
}

void MyListView::addItem(QImage i)
{
    QStandardItem* item = new QStandardItem();
    item->setIcon(geticon(i));
    model->appendRow(item);
}

QIcon MyListView::geticon(QImage img)
{
    img = img.scaled(this->iconSize(),Qt::KeepAspectRatio);
    QIcon result(QPixmap::fromImage(img));
    return result;
}

void MyListView::mousePressEvent(QMouseEvent *e)
{
    lastP = e->pos();
    b_move = false;
}

void MyListView::mouseMoveEvent(QMouseEvent * e)
{
    int diff = lastP.x() - e->pos().x();
    if(abs(diff) > 20)
    {
        b_move = true;
        horizontalScrollBar()->setValue(currentValue + diff);
    }
}

void MyListView::mouseReleaseEvent(QMouseEvent * e)
{
    if(!b_move)
    {
        QListView::mousePressEvent(e);
        setCurrentIndex(currentIndex());
    }
    currentValue = horizontalScrollBar()->value();
}



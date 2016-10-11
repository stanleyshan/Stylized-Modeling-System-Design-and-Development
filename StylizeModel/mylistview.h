#ifndef MYLISTVIEW_H
#define MYLISTVIEW_H

#include <QListView>
#include <QMouseEvent>
#include <QScrollBar>
#include <QStandardItemModel>

class MyListView : public QListView
{
public:
    MyListView(QSize size);
    int move = 0;
    void addItem(QImage i);
    QIcon geticon(QImage i);
    void mousePressEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent * e);
    void mouseReleaseEvent(QMouseEvent * e);

private:
    QStandardItemModel *model;
    QPoint lastP;
    int currentValue;
    bool b_move = false;

};

#endif // MYLISTVIEW_H

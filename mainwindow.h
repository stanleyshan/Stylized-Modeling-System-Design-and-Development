#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "mylistview.h"
#include "qopenmeshobject.h"

#include <QMainWindow>
#include <QScreen>
#include <QApplication>
#include <QSize>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QImage>
#include <QStackedWidget>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void initialData();

    static MainWindow *Instance;
    OpenMeshObject *openMeshObject;
    static int deform_type; // 變形部位 (0:眼睛, 1:耳朵, 2:嘴巴, 3:正面鼻子, 4:側邊鼻子, 5:下巴
    static bool b_deform;
    static bool b_enableStroke; // 畫筆觸與選轉視角的操作切換
    static bool b_resetModel; // 更新opengl render的物件
    static bool b_reset; // 重置變形結果
    static float cameraX;
    static float cameraZ;

private:
    Ui::MainWindow *ui;
    QScreen *screen = QApplication::screens().at(0);
    int Screenw = screen->size().width();
    int Screenh = screen->size().height();
    QSize btnsize = QSize(Screenw/5, Screenw/5);
    QSize iconsize = QSize(0.12*Screenh, 0.12*Screenh);

    //QImage - pushbutton
    QImage changecloth = QImage(":/imageFiles/changeclothicon.png");
    QImage changecloth_click = QImage(":/imageFiles/changeclothicon_click.png");
    QImage deform = QImage(":/imageFiles/stroke.png");
    QImage deform_click = QImage(":/imageFiles/stroke_click.png");

    //QImage - mainview
    QImage hat = QImage(":/imageFiles/haticon.png");
    QImage hair = QImage(":/imageFiles/hairicon.png");
    QImage cloth = QImage(":/imageFiles/clothicon.png");
    QImage pant = QImage(":/imageFiles/panticon.png");
    QImage shose = QImage(":/imageFiles/shoseicon.png");
    QImage part = QImage(":/imageFiles/particon.png");
    QImage dress = QImage(":images/dressicon.png");
    QImage suit = QImage(":/imageFiles/suiticon.png");

    //QImage - fivesense
    QImage eyes = QImage(":/imageFiles/eyes.png");
    QImage ears = QImage(":/imageFiles/ears.png");
    QImage lips = QImage(":/imageFiles/lips.png");
    QImage nosefront = QImage(":/imageFiles/nosefront.png");
    QImage noseside = QImage(":/imageFiles/noseside.png");
    QImage chin = QImage(":/imageFiles/chin.png");

    //QPushButton
    QPushButton *changeclothbtn = setbtn(changecloth, btnsize);
    QPushButton *deformbtn = setbtn(deform, btnsize);

    //Slide bar
    //1. changecloth
    MyListView *mainview, *clothview, *pantsview, *onepieceview, *clothsetview, *hatview, *hairview, *shoesview, *partview;
    // subview container
    QStackedWidget *substackwidget = new QStackedWidget;
    //main_costume state
    bool costume_state[8];
    //2. deform
    MyListView *deformview;

private slots:
    QIcon geticon(QImage, QSize);
    QPushButton *setbtn(QImage, QSize);
    void changecloth_func(); // 切換至換裝介面
    void deform_func(); // 切換至變形介面
    void main_costumeclick(const QModelIndex&); // 主選單的點擊事件
    void hatView_click(const QModelIndex&); // 選擇帽子時的點擊事件
    void hairView_click(const QModelIndex&); // 選擇頭髮時的點擊事件
    void clothView_click(const QModelIndex&); // 選擇衣服時的點擊事件
    void pantsView_click(const QModelIndex&); // 選擇褲子時的點擊事件
    void shoesView_click(const QModelIndex&); // 選擇鞋子時的點擊事件
    void partView_click(const QModelIndex&); // 選擇裝飾品時的點擊事件
    void clothSetView_click(const QModelIndex&); // 選擇套裝時的點擊事件
    void deformView_click(const QModelIndex&); // 選擇變形部位時的點擊事件
};

#endif // MAINWINDOW_H

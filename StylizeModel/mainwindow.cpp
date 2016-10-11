#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "plugin.h"
#include "meshrenderer.h"

#include <QDebug>
#include <QDir>

// 初始化static參數
MainWindow *MainWindow::Instance = NULL;
bool MainWindow::b_enableStroke = false;
bool MainWindow::b_deform = false;
bool MainWindow::b_resetModel = false;
bool MainWindow::b_reset = false;
int MainWindow::deform_type = -1;
float MainWindow::cameraX = 0.0f;
float MainWindow::cameraZ = -1.0f;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    openMeshObject(0)
{
    ui->setupUi(this);
    Instance = this;

    /* 設定OpenGL視窗大小*/
    ui->openGLWidget->setGeometry(0, 0, Screenw, Screenh);

    //Pushbutton
    ui->btnwidget->setGeometry(QRect(Screenw - btnsize.width(), 0.6*Screenh - 2*btnsize.height(), btnsize.width(), btnsize.height() * 2.5));
    ui->btnwidget->setStyleSheet("background-color:rgba(0,0,0,0)");
    ui->btnlayout->addWidget(deformbtn);
    ui->btnlayout->addWidget(changeclothbtn);
    connect(deformbtn, SIGNAL(clicked()), this, SLOT(deform_func()));
    connect(changeclothbtn, SIGNAL(clicked()), this, SLOT(changecloth_func()));

    mainview = new MyListView(iconsize);
    mainview->addItem(hat);
    mainview->addItem(hair);
    mainview->addItem(cloth);
    mainview->addItem(pant);
    mainview->addItem(shose);
    mainview->addItem(part);
    mainview->addItem(suit);
    connect(mainview, SIGNAL(pressed(QModelIndex)), this, SLOT(main_costumeclick(QModelIndex)));

    //hat
    hatview = new MyListView(iconsize);
    hatview->addItem(QImage(":/imageFiles/icon_hat1.png"));
    hatview->addItem(QImage(":/imageFiles/icon_hat2.png"));
    connect(hatview, SIGNAL(pressed(QModelIndex)), this, SLOT(hatView_click(QModelIndex)));

    //hair
    hairview = new MyListView(iconsize);
    hairview->addItem(QImage(":/imageFiles/icon_hair1.png"));
    hairview->addItem(QImage(":/imageFiles/icon_hair2.png"));
    connect(hairview, SIGNAL(pressed(QModelIndex)), this, SLOT(hairView_click(QModelIndex)));

    //cloth
    clothview = new MyListView(iconsize);
    clothview->addItem(QImage(":/imageFiles/icon_cloth1.png"));
    clothview->addItem(QImage(":/imageFiles/icon_cloth2.png"));
    connect(clothview, SIGNAL(pressed(QModelIndex)), this, SLOT(clothView_click(QModelIndex)));

    //pant
    pantsview = new MyListView(iconsize);
    pantsview->addItem(QImage(":/imageFiles/icon_pants1.png"));
    connect(pantsview, SIGNAL(pressed(QModelIndex)), this, SLOT(pantsView_click(QModelIndex)));

    //shose
    shoesview = new MyListView(iconsize);
    shoesview->addItem(QImage(":/imageFiles/icon_shoes1.png"));
    shoesview->addItem(QImage(":/imageFiles/icon_shoes2.png"));
    connect(shoesview, SIGNAL(pressed(QModelIndex)), this, SLOT(shoesView_click(QModelIndex)));

    //part
    partview = new MyListView(iconsize);
    partview->addItem(QImage(":/imageFiles/icon_decoration1.png"));
    partview->addItem(QImage(":/imageFiles/icon_decoration2.png"));
    connect(partview, SIGNAL(pressed(QModelIndex)), this, SLOT(partView_click(QModelIndex)));

    //clothset
    clothsetview = new MyListView(iconsize);
    clothsetview->addItem(QImage(":/imageFiles/icon_ironman.png"));
    clothsetview->addItem(QImage(":/imageFiles/icon_luffy.png"));
    clothsetview->addItem(QImage(":/imageFiles/icon_minion.png"));
    clothsetview->addItem(QImage(":/imageFiles/icon_chopper.png"));
    connect(clothsetview, SIGNAL(pressed(QModelIndex)), this, SLOT(clothSetView_click(QModelIndex)));

    //slide bar
    //1.change cloth-main
    ui->main_costumewidget->setGeometry(QRect(0, 0.8*Screenh, Screenw, 0.2*Screenh));
    ui->main_costumewidget->setStyleSheet("background-color:rgba(207,182,137,255)");
    ui->main_costumewidget->setVisible(false);
    ui->main_costumewidget->raise();
    ui->main_costumelayout->addWidget(mainview);

    //subview
    substackwidget->addWidget(hatview);
    substackwidget->addWidget(hairview);
    substackwidget->addWidget(clothview);
    substackwidget->addWidget(pantsview);
    substackwidget->addWidget(shoesview);
    substackwidget->addWidget(partview);
    substackwidget->addWidget(clothsetview);

    //sub_costume
    ui->sub_costumewidget->setGeometry(QRect(0, 0.65*Screenh, Screenw, 0.2*Screenh));
    ui->sub_costumewidget->setStyleSheet("background-color:rgba(207,182,137,255)");
    ui->sub_costumewidget->setVisible(false);
    ui->sub_costumelayout->addWidget(substackwidget);

    //deformview
    deformview = new MyListView(iconsize);
    deformview->addItem(eyes);
    deformview->addItem(ears);
    deformview->addItem(lips);
    deformview->addItem(nosefront);
    deformview->addItem(noseside);
    deformview->addItem(chin);
    deformview->addItem(QImage(":/imageFiles/reset.png"));
    connect(deformview, SIGNAL(pressed(QModelIndex)), this, SLOT(deformView_click(QModelIndex)));

    //2.deform
    ui->deformwidget->setGeometry(QRect(0, 0.8*Screenh, Screenw, 0.2*Screenh));
    ui->deformwidget->setStyleSheet("background-color:rgba(72,119,137,255)");
    ui->deformwidget->setVisible(false);
    ui->deformlayout->addWidget(deformview);
}

MainWindow::~MainWindow()
{
    delete ui;
}

/*
 * changecloth_func
 *
 * 描述: 切換至變裝介面
 *
 * 輸入: 無
 * 輸出: 無
*/
void MainWindow::changecloth_func()
{
    b_enableStroke = false;
    b_deform = false;
    cameraZ = -1.0f;
    deformbtn->setIcon(geticon(deform,btnsize));
    changeclothbtn->setIcon(geticon(changecloth_click,btnsize));
    ui->main_costumewidget->setVisible(true);
    ui->sub_costumewidget->setVisible(false);
    ui->deformwidget->setVisible(false);
    ui->openGLWidget->reset();
    ui->openGLWidget->update();
}

/*
 * deform_func
 *
 * 描述: 切換至變形介面
 *
 * 輸入: 無
 * 輸出: 無
*/
void MainWindow::deform_func()
{
    b_deform = true;
    cameraZ = 0.0f;
    deformbtn->setIcon(geticon(deform_click,btnsize));
    changeclothbtn->setIcon(geticon(changecloth,btnsize));
    ui->deformwidget->setVisible(true);
    ui->main_costumewidget->setVisible(false);
    ui->sub_costumewidget->setVisible(false);
}

/*
 * main_costumeclick
 *
 * 描述: 主選單的點擊事件
 *
 * 輸入: const QModelIndex &index - 點擊項目的index
*/
void MainWindow::main_costumeclick(const QModelIndex &index)
{
    MeshRenderer::main_costume_idx = index.row();
    if(!costume_state[index.row()])
    {
        if(substackwidget->currentIndex() != index.row())
            costume_state[substackwidget->currentIndex()] = false;
        substackwidget->setCurrentIndex(index.row());
        ui->sub_costumewidget->setVisible(true);
        costume_state[index.row()] = true;
    }
    else
    {
        ui->sub_costumewidget->setVisible(false);
        costume_state[index.row()] = false;
    }
}

/*
 * hatView_click
 *
 * 描述:  選擇帽子時的點擊事件
 *
 * 輸入: const QModelIndex &index - 點擊項目的index
*/
void MainWindow::hatView_click(const QModelIndex &index)
{
    MeshRenderer::sub_constume_idx = index.row();
    b_resetModel = true;
    ui->sub_costumewidget->setVisible(false);
    costume_state[MeshRenderer::main_costume_idx] = false;
    ui->openGLWidget->update();
}

/*
 * hairView_click
 *
 * 描述:  選擇頭髮時的點擊事件
 *
 * 輸入: const QModelIndex &index - 點擊項目的index
*/
void MainWindow::hairView_click(const QModelIndex &index)
{
    MeshRenderer::sub_constume_idx = index.row();
    b_resetModel = true;
    ui->sub_costumewidget->setVisible(false);
    costume_state[MeshRenderer::main_costume_idx] = false;
    ui->openGLWidget->update();
}

/*
 * clothView_click
 *
 * 描述:  選擇衣服時的點擊事件
 *
 * 輸入: const QModelIndex &index - 點擊項目的index
*/
void MainWindow::clothView_click(const QModelIndex &index)
{
    MeshRenderer::sub_constume_idx = index.row();
    b_resetModel = true;
    ui->sub_costumewidget->setVisible(false);
    costume_state[MeshRenderer::main_costume_idx] = false;
    ui->openGLWidget->update();
}

/*
 * pantsView_click
 *
 * 描述:  選擇褲子時的點擊事件
 *
 * 輸入: const QModelIndex &index - 點擊項目的index
*/
void MainWindow::pantsView_click(const QModelIndex &index)
{
    MeshRenderer::sub_constume_idx = index.row();
    b_resetModel = true;
    ui->sub_costumewidget->setVisible(false);
    costume_state[MeshRenderer::main_costume_idx] = false;
    ui->openGLWidget->update();
}

/*
 * shoesView_click
 *
 * 描述:  選擇鞋子時的點擊事件
 *
 * 輸入: const QModelIndex &index - 點擊項目的index
*/
void MainWindow::shoesView_click(const QModelIndex &index)
{
    MeshRenderer::sub_constume_idx = index.row();
    b_resetModel = true;
    ui->sub_costumewidget->setVisible(false);
    costume_state[MeshRenderer::main_costume_idx] = false;
    ui->openGLWidget->update();
}

/*
 * partView_click
 *
 * 描述:  選擇裝飾品時的點擊事件
 *
 * 輸入: const QModelIndex &index - 點擊項目的index
*/
void MainWindow::partView_click(const QModelIndex &index)
{
    MeshRenderer::sub_constume_idx = index.row();
    b_resetModel = true;
    ui->sub_costumewidget->setVisible(false);
    costume_state[MeshRenderer::main_costume_idx] = false;
    ui->openGLWidget->update();
}

/*
 * clothSetView_click
 *
 * 描述:  選擇套裝時的點擊事件
 *
 * 輸入: const QModelIndex &index - 點擊項目的index
*/
void MainWindow::clothSetView_click(const QModelIndex &index)
{
    MeshRenderer::sub_constume_idx = index.row();
    b_resetModel = true;
    ui->sub_costumewidget->setVisible(false);
    costume_state[MeshRenderer::main_costume_idx] = false;
    ui->openGLWidget->update();
}

/*
 * deformView_click
 *
 * 描述:  選擇變形部位時的點擊事件
 *
 * 輸入: const QModelIndex &index - 點擊項目的index
*/
void MainWindow::deformView_click(const QModelIndex &index)
{
    if(index.row() < 6)
    {
        b_enableStroke = true;
        deform_type = index.row();
    }
    else
    {
        b_reset = true;
    }
    ui->openGLWidget->update();
}

QIcon MainWindow::geticon(QImage i, QSize s)
{
    i = i.scaled(s,Qt::KeepAspectRatio);
    QIcon result(QPixmap::fromImage(i));
    return result;
}

QPushButton *MainWindow::setbtn(QImage i, QSize s)
{
    QPushButton *btn = new QPushButton();
    btn->setFlat(true);
    btn->setStyleSheet("border:0px; outline:0px");
    btn->setIconSize(s);
    btn->setIcon(geticon(i,s));
    return btn;
}

/*
 * initialData
 *
 * 描述: 初始化3D模型
*/
void MainWindow::initialData()
{
    if (!openMeshObject )
    {
//        QString fileName = Plugin::m_appFileDirectory+"/modelFiles/boy15.obj";
        QString fileName = Plugin::m_appFileDirectory+"/modelFiles/boyWithoutEye.obj";
        openMeshObject = new OpenMeshObject(fileName);
    }
}

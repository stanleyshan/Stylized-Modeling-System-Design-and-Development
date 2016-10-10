#define DLIB_JPEG_SUPPORT

#include <vector>

#include <Qdebug>
#include <QGuiApplication>
#include <QBuffer>
#include <QPixmap>
#include <QDir>
#include <QJsonDocument>
#include <QPainter>
#include <QElapsedTimer>

#include "mainwindow.h"
#include "plugin.h"

//using namespace dlib;
using namespace std;

//初始化static參數
QString Plugin::m_appFileDirectory = "";
QString Plugin::m_imagePath = "";
QList<double> Plugin::m_featurePos = QList<double>();
Rect Plugin::m_faceRect = Rect();

Plugin::Plugin(QObject *parent)
    : QObject(parent),
      clm(0),
      params(0)
{
    //取得app的根目錄路徑
    QDir dir = QDir::current();
    m_appFileDirectory = dir.absolutePath();
    qDebug() << dir.absolutePath();

    //將.obj檔存入手機根目錄中
    saveFile("boy13.obj");
    saveFile("boy15.obj");
    saveFile("boyWithoutEye.obj");
    saveFile("boytest.obj");
    saveFile("broom.obj");
    saveFile("chopper.obj");
    saveFile("cloth01.obj");
    saveFile("cloth02.obj");
    saveFile("gloves01.obj");
    saveFile("gloves02.obj");
    saveFile("hair01.obj");
    saveFile("hair02.obj");
    saveFile("hat01.obj");
    saveFile("hat02.obj");
    saveFile("ironman1.obj");
    saveFile("ironman2.obj");
    saveFile("ironman3.obj");
    saveFile("ironman4.obj");
    saveFile("luffy_cloth.obj");
    saveFile("luffy_hat.obj");
    saveFile("luffy_shorts.obj");
    saveFile("luffy_slippers.obj");
    saveFile("minion1.obj");
    saveFile("minion2.obj");
    saveFile("minion3.obj");
    saveFile("minion4.obj");
    saveFile("pants01.obj");
    saveFile("shoes01.obj");
    saveFile("shoes02.obj");
    saveFile("skateboard.obj");

    saveFile("haarcascade_frontalface_alt.xml");

    //saveFile("1000_943_test6.svm");
    //saveFile("1000_1000.dat");

    //初始化json檔案
    m_faceFilter = parseJsonFile(":/jsonFiles/model_pca_20_svm.json");
    m_leftEyeFilter = parseJsonFile(":/jsonFiles/left_eye_filter.json");
    m_rightEyeFilter = parseJsonFile(":/jsonFiles/right_eye_filter.json");
    m_noseFilter = parseJsonFile(":/jsonFiles/nose_filter.json");

    qDebug()<<"check point1 !!";

    clm = new Clm(params);
    clm->init(m_faceFilter, m_leftEyeFilter, m_rightEyeFilter, m_noseFilter);

   //初始化臉部偵測模型檔案
   //std::string face_model_path = (m_appFileDirectory+"/face_modelFiles/1000_943_test6.svm").toStdString();//資料放於/資源->svm.qrc
   //dlib::deserialize(face_model_path) >> detector;

   dlib::deserialize("/storage/emulated/0/Download/1000_943_test6.svm") >> detector;//svm放手機用，shan

   //初始化臉部特徵點偵測模型檔案
   //std::string face_landmark_model_path = (m_appFileDirectory+"/face_modelFiles/1000_1000.dat").toStdString();//資料放於/資源->svm.qrc
   //dlib::deserialize(face_landmark_model_path) >> landmark_detector;

   dlib::deserialize("/storage/emulated/0/Download/1000_1000.dat") >> landmark_detector;//data放手機用，shan

   qDebug()<<"check point2 !!";
}

/*
 * FindFeatures
 *
 * 描述: 透過plugin的方式，呼叫Android專案中的函式，計算出人臉特徵點。
 *
 * 輸入: const QString &imagePath - 影像路徑
 * 輸出: 無
*/
void Plugin::findFeatures(const QString &imagePath)
{
    qDebug() << m_appFileDirectory;


    QElapsedTimer timer;
    timer.start();
    qDebug()<<"Timer start!!";
    qDebug() << "The start time:" << timer.elapsed() << "milliseconds";


    /*使用單張圖片的方式(需要修改三個參數)
     * 1. set std::string a in plugin.cpp
     *  example: std::string a = "/storage/emulated/0/DCIM/1345.jpg";
     *
     * 2. change b_test parameter to "true" in 資源->qrcFiles->qml.qrc->/->main.qml
     *
     * 3. change photoPathTest parameter to file path in main.qml
     *  example: property string photoPathTest: "/storage/sdcard1/Download/39.jpg"
    */


    //std::string a = imagePath.toUtf8().constData();//相機路徑

    std::string a = "/storage/emulated/0/DCIM/25.jpg";

    std::string original = a;

    QString b = QString::fromStdString(a);
    m_img = QImage(b, "JPEG");

    int w = m_img.width();
    int h = m_img.height();

    double scale = 1.0;

    qDebug()<<"Start image rotation";
    qDebug() << "Time: " << timer.elapsed() << "milliseconds";

    QString rotate_path;


    //圖片旋轉(如果寬大於高)
    if(w > h)
    {
        rotate_path = m_appFileDirectory + "/rotate.jpg";
        qDebug() << "totate!";
        QTransform transform;
        transform.rotate(-90);
        m_img = m_img.transformed(transform);

        m_img.save(rotate_path);
        //m_img = QImage(rotate_path, "JPEG"); //這行可以拿掉，節省0.2s

        w = m_img.width();
        h = m_img.height();

        original = rotate_path.toUtf8().constData();

    }


    qDebug()<<"Finish image rotation";
    qDebug() << "Time: " << timer.elapsed() << "milliseconds";


    //圖片縮放
    qDebug()<<"Start image scaling";
    qDebug() << "Time: " << timer.elapsed() << "milliseconds";

    if(w > 1000 || h > 1000){
        scale = 4.0; //縮放程度可以自行調整，會對程式運行速度以及face detection結果造成影響
    }
    else if(w > 600 || h > 600){
        scale = 2.0;
    }

    if(scale != 1.0){
        m_img = m_img.scaled(w/scale,h/scale,Qt::KeepAspectRatio);
        m_img.save(m_appFileDirectory+"/scaled.jpg");

        QString path  = m_appFileDirectory+"/scaled.jpg";
        a = path.toUtf8().constData();
    }


    qDebug()<<"Finish image scaling";
    qDebug() << "Time: " << timer.elapsed() << "milliseconds";

    std::vector<dlib::rectangle> dets;
    dlib::rectangle original_dlib;

    dlib::array2d<dlib::rgb_pixel> img,img_original;
    dlib::load_image(img, a);
    dlib::load_image(img_original, original);

    qDebug()<<"load image";
    qDebug() << "Time: " << timer.elapsed() << "milliseconds";


    //start dlib face detection
    //使用方式: detector(圖片)
    dets = detector(img);
    qDebug() << "have " << dets.size() << "facessssssssssssssssssssssssssssssssss";

    if(dets.size() > 0){
        original_dlib.set_left(dets[0].left() * scale);
        original_dlib.set_top(dets[0].top() * scale);
        original_dlib.set_right((dets[0].left()+
                                           dets[0].width()) * scale);
        original_dlib.set_bottom((dets[0].top() +
                                           dets[0].height()) * scale);
    }


    qDebug()<<"finishing dlib face detection";
    qDebug() << "Time: " << timer.elapsed() << "milliseconds";


    std::vector<dlib::full_object_detection> shapes;
    m_featurePos.clear();


    //如果臉部偵測有偵測到就會進入以下的程式碼
    if(dets.size() > 0){
        dlib::full_object_detection shape;

        //取得68個特徵點(landmark)
        //使用方式:landmark_detector(圖片,臉部偵測結果)
        shape = landmark_detector(img_original, original_dlib);

        //臉部偵測結果(紅色點點)
        m_featurePos.push_back(((double)original_dlib.left()));
        m_featurePos.push_back(((double)original_dlib.top()));
        m_featurePos.push_back((double)(original_dlib.left() + original_dlib.width()));
        m_featurePos.push_back((double)(original_dlib.top() + original_dlib.height()));


        shapes.push_back(shape);

        qDebug()<<"fitting end";
        qDebug() << "The fitting took" << timer.elapsed() << "milliseconds";


        //get shapes
        const dlib::full_object_detection& d = shapes[0];


        qDebug()<<"start to get landmarks";
        for(unsigned long i = 0 ; i < 68 ; i++){
            dlib::point a = d.part(i);
            m_featurePos.push_back((double)a.x());
            m_featurePos.push_back((double)a.y());
        }


        qDebug()<<"geting landmark end";
        qDebug() << "Time" << timer.elapsed() << "milliseconds";
    }
    else{
        //如果都沒有偵測到，則採用測試用的landmark

        qDebug()<<"Didn't detect any face!!";

        double pos[] = {570.613, 1044.9,
                        566.996, 1154.17,
                        590.249, 1279.06,
                        625.843, 1393.81,
                        680.142, 1469.43,
                        748.102, 1538.9,
                        821.147, 1579.37,
                        917.913, 1612.73,
                        1016.34, 1601.73,
                        1091.38, 1545.47,
                        1150.72, 1479.13,
                        1211.3, 1391.4,
                        1239.44, 1287.25,
                        1260.19, 1167.59,
                        1242.62, 1051.67,
                        1207.16, 979.359,
                        1152.79, 953.607,
                        1071.06, 954.986,
                        1005.36, 967.787,
                        646.446, 986.576,
                        701.995, 958.71,
                        785.015, 958.844,
                        851.354, 970.407,
                        695.442, 1060.86,
                        762.016, 1030.47,
                        831.739, 1061.45,
                        761.925, 1078.05,
                        764.919, 1052.53,
                        1159.96, 1057.15,
                        1095.47, 1027.06,
                        1024.35, 1059.23,
                        1095.76, 1075.26,
                        1095.14, 1049.36,
                        926.643, 1035.63,
                        846.226, 1197.25,
                        816.189, 1250.32,
                        844.54, 1287.81,
                        923.205, 1301.29,
                        1002.7, 1287.33,
                        1030.73, 1250.9,
                        1000.92, 1197.25,
                        924.589, 1142.03,
                        867.101, 1267.19,
                        978.791, 1266.53,
                        781.328, 1413.28,
                        830.316, 1383.29,
                        883.892, 1371.98,
                        921.481, 1379.53,
                        958.83, 1371.89,
                        1011.89, 1382.76,
                        1060.45, 1412.51,
                        1024.72, 1447.3,
                        979.866, 1469.05,
                        919.928, 1476.21,
                        860.431, 1470.09,
                        814.871, 1448.01,
                        851.222, 1424.08,
                        920.017, 1430.29,
                        989.613, 1422.96,
                        990.293, 1402.93,
                        920.629, 1405.27,
                        852.352, 1403.29,
                        923.259, 1242.78,
                        723.668, 1039.43,
                        802.767, 1038.03,
                        798.503, 1071.92,
                        725.562, 1072.98,
                        1133.45, 1035.65,
                        1054.07, 1035.52,
                        1058.52, 1069.49,
                        1131.11, 1069.52};

          for(int i=0; i<142; i++)
          {
              m_featurePos.push_back(pos[i]);
          }

    }

}

/*
 * GetFeaturePos
 *
 * 描述: 取得71個特徵點座標
 *
 * 輸入: 無
 * 輸出: QList<double> - 特徵點座標
*/
QList<double> Plugin::getFeaturePos()
{
    return m_featurePos;
}

/*
 * SetFeaturePos
 *
 * 描述: 設定特徵點座標
 *
 * 輸入: int idx - 第幾個特徵點
 *      double x, double y - 特徵點座標
 * 輸出: 無
*/
void Plugin::setFeaturePos(int idx, double x, double y)
{
    if(m_featurePos[0] != -1)
    {
        m_featurePos[idx] = x;
        m_featurePos[idx+1] = y;
    }
}

/*
 * SetFaceRect
 *
 * 描述: 設定人臉的偵測框
 *
 * 輸入: int x, int y - 偵測框的起始點座標
 *      int width - 偵測框的寬
 *      int height - 偵測框的長
 * 輸出: 無
*/
void Plugin::setFaceRect(int x, int y, int width, int height)
{
    QImage tmp = m_img.copy(x, y, width, height);
    tmp.save(m_appFileDirectory+"/face.jpg");

    float scale = 500.0f/m_img.height();

    QVector<double> box = QVector<double>(4);
    box[0] = x*scale;
    box[1] = y*scale;
    box[2] = width*scale;
    box[3] = height*scale;

    m_img.save(m_appFileDirectory+"/element.jpg");
    clm->setRootDirectory(m_appFileDirectory);
    clm->start(m_img, box);

    QVector<QVector<double>> pos = clm->getCurrentPosition();
    m_featurePos.clear();
    for(int i=0; i<pos.length(); i++)
    {
        m_featurePos.push_back(pos[i][0]/scale);
        m_featurePos.push_back(pos[i][1]/scale);
    }
}

/*
 * SaveFile
 *
 * 描述: 存檔
 *
 * 輸入: QString fileName - 檔名
 * 輸出: 無
*/
void Plugin::saveFile(QString fileName)
{
    // 存檔
    QDir dir(m_appFileDirectory+"/");
    if(fileName.contains(".xml"))
    {
        if(!QFile::exists(m_appFileDirectory+"/xmlFiles"))
            dir.mkdir("xmlFiles");

        QFile in(":/xmlFiles/"+fileName);
        in.open(QIODevice::ReadOnly);

        QFile out(m_appFileDirectory+"/xmlFiles/"+fileName);
        out.open(QIODevice::WriteOnly);

        out.write(in.readAll());
        in.close();
        out.close();
    }
    else if(fileName.contains(".obj") || fileName.contains(".mtl"))
    {
        if(!QFile::exists(m_appFileDirectory+"/modelFiles"))
            dir.mkdir("modelFiles");

        QFile in(":modelFiles/"+fileName);
        in.open(QIODevice::ReadOnly);

        QFile out(m_appFileDirectory+"/modelFiles/"+fileName);
        out.open(QIODevice::WriteOnly);

        out.write(in.readAll());
        in.close();
        out.close();
    }
    else if(fileName.contains(".svm")|| fileName.contains(".dat")){
        if(!QFile::exists(m_appFileDirectory+"/face_modelFiles"))
            dir.mkdir("face_modelFiles");

        QFile in(":face_modelFiles/"+fileName);
        in.open(QIODevice::ReadOnly);

        QFile out(m_appFileDirectory+"/face_modelFiles/"+fileName);
        out.open(QIODevice::WriteOnly);

        out.write(in.readAll());
        in.close();
        out.close();
    }
}

/*
 * SwitchToDeformWindow
 *
 * 描述: 切換到變形介面
 *
 * 輸入: 無
 * 輸出: 無
*/
void Plugin::switchToDeformWindow()
{
    createFaceTexture(m_img);
    MainWindow *window = new MainWindow();
    window->show();
}

QJsonObject Plugin::parseJsonFile(QString fileName)
{
    qDebug() << "jsonFileName = " << fileName;
    QFile in(fileName);
    in.open(QIODevice::ReadOnly);

    QString value = (QString) in.readAll();
    QJsonDocument document = QJsonDocument::fromJson(value.toUtf8());

    return document.object();
}

void Plugin::createFaceTexture(QImage pSrc)
{
    //Original picture do equalizeHist
//    Mat newImg(pSrc.height(), pSrc.width(), CV_8UC3, (uchar*)pSrc.bits(), pSrc.bytesPerLine());
//    Mat ycrcb;
//    cvtColor(newImg,ycrcb,CV_BGR2YCrCb);
//    vector<Mat> channels;
//    split(ycrcb,channels);
//    equalizeHist(channels[0], channels[0]);
//    Mat result;
//    merge(channels,ycrcb);
//    cvtColor(ycrcb,result,CV_YCrCb2BGR);

//    QImage QnewImg= QImage(result.data, result.cols, result.rows, result.step, QImage::Format_RGB888);
//    QImage imgSrc = QnewImg.copy(m_faceRect.x, m_faceRect.y, m_faceRect.width, m_faceRect.height);
//    QImage imgMask = QImage(m_faceRect.width, m_faceRect.height, QImage::Format_RGB888);

    QImage imgSrc = pSrc.copy(m_faceRect.x, m_faceRect.y, m_faceRect.width, m_faceRect.height);
    QImage imgMask = QImage(m_faceRect.width, m_faceRect.height, QImage::Format_RGB888);

    // Create the mask
    // Region of left eye
    int tmpStartX = m_featurePos[38] - m_faceRect.x;
    int tmpEndX = m_featurePos[44] - m_faceRect.x;
    int tmpStartY = (m_featurePos[45]+m_featurePos[49])/2.0 - m_faceRect.y;
    int tmpEndY = (m_featurePos[53]+m_featurePos[83])/2.0 - m_faceRect.y;
    for(int i=tmpStartX; i<tmpEndX; i++)
        for(int j=tmpStartY; j<tmpEndY; j++)
            imgMask.setPixel(i, j, qRgb(255, 255, 255));

    // Region of right eye
    tmpStartX = m_featurePos[36] - m_faceRect.x;
    tmpEndX = m_featurePos[30] - m_faceRect.x;
    tmpStartY = (m_featurePos[45]+m_featurePos[49])/2.0 - m_faceRect.y;
    tmpEndY = (m_featurePos[53]+m_featurePos[83])/2.0 - m_faceRect.y;
    for(int i=tmpStartX; i<tmpEndX; i++)
        for(int j=tmpStartY; j<tmpEndY; j++)
            imgMask.setPixel(i, j, qRgb(255, 255, 255));

    // Region of nose
    tmpStartX = m_featurePos[88] - m_faceRect.x;
    tmpEndX = m_featurePos[100] - m_faceRect.x;
    tmpStartY = (m_featurePos[83]+m_featurePos[69])/2.0 - m_faceRect.y;
    tmpEndY = (m_featurePos[75]+m_featurePos[95])/2.0 - m_faceRect.y;
    for(int i=tmpStartX; i<tmpEndX; i++)
        for(int j=tmpStartY; j<tmpEndY; j++)
            imgMask.setPixel(i, j, qRgb(255, 255, 255));

    // Region of mouth
    tmpStartX = m_featurePos[52] - m_faceRect.x;
    tmpEndX = m_featurePos[62] - m_faceRect.x;
    tmpStartY = (m_featurePos[75]+m_featurePos[95])/2.0 - m_faceRect.y;
    tmpEndY = m_featurePos[107] - m_faceRect.y;
    for(int i=tmpStartX; i<tmpEndX; i++)
        for(int j=tmpStartY; j<tmpEndY; j++)
            imgMask.setPixel(i, j, qRgb(255, 255, 255));

    // Scale the image to 400x400
    QTransform transform;
    float s = 400.0f / imgSrc.width();
    transform.scale(s, s);
    imgSrc = imgSrc.transformed(transform);
    imgMask = imgMask.transformed(transform);

    // Save image
    imgSrc.save(m_appFileDirectory+"/src.png");
    imgMask.save(m_appFileDirectory+"/mask.png");

    QImage imgTarget(":/textureFiles/target.png");

    // Convert the image format
    imgSrc = imgSrc.convertToFormat(QImage::Format_RGB888);
    imgTarget = imgTarget.convertToFormat(QImage::Format_RGB888);
    imgMask = imgMask.convertToFormat(QImage::Format_Indexed8);

    Mat matSrc(imgSrc.height(), imgSrc.width(), CV_8UC3, (uchar*)imgSrc.bits(), imgSrc.bytesPerLine());
    Mat matTarget(imgTarget.height(), imgTarget.width(), CV_8UC3, (uchar*)imgTarget.bits(), imgTarget.bytesPerLine());
    Mat matMask(imgMask.height(), imgMask.width(), CV_8UC1, (uchar*)imgMask.bits(), imgMask.bytesPerLine());

    //調整target影像的顏色與相機影像的histogram相似
    PoissonBlender pb = PoissonBlender(matSrc, matTarget, matMask);

    cv::Mat dst_img;
    pb.seamlessClone(dst_img, 314, 56, true);

    Rect region;
    // Blur the region of left eye
    tmpStartX = (m_featurePos[38] - m_faceRect.x) * s + 312;
    tmpEndX = (m_featurePos[44]- m_faceRect.x) * s + 312;
    tmpStartY = ((m_featurePos[45]+m_featurePos[49])/2.0 - m_faceRect.y) * s + 56 - 20;
    tmpEndY = ((m_featurePos[45]+m_featurePos[49])/2.0 - m_faceRect.y) * s + 56 + 20;
    region = Rect(tmpStartX, tmpStartY, tmpEndX-tmpStartX, tmpEndY-tmpStartY);
    blur(dst_img(region), dst_img(region), Size(7, 7));

    tmpStartX = (m_featurePos[44] - m_faceRect.x) * s + 312 - 20;
    tmpEndX = (m_featurePos[44]- m_faceRect.x) * s + 312 + 20;
    tmpStartY = ((m_featurePos[45]+m_featurePos[49])/2.0 - m_faceRect.y) * s + 56;;
    tmpEndY = ((m_featurePos[53]+m_featurePos[83])/2.0 - m_faceRect.y) * s + 56;
    region = Rect(tmpStartX, tmpStartY, tmpEndX-tmpStartX, tmpEndY-tmpStartY);
    blur(dst_img(region), dst_img(region), Size(3, 3));

    tmpStartX = (m_featurePos[38] - m_faceRect.x) * s + 312;
    tmpEndX = (m_featurePos[44]- m_faceRect.x) * s + 312;
    tmpStartY = ((m_featurePos[53]+m_featurePos[83])/2.0 - m_faceRect.y) * s + 56 - 20;
    tmpEndY = ((m_featurePos[53]+m_featurePos[83])/2.0 - m_faceRect.y) * s + 56 + 20;
    region = Rect(tmpStartX, tmpStartY, tmpEndX-tmpStartX, tmpEndY-tmpStartY);
    blur(dst_img(region), dst_img(region), Size(7, 7));

    tmpStartX = (m_featurePos[38] - m_faceRect.x) * s + 312 - 20;
    tmpEndX = (m_featurePos[38] - m_faceRect.x) * s + 312 + 20;
    tmpStartY = ((m_featurePos[45]+m_featurePos[49])/2.0 - m_faceRect.y) * s + 56;
    tmpEndY = ((m_featurePos[45]+m_featurePos[49])/2.0 - m_faceRect.y) * s + 56;
    region = Rect(tmpStartX, tmpStartY, tmpEndX-tmpStartX, tmpEndY-tmpStartY);
    blur(dst_img(region), dst_img(region), Size(3, 3));

    // Blur the region of right eye
    tmpStartX = (m_featurePos[36] - m_faceRect.x) * s + 312;
    tmpEndX = (m_featurePos[30]- m_faceRect.x) * s + 312;
    tmpStartY = ((m_featurePos[45]+m_featurePos[49])/2.0 - m_faceRect.y) * s + 56 - 20;
    tmpEndY = ((m_featurePos[45]+m_featurePos[49])/2.0 - m_faceRect.y) * s + 56 + 20;
    region = Rect(tmpStartX, tmpStartY, tmpEndX-tmpStartX, tmpEndY-tmpStartY);
    blur(dst_img(region), dst_img(region), Size(7, 7));

    tmpStartX = (m_featurePos[30] - m_faceRect.x) * s + 312 - 20;
    tmpEndX = (m_featurePos[30]- m_faceRect.x) * s + 312 + 20;
    tmpStartY = ((m_featurePos[45]+m_featurePos[49])/2.0 - m_faceRect.y) * s + 56;
    tmpEndY = ((m_featurePos[53]+m_featurePos[83])/2.0 - m_faceRect.y) * s + 56;
    region = Rect(tmpStartX, tmpStartY, tmpEndX-tmpStartX, tmpEndY-tmpStartY);
    blur(dst_img(region), dst_img(region), Size(3, 3));

    tmpStartX = (m_featurePos[36] - m_faceRect.x) * s + 312;
    tmpEndX = (m_featurePos[30]- m_faceRect.x) * s + 312;
    tmpStartY = ((m_featurePos[53]+m_featurePos[83])/2.0 - m_faceRect.y) * s + 56 - 20;
    tmpEndY = ((m_featurePos[53]+m_featurePos[83])/2.0 - m_faceRect.y) * s + 56 + 20;
    region = Rect(tmpStartX, tmpStartY, tmpEndX-tmpStartX, tmpEndY-tmpStartY);
    blur(dst_img(region), dst_img(region), Size(7, 7));

    tmpStartX = (m_featurePos[36] - m_faceRect.x) * s + 312 - 20;
    tmpEndX = (m_featurePos[36] - m_faceRect.x) * s + 312 + 20;
    tmpStartY = ((m_featurePos[45]+m_featurePos[49])/2.0 - m_faceRect.y) * s + 56;
    tmpEndY = ((m_featurePos[53]+m_featurePos[83])/2.0 - m_faceRect.y) * s + 56;
    region = Rect(tmpStartX, tmpStartY, tmpEndX-tmpStartX, tmpEndY-tmpStartY);
    blur(dst_img(region), dst_img(region), Size(3, 3));

    // Blur the region of nose
    tmpStartX = (m_featurePos[88] - m_faceRect.x) * s + 312 - 20;
    tmpEndX = (m_featurePos[88] - m_faceRect.x) * s + 312 + 20;
    tmpStartY = ((m_featurePos[83]+m_featurePos[69])/2.0 - m_faceRect.y) * s + 56;
    tmpEndY = ((m_featurePos[75]+m_featurePos[95])/2.0 - m_faceRect.y) * s + 56;
    region = Rect(tmpStartX, tmpStartY, tmpEndX-tmpStartX, tmpEndY-tmpStartY);
    blur(dst_img(region), dst_img(region), Size(7, 7));

    tmpStartX = (m_featurePos[100] - m_faceRect.x) * s + 312 - 20;
    tmpEndX = (m_featurePos[100] - m_faceRect.x) * s + 312 + 20;
    tmpStartY = ((m_featurePos[83]+m_featurePos[69])/2.0 - m_faceRect.y) * s + 56;
    tmpEndY = ((m_featurePos[75]+m_featurePos[95])/2.0 - m_faceRect.y) * s + 56;
    region = Rect(tmpStartX, tmpStartY, tmpEndX-tmpStartX, tmpEndY-tmpStartY);
    blur(dst_img(region), dst_img(region), Size(7, 7));


    QImage testImg = QImage(dst_img.data, dst_img.cols, dst_img.rows, dst_img.step, QImage::Format_RGB888).copy();
    qDebug() << testImg.save(m_appFileDirectory+"/result.png");
    tmpStartX = (m_featurePos[2] - m_faceRect.x) * s + 312;
    tmpEndX = (m_featurePos[26]- m_faceRect.x) * s + 312;
    tmpStartY = (m_featurePos[41] - m_faceRect.y) * s + 56;
    tmpEndY = (m_featurePos[15] - m_faceRect.y) * s + 56;
    QImage afterBlendingImg = testImg.copy(tmpStartX, tmpStartY, tmpEndX-tmpStartX, tmpEndY-tmpStartY);
    qDebug() << afterBlendingImg.save(m_appFileDirectory+"/afterBlending.png");
}

void Plugin::checkImage(QImage pSrc)
{

}

void Plugin::test_SetFeaturePos()
{
    double pos[] = {570.613, 1044.9,
                    566.996, 1154.17,
                    590.249, 1279.06,
                    625.843, 1393.81,
                    680.142, 1469.43,
                    748.102, 1538.9,
                    821.147, 1579.37,
                    917.913, 1612.73,
                    1016.34, 1601.73,
                    1091.38, 1545.47,
                    1150.72, 1479.13,
                    1211.3, 1391.4,
                    1239.44, 1287.25,
                    1260.19, 1167.59,
                    1242.62, 1051.67,
                    1207.16, 979.359,
                    1152.79, 953.607,
                    1071.06, 954.986,
                    1005.36, 967.787,
                    646.446, 986.576,
                    701.995, 958.71,
                    785.015, 958.844,
                    851.354, 970.407,
                    695.442, 1060.86,
                    762.016, 1030.47,
                    831.739, 1061.45,
                    761.925, 1078.05,
                    764.919, 1052.53,
                    1159.96, 1057.15,
                    1095.47, 1027.06,
                    1024.35, 1059.23,
                    1095.76, 1075.26,
                    1095.14, 1049.36,
                    926.643, 1035.63,
                    846.226, 1197.25,
                    816.189, 1250.32,
                    844.54, 1287.81,
                    923.205, 1301.29,
                    1002.7, 1287.33,
                    1030.73, 1250.9,
                    1000.92, 1197.25,
                    924.589, 1142.03,
                    867.101, 1267.19,
                    978.791, 1266.53,
                    781.328, 1413.28,
                    830.316, 1383.29,
                    883.892, 1371.98,
                    921.481, 1379.53,
                    958.83, 1371.89,
                    1011.89, 1382.76,
                    1060.45, 1412.51,
                    1024.72, 1447.3,
                    979.866, 1469.05,
                    919.928, 1476.21,
                    860.431, 1470.09,
                    814.871, 1448.01,
                    851.222, 1424.08,
                    920.017, 1430.29,
                    989.613, 1422.96,
                    990.293, 1402.93,
                    920.629, 1405.27,
                    852.352, 1403.29,
                    923.259, 1242.78,
                    723.668, 1039.43,
                    802.767, 1038.03,
                    798.503, 1071.92,
                    725.562, 1072.98,
                    1133.45, 1035.65,
                    1054.07, 1035.52,
                    1058.52, 1069.49,
                    1131.11, 1069.52};
//    double pos[] = {671.604, 1715.6,
//                    660.222, 1847.79,
//                    674.417, 1982.25,
//                    705.079, 2118.56,
//                    761.766, 2234.15,
//                    839.312, 2330.64,
//                    930.25, 2409.77,
//                    1042.96, 2436.36,
//                    1161.25, 2415.44,
//                    1263.49, 2341.6,
//                    1350.83, 2248.48,
//                    1415.48, 2134.09,
//                    1450.97, 1997.45,
//                    1469.54, 1863.34,
//                    1461.29, 1729.62,
//                    1378.17, 1662.6,
//                    1314.67, 1630.78,
//                    1218.46, 1634.25,
//                    1141.22, 1650.13,
//                    735.658, 1649.16,
//                    794.97, 1620.13,
//                    888.758, 1627.41,
//                    964.262, 1646.43,
//                    788.617, 1746.74,
//                    862.691, 1716.21,
//                    943.068, 1753.61,
//                    861.269, 1770.96,
//                    866.172, 1741.69,
//                    1323.5, 1757.75,
//                    1247.87, 1724.15,
//                    1166.64, 1758.54,
//                    1247.64, 1779.13,
//                    1245.45, 1749.65,
//                    1051.04, 1731.09,
//                    958.165, 1918.51,
//                    923.656, 1977.56,
//                    953.987, 2020.13,
//                    1042.09, 2043.19,
//                    1136.88, 2024.35,
//                    1170.11, 1982.12,
//                    1136, 1922,
//                    1045.28, 1858.43,
//                    977.25, 2002.16,
//                    1109.89, 2004.11,
//                    892.871, 2151.64,
//                    946.238, 2127.47,
//                    1002.05, 2118.96,
//                    1043.14, 2127.94,
//                    1085.08, 2120.75,
//                    1144.41, 2131.74,
//                    1203.5, 2158.63,
//                    1162.15, 2195.24,
//                    1109.72, 2219.08,
//                    1042.84, 2226.07,
//                    978.184, 2215.86,
//                    930.531, 2189.91,
//                    969.856, 2162.88,
//                    1043.99, 2173.41,
//                    1121.12, 2166.57,
//                    1120.26, 2156.57,
//                    1043.36, 2160.08,
//                    969.267, 2153.08,
//                    1040.39, 1980,
//                    819.461, 1724.46,
//                    909.144, 1726.21,
//                    903.87, 1765.06,
//                    820.563, 1762.95,
//                    1291.71, 1734.13,
//                    1201.15, 1732.27,
//                    1205.25, 1771.52,
//                    1289.4, 1772.78};
    //Initialize feature position
    m_featurePos.clear();
    for(int i=0; i<142; i++)
    {
        m_featurePos.push_back(pos[i]);
    }

    //Initalize the rect of face
    m_faceRect.x = 519;
    m_faceRect.y = 755;
    m_faceRect.width = 782;
    m_faceRect.height = 782;

    m_imagePath = "/mnt/sdcard/DCIM/IMG_00000001.jpg";
//    m_imagePath = "/mnt/sdcard/Download/data4.jpg";

    //Initialize image
    m_img = QImage(m_imagePath, "JPEG");
    if(m_img.width() > m_img.height())
    {
        QTransform transform;
        transform.rotate(-90);
        m_img = m_img.transformed(transform);
        m_img.save(m_appFileDirectory+"/oface.jpg");
    }

    m_img = m_img.convertToFormat(QImage::Format_RGB888);
}

void Plugin::meanFilter(Mat &pSrc, int pSX, int pSY, int pEX, int pEY)
{
    qDebug() << "Plugin::meanFilter()";
    Mat tmp = pSrc(Rect(pSX, pSY, pEX-pSX, pEY-pSY)).clone();
    blur(tmp, tmp, Size(3,3));

    QImage img = QImage(tmp.data, tmp.cols, tmp.rows, tmp.step, QImage::Format_RGB888).copy();
    qDebug() << img.save(m_appFileDirectory+"/tmp.png");
}

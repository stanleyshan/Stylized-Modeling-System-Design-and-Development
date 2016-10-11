#ifndef PLUGIN_H
#define PLUGIN_H

#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_io.h>
#include <iostream>
#include "dlib/image_processing/shape_predictor.h"


#include "qopenmeshobject.h"
#include "Params.h"
#include "Clm.h"
#include "PoissonBlender.h"

#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <QObject>
#include <QJsonObject>

using namespace cv;

class Plugin : public QObject
{
    Q_OBJECT

public:
    explicit Plugin(QObject *parent = 0);
    // 從影像找臉部特徵點
    Q_INVOKABLE void findFeatures( const QString& imagePath);
    // 取得特徵點
    Q_INVOKABLE QList<double> getFeaturePos();
    // 設定特徵點
    Q_INVOKABLE void setFeaturePos(int idx, double x, double y);
    // 設定人臉的偵測框
    Q_INVOKABLE void setFaceRect(int x, int y, int width, int height);
    // 存檔
    Q_INVOKABLE void saveFile(QString objName);
    // 切換至變形介面
    Q_INVOKABLE void switchToDeformWindow();
    //解析json檔案
    QJsonObject parseJsonFile(QString fileName);
    //產生poisson blending之後的貼圖
    void createFaceTexture(QImage pSrc);
    //檢查jpeg的orientation
    void checkImage(QImage pSrc);
    //test function
    Q_INVOKABLE void test_SetFeaturePos();
    void meanFilter(Mat &pSrc, int pSX, int pSY, int pEX, int pEY);

    static QString          m_appFileDirectory; // app的根目錄路徑
    static QString          m_imagePath;        // 影像的儲存路徑
    static QList<double>    m_featurePos;       // 臉部特徵點
    //由opencv追蹤出來的人臉框框
    static Rect m_faceRect;

private:
    //原圖
    QImage m_img, m_img_5, m_img_6, m_img_7, m_img_8, m_img_9, m_img_10;
    CascadeClassifier m_faceCascade;
    Clm *clm;
    Params *params;
    QJsonObject m_faceFilter, m_leftEyeFilter, m_rightEyeFilter, m_noseFilter;

    typedef dlib::scan_fhog_pyramid<dlib::pyramid_down<6> > image_scanner_type;
    dlib::object_detector<image_scanner_type> detector;
    dlib::shape_predictor landmark_detector;

};

#endif // PLUGIN_H

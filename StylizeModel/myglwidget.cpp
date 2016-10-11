/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtCore module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "myglwidget.h"
#include "mainwindow.h"

#include <QMouseEvent>
#include <QDebug>
#include <math.h>

MyGLWidget::MyGLWidget(QWidget *parent) :
    QOpenGLWidget(parent),
    texture(0),
    angularSpeed(0),
    currentStroke_idx(0),
    current_deform_type(-1),
    m_strokeDeform(0),
    rotateAngle(0)
{
    setAttribute(Qt::WA_AcceptTouchEvents);
    m_spline = new HSSSpline::HSpline2D;
}

MyGLWidget::~MyGLWidget()
{
    // Make sure the context is current when deleting the texture and the buffers.
    makeCurrent();
    delete texture;
    doneCurrent();
}

#ifdef Q_OS_WIN
void MyGLWidget::mousePressEvent(QMouseEvent *e)
{
    if(current_deform_type != MainWindow::deform_type)
        vec_stroke_data2D.clear();

    if(MainWindow::b_enableStroke && vec_stroke_data2D.length() == 0)
    {
        switch(MainWindow::deform_type)
        {
            /* eye */
            case 0:
                current_deform_type = 0;
                num_stroke = 2;
                break;
            /* ear */
            case 1:
                current_deform_type = 1;
                num_stroke = 1;
                break;
            /* mouth */
            case 2:
                current_deform_type = 2;
                num_stroke = 2;
                break;
            /* The front of nose */
            case 3:
                current_deform_type = 3;
                num_stroke = 1;
                break;
            /* The side of nose */
            case 4:
                current_deform_type = 4;
                num_stroke = 1;
                break;
            /* chin */
            case 5:
                current_deform_type = 5;
                num_stroke = 1;
                break;
            default:
                break;
        }
        vec_stroke_data2D = QVector<vector<QVector2D>>(num_stroke);
        test_data2D = QVector<vector<QVector2D>>(num_stroke);
    }
    else
    {
        // Save mouse press position
        mousePressPosition = QVector2D(e->localPos());
    }
}
#endif // Q_OS_WIN

#ifdef Q_OS_WIN
void MyGLWidget::mouseReleaseEvent(QMouseEvent *e)
{
    if(MainWindow::b_enableStroke)
    {
        currentStroke_idx++;
        if(currentStroke_idx == num_stroke)
        {
            /* Copy the data of user's stroke into the data structure of spline */
            for(int i=0; i<vec_stroke_data2D.length(); i++)
            {
                m_points().clear();
                for(int j=0; j<vec_stroke_data2D[i].size(); j++)
                {
                    m_points().push_back(HSSSpline::PathPoint<2>(vec_stroke_data2D[i][j].x() ,vec_stroke_data2D[i][j].y()));
                }
                /* Do spline */
                m_spline->AssignPoints(m_points);

                switch(MainWindow::deform_type)
                {
                    /* eye */
                    case 0:
                        qDebug() << "eye";
                        m_spline->FittingCurve(1.0/(mVec_origin_contour[i].size()-1), false);
                        /* Get the control points*/
                        m_CtrlPoints = m_spline->GetCtrlPoints();
                        /* Right eye */
                        vec_stroke_data3D.push_back(screen_to_world(m_CtrlPoints, mVec_origin_contour[i]));

                        vec_stroke_data3D.push_back(reverse(vec_stroke_data3D[2*i], QVector3D(-1, 1, 1)));
                        break;
                    /* ear */
                    case 1:
                        qDebug() << "ear";
                        m_spline->FittingCurve(1.0/(mVec_origin_contour[i].size()-1), false);
                        /* Get the control points*/
                        m_CtrlPoints = m_spline->GetCtrlPoints();
                        /* Right ear*/
                        vec_stroke_data3D.push_back(screen_to_world(m_CtrlPoints, mVec_origin_contour[i]));

                        /* Reverse the x value of right ear to get the left ear*/
                        vec_stroke_data3D.push_back(reverse(vec_stroke_data3D[0], QVector3D(-1, 1, 1)));
                        break;
                    /* mouth */
                    case 2:
                        qDebug() << "mouth";
                        m_spline->FittingCurve(1.0/(mVec_origin_contour[i].size()-1), false);
                        /* Get the control points*/
                        m_CtrlPoints = m_spline->GetCtrlPoints();
                        vec_stroke_data3D.push_back(screen_to_world(m_CtrlPoints, mVec_origin_contour[i]));
                        break;
                    /* The front of nose */
                    case 3:
                        qDebug() << "front nose";
                        m_spline->FittingCurve(1.0/(mVec_origin_contour[i].size()-1), false);
                        /* Get the control points*/
                        m_CtrlPoints = m_spline->GetCtrlPoints();
                        vec_stroke_data3D.push_back(screen_to_world(m_CtrlPoints, mVec_origin_contour[i]));
                        break;
                    /* The side of nose */
                    case 4:
                        qDebug() << "side nose";
                        m_spline->FittingCurve(1.0/(mVec_origin_contour[i].size()-1), false);
                        /* Get the control points*/
                        m_CtrlPoints = m_spline->GetCtrlPoints();
                        vec_stroke_data3D.push_back(screen_to_world(m_CtrlPoints, mVec_origin_contour[i]));
                        break;
                    /* chin */
                    case 5:
                        qDebug() << "chin";
                        m_spline->FittingCurve(1.0/(mVec_origin_contour[i].size()-1), false);
                        /* Get the control points*/
                        m_CtrlPoints = m_spline->GetCtrlPoints();
                        vec_stroke_data3D.push_back(screen_to_world(m_CtrlPoints, mVec_origin_contour[i]));
                        break;
                    default:
                        break;
                }
            }
            vec_stroke_data2D.clear();
            MainWindow::b_enableStroke = false;
            currentStroke_idx = 0;
            m_strokeDeform->main(mVec_origin_contour, vec_stroke_data3D);
            deform_mesh = m_strokeDeform->Get_mesh();
            MainWindow::Instance->openMeshObject->meshRenderer->TriMesh_to_Mesh(deform_mesh);
            MainWindow::Instance->openMeshObject->meshRenderer->initMeshGeometry();
            mVec_origin_contour.clear();
            update();
        }
    }
}
#endif // Q_OS_WIN

#ifdef Q_OS_WIN
void MyGLWidget::mouseMoveEvent(QMouseEvent *e)
{
    if(MainWindow::b_enableStroke)
    {
        vec_stroke_data2D[currentStroke_idx].push_back(QVector2D(e->localPos()));
    }
    else
    {
        QVector2D endP = QVector2D(e->localPos());

        rotateAngle += (endP.x() - mousePressPosition.x())/2;

        if(rotateAngle > 360)
            rotateAngle -= 360;

        mousePressPosition = endP;
    }
    update();
}
#endif // Q_OS_WIN

void MyGLWidget::onTouchBegin(const QList<QTouchPoint> &touchPoints)
{
    switch (touchPoints.count())
    {
        case 1:
        {
            if(current_deform_type != MainWindow::deform_type)
                vec_stroke_data2D.clear();

            if(MainWindow::b_enableStroke && vec_stroke_data2D.length() == 0)
            {
                switch(MainWindow::deform_type)
                {
                    /* eye */
                    case 0:
                        current_deform_type = 0;
                        num_stroke = 2;
                        break;
                    /* ear */
                    case 1:
                        qDebug() << "ear";
                        current_deform_type = 1;
                        num_stroke = 1;
                        break;
                    /* mouth */
                    case 2:
                        current_deform_type = 2;
                        num_stroke = 2;
                        break;
                    /* The front of nose */
                    case 3:
                        current_deform_type = 3;
                        num_stroke = 1;
                        break;
                    /* The side of nose */
                    case 4:
                        current_deform_type = 4;
                        num_stroke = 1;
                        break;
                    /* chin */
                    case 5:
                        current_deform_type = 5;
                        num_stroke = 1;
                        break;
                    default:
                        break;
                }
                vec_stroke_data2D = QVector<vector<QVector2D>>(num_stroke);
                test_data2D = QVector<vector<QVector2D>>(num_stroke);
            }
            break;
        }
        default:
            break;
    }
}
void MyGLWidget::onTouchUpdate(const QList<QTouchPoint> &touchPoints)
{
    /* 接觸點數量改變時的Update不使用，因為lastPos()並不一定是上ㄧ個frame的作標 */
    if ( preTouchCount != touchPoints.count() )
    {
        preTouchCount = touchPoints.count();
        return;
    }

    switch (touchPoints.count())
    {
        case 1:
        {
            if(MainWindow::b_enableStroke)
            {
                vec_stroke_data2D[currentStroke_idx].push_back(QVector2D(touchPoints[0].pos()));
            }
            else
            {
                rotateAngle += (touchPoints[0].pos().x() - touchPoints[0].lastPos().x())/2;

                if(rotateAngle > 360)
                    rotateAngle -= 360;
            }
            update();
            break;
        }
        case 2:
        {
            if(!MainWindow::b_deform)
            {
                qreal preLength = QVector2D(touchPoints[0].lastPos() - touchPoints[1].lastPos()).length();
                qreal nowLength = QVector2D(touchPoints[0].pos() - touchPoints[1].pos()).length();
                qreal deltaLength = nowLength - preLength;
                MainWindow::cameraZ += deltaLength/360.0f;
                if(MainWindow::cameraZ < -1.0f)
                    MainWindow::cameraZ = -1.0f;
                else if(MainWindow::cameraZ > 0.0f)
                    MainWindow::cameraZ = 0.0f;
                update();
            }
            break;
        }
        default:
            break;
    }
}

void MyGLWidget::onTouchEnd(const QList<QTouchPoint> &touchPoints)
{
    switch (touchPoints.count())
    {
        case 1:
        {
            if(MainWindow::b_enableStroke)
            {
                currentStroke_idx++;
                if(currentStroke_idx == num_stroke)
                {
                    /* Copy the data of user's stroke into the data structure of spline */
                    for(int i=0; i<vec_stroke_data2D.length(); i++)
                    {
                        m_points().clear();
                        for(int j=0; j<vec_stroke_data2D[i].size(); j++)
                        {
                            m_points().push_back(HSSSpline::PathPoint<2>(vec_stroke_data2D[i][j].x() ,vec_stroke_data2D[i][j].y()));
                        }
                        /* Do spline */
                        m_spline->AssignPoints(m_points);

                        switch(MainWindow::deform_type)
                        {
                            /* eye */
                            case 0:
                                qDebug() << "eye";
                                m_spline->FittingCurve(1.0/(mVec_origin_contour[i].size()-1), false);
                                /* Get the control points*/
                                m_CtrlPoints = m_spline->GetCtrlPoints();
                                /* Right eye */
                                vec_stroke_data3D.push_back(screen_to_world(m_CtrlPoints, mVec_origin_contour[i]));

                                vec_stroke_data3D.push_back(reverse(vec_stroke_data3D[2*i], QVector3D(-1, 1, 1)));
                                break;
                            /* ear */
                            case 1:
                                qDebug() << "ear";
                                m_spline->FittingCurve(1.0/(mVec_origin_contour[i].size()-1), false);
                                /* Get the control points*/
                                m_CtrlPoints = m_spline->GetCtrlPoints();
                                /* Right ear*/
                                vec_stroke_data3D.push_back(screen_to_world(m_CtrlPoints, mVec_origin_contour[i]));

                                /* Reverse the x value of right ear to get the left ear*/
                                vec_stroke_data3D.push_back(reverse(vec_stroke_data3D[0], QVector3D(-1, 1, 1)));
                                break;
                            /* mouth */
                            case 2:
                                qDebug() << "mouth";
                                m_spline->FittingCurve(1.0/(mVec_origin_contour[i].size()-1), false);
                                /* Get the control points*/
                                m_CtrlPoints = m_spline->GetCtrlPoints();
                                vec_stroke_data3D.push_back(screen_to_world(m_CtrlPoints, mVec_origin_contour[i]));
                                break;
                            /* The front of nose */
                            case 3:
                                qDebug() << "front nose";
                                m_spline->FittingCurve(1.0/(mVec_origin_contour[i].size()-1), false);
                                /* Get the control points*/
                                m_CtrlPoints = m_spline->GetCtrlPoints();
                                vec_stroke_data3D.push_back(screen_to_world(m_CtrlPoints, mVec_origin_contour[i]));
                                break;
                            /* The side of nose */
                            case 4:
                                qDebug() << "side nose";
                                m_spline->FittingCurve(1.0/(mVec_origin_contour[i].size()-1), false);
                                /* Get the control points*/
                                m_CtrlPoints = m_spline->GetCtrlPoints();
                                vec_stroke_data3D.push_back(screen_to_world(m_CtrlPoints, mVec_origin_contour[i]));
                                break;
                            /* chin */
                            case 5:
                                qDebug() << "chin";
                                m_spline->FittingCurve(1.0/(mVec_origin_contour[i].size()-1), false);
                                /* Get the control points*/
                                m_CtrlPoints = m_spline->GetCtrlPoints();
                                vec_stroke_data3D.push_back(screen_to_world(m_CtrlPoints, mVec_origin_contour[i]));
                                break;
                            default:
                                break;
                        }
                    }
                    vec_stroke_data2D.clear();
                    MainWindow::b_enableStroke = false;
                    currentStroke_idx = 0;
                    m_strokeDeform->main(mVec_origin_contour, vec_stroke_data3D);
                    deform_mesh = m_strokeDeform->Get_mesh();
                    MainWindow::Instance->openMeshObject->meshRenderer->TriMesh_to_Mesh(deform_mesh);
                    MainWindow::Instance->openMeshObject->meshRenderer->initMeshGeometry();
                    mVec_origin_contour.clear();
                    update();
                }
            }
            break;
        }
        default:
            break;
    }
}

bool MyGLWidget::event(QEvent *event)
{
    const QTouchEvent *touchEvent = (QTouchEvent*)event;
    switch(event->type())
    {
        case QEvent::TouchBegin: onTouchBegin(touchEvent->touchPoints()); break;
        case QEvent::TouchUpdate: onTouchUpdate(touchEvent->touchPoints()); break;
        case QEvent::TouchEnd: onTouchEnd(touchEvent->touchPoints()); break;
        case QEvent::TouchCancel: break;
        default:
            return QWidget::event(event);
    }
    /* return true表示接收了這個事件，要注意的是如果沒有接收TouchBegin事件，就不會接收到之後的事件(TouchUpdate...) */
    return true;
}

void MyGLWidget::initializeGL()
{
    initializeOpenGLFunctions();

    glClearColor(0, 0, 0, 1);

    initShaders();
    initTextures();

    mVec_eye_contour_idx = QVector<QVector<int>> {
                                                 //right eye up
                                                 QVector<int>{224, 223, 307, 222, 310, 221, 557, 220},
                                                 //left eye up
                                                 QVector<int>{1961, 1960, 2044, 1959, 2047, 1958, 2294, 1957},
                                                 //right eye down
                                                 QVector<int>{295, 290, 278, 279, 328, 280, 695, 219},
                                                 //left eye down
                                                 QVector<int>{2032, 2027, 2015, 1969, 2066, 2017, 2487, 1956}};

    mVec_mouth_contour_idx = QVector<QVector<int>> {
                                                   //mouth up
//                                                   QVector<int>{191, 694, 684, 269, 197, 339, 29, 2076, 1934, 2006, 2421, 2431, 1928},
                                                   QVector<int>{192, 617, 607, 232, 198, 269, 29, 2006, 1959, 1958, 1910, 1908, 3312},
                                                   //mouth down
//                                                   QVector<int>{355, 796, 193, 195, 196, 50, 1933, 1932, 1930, 2533, 2093}};
                                                   QVector<int>{284, 702, 194, 701, 700, 50, 1906, 1904, 1901, 1903, 3314}};

    mVec_front_nose_contour_idx = QVector<QVector<int>> {/*QVector<int>{741, 740, 606, 609, 131, 107, 751, 216, 797, 602, 218, 23, 2339, 2534, 1955, 268, 2005, 1953, 2488, 1844, 1868, 2346, 2343, 2477, 2478}*/
                                                            QVector<int>{647, 531, 534, 132, 235, 234, 216, 217, 231, 703, 527, 219, 23, 3408, 3413, 3410, 3409, 1966, 1965, 1954, 1953, 2010, 2222, 3446, 3470}};

    mVec_side_nose_contour_idx = QVector<QVector<int>> {/*QVector<int>{35, 5, 6, 36, 37, 38, 7, 8, 45, 9, 20, 21, 25, 22, 26, 23}*/
                                                            QVector<int>{35, 5, 36, 6, 37, 38, 7, 8, 45, 9, 20, 21, 25, 26, 22, 23}};

    mVec_chin_contour_idx = QVector<QVector<int>> {/*QVector<int>{190, 189, 122, 623, 627, 769, 621, 619, 634, 630, 109, 3, 1846, 2371, 2367, 2356, 2358, 2506, 2364, 2360, 1859, 1926, 1927}*/
                                                      QVector<int>{242, 1738, 540, 1728, 673, 105, 543, 558, 535, 106, 1, 1798, 1797, 2232, 2230, 3304, 3308, 3300, 3325, 3333, 3350}};

    mVec_ear_contour_idx = QVector<QVector<int>> {
                                                 //right ear
//                                                 QVector<int>{487, 395, 392, 391, 388, 387, 385, 516, 503, 383, 538, 381, 508, 380, 521, 377, 379, 399},
                                                    QVector<int>{416, 324, 321, 320, 317, 316, 314, 445, 432, 312, 467, 310, 437, 309, 450, 306, 308, 328},
                                                 //left ear
                                                 /*QVector<int>{2106, 2131, 2129, 2128, 2125, 2122, 2124, 2253, 2240, 2120, 2275, 2118, 2245, 2117, 2258, 2114, 2116, 2136}*/
                                                    QVector<int>{2156, 2064, 2066, 2063, 2062, 2059, 2061, 2176, 2058, 2056, 2167, 2166, 2196, 2186, 2055, 2054, 2052, 3286}};

    /* Initialize view matrix */
    eye = QVector3D(0.0f, 0.0f, 1.0f);
    center = QVector3D(0.0f, 0.0f, 0.0f);
    up = QVector3D(0.0, 1.0, 0.0f);

    v.setToIdentity();
    v.lookAt(eye, center, up);

    // Enable depth buffer
    glEnable(GL_DEPTH_TEST);

    // Enable back face culling
    glEnable(GL_CULL_FACE);

    MainWindow::Instance->initialData();
    m_strokeDeform = new StrokeDeform();
}

void MyGLWidget::initShaders()
{
    // Compile vertex shader
    if(!program.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaderFiles/vshader.glsl"))
        close();
    if(!stroke_on_screen_program.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaderFiles/stroke_on_screen.vsh"))
        close();
    if(!worldCoordinate_program.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaderFiles/strokeVS.glsl"))
        close();

    // Compile fragment shader
    if(!program.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaderFiles/fshader.glsl"))
        close();
    if(!stroke_on_screen_program.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaderFiles/stroke_on_screen.fsh"))
        close();
    if(!worldCoordinate_program.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaderFiles/strokeFS.glsl"))
        close();

    // Link shader pipeline
    if(!program.link())
        close();
    if(!stroke_on_screen_program.link())
        close();
    if(!worldCoordinate_program.link())
        close();
}

void MyGLWidget::initTextures()
{
    isSkinColor = false;
    // Load texture image
    texture = new QOpenGLTexture(QImage(Plugin::m_appFileDirectory+"/target.png").mirrored());

    // Set nearest filtering mode for texture minification
    texture->setMinificationFilter(QOpenGLTexture::Nearest);

    // Set bilinear filtering mode for texture magnification
    texture->setMagnificationFilter(QOpenGLTexture::Linear);

    // Wrap texture coordinates by repeating
    // f.ex. texture coordinate (1.1, 1.2) is same as (0.1, 0.2)
    texture->setWrapMode(QOpenGLTexture::ClampToEdge);
    m_textures.push_back(texture);

    // Face texture ( Make square )
    source_img = QImage(Plugin::m_imagePath).mirrored();
    if(source_img.width() > source_img.height())
    {
        QTransform transform;
        transform.rotate(-90);
        source_img = source_img.transformed(transform);
    }
    source_img.save(Plugin::m_appFileDirectory+"/faceMask.jpg");
    int imgSize;
    if(source_img.width()>source_img.height())
    {
        imgSize = source_img.width();
    }
    else
    {
        imgSize = source_img.height();
    }
    QImage square(imgSize, imgSize, QImage::Format_RGB32);
    for(int i = 0; i<source_img.width(); i++)
    {
        for(int j = 0; j<source_img.height(); j++)
        {
            QRgb value = source_img.pixel(i,j);
            square.setPixel(i, j, value);
        }
    }
    square.save(Plugin::m_appFileDirectory+"/square.jpg");
    FaceSquare = square;
    texture = new QOpenGLTexture(square);
    texture->setMinificationFilter(QOpenGLTexture::Nearest);
    texture->setMagnificationFilter(QOpenGLTexture::Linear);
    texture->setWrapMode(QOpenGLTexture::ClampToEdge);
    m_textures.push_back(texture);

    // Hat texture
    texture = new QOpenGLTexture(QImage(":/textureFiles/hat1.png").mirrored());
    texture->setMinificationFilter(QOpenGLTexture::Nearest);
    texture->setMagnificationFilter(QOpenGLTexture::Linear);
    texture->setWrapMode(QOpenGLTexture::ClampToEdge);
    m_textures.push_back(texture);

    // Hair texture
    texture = new QOpenGLTexture(QImage(":/textureFiles/hair1.png").mirrored());
    texture->setMinificationFilter(QOpenGLTexture::Nearest);
    texture->setMagnificationFilter(QOpenGLTexture::Linear);
    texture->setWrapMode(QOpenGLTexture::ClampToEdge);
    m_textures.push_back(texture);

    texture = new QOpenGLTexture(QImage(":/textureFiles/hair2.png").mirrored());
    texture->setMinificationFilter(QOpenGLTexture::Nearest);
    texture->setMagnificationFilter(QOpenGLTexture::Linear);
    texture->setWrapMode(QOpenGLTexture::ClampToEdge);
    m_textures.push_back(texture);

    // Cloth texture
    texture = new QOpenGLTexture(QImage(":/textureFiles/cloth1.png").mirrored());
    texture->setMinificationFilter(QOpenGLTexture::Nearest);
    texture->setMagnificationFilter(QOpenGLTexture::Linear);
    texture->setWrapMode(QOpenGLTexture::ClampToEdge);
    m_textures.push_back(texture);

    // Pants texture
    texture = new QOpenGLTexture(QImage(":/textureFiles/pants1.png").mirrored());
    texture->setMinificationFilter(QOpenGLTexture::Nearest);
    texture->setMagnificationFilter(QOpenGLTexture::Linear);
    texture->setWrapMode(QOpenGLTexture::ClampToEdge);
    m_textures.push_back(texture);

    // Shoes texture
    texture = new QOpenGLTexture(QImage(":/textureFiles/shoes1.png").mirrored());
    texture->setMinificationFilter(QOpenGLTexture::Nearest);
    texture->setMagnificationFilter(QOpenGLTexture::Linear);
    texture->setWrapMode(QOpenGLTexture::ClampToEdge);
    m_textures.push_back(texture);

    // Part texture
    texture = new QOpenGLTexture(QImage(":/textureFiles/boy_texture.png").mirrored());
    texture->setMinificationFilter(QOpenGLTexture::Nearest);
    texture->setMagnificationFilter(QOpenGLTexture::Linear);
    texture->setWrapMode(QOpenGLTexture::ClampToEdge);
    m_textures.push_back(texture);

    texture = new QOpenGLTexture(QImage(":/textureFiles/skateboard1.jpg").mirrored());
    texture->setMinificationFilter(QOpenGLTexture::Nearest);
    texture->setMagnificationFilter(QOpenGLTexture::Linear);
    texture->setWrapMode(QOpenGLTexture::ClampToEdge);
    m_textures.push_back(texture);

    // Ironman texture
    texture = new QOpenGLTexture(QImage(":/textureFiles/ironman_red.png").mirrored());
    texture->setMinificationFilter(QOpenGLTexture::Nearest);
    texture->setMagnificationFilter(QOpenGLTexture::Linear);
    texture->setWrapMode(QOpenGLTexture::ClampToEdge);
    m_textures.push_back(texture);

    texture = new QOpenGLTexture(QImage(":/textureFiles/ironman_yellow.png").mirrored());
    texture->setMinificationFilter(QOpenGLTexture::Nearest);
    texture->setMagnificationFilter(QOpenGLTexture::Linear);
    texture->setWrapMode(QOpenGLTexture::ClampToEdge);
    m_textures.push_back(texture);

    texture = new QOpenGLTexture(QImage(":/textureFiles/ironman_white.png").mirrored());
    texture->setMinificationFilter(QOpenGLTexture::Nearest);
    texture->setMagnificationFilter(QOpenGLTexture::Linear);
    texture->setWrapMode(QOpenGLTexture::ClampToEdge);
    m_textures.push_back(texture);

    texture = new QOpenGLTexture(QImage(":/textureFiles/ironman4.png").mirrored());
    texture->setMinificationFilter(QOpenGLTexture::Nearest);
    texture->setMagnificationFilter(QOpenGLTexture::Linear);
    texture->setWrapMode(QOpenGLTexture::ClampToEdge);
    m_textures.push_back(texture);

    // Luffy texture
    texture = new QOpenGLTexture(QImage(":/textureFiles/luffy.png").mirrored());
    texture->setMinificationFilter(QOpenGLTexture::Nearest);
    texture->setMagnificationFilter(QOpenGLTexture::Linear);
    texture->setWrapMode(QOpenGLTexture::ClampToEdge);
    m_textures.push_back(texture);

    // Minion texture
    texture = new QOpenGLTexture(QImage(":/textureFiles/minion1.png").mirrored());
    texture->setMinificationFilter(QOpenGLTexture::Nearest);
    texture->setMagnificationFilter(QOpenGLTexture::Linear);
    texture->setWrapMode(QOpenGLTexture::ClampToEdge);
    m_textures.push_back(texture);

    texture = new QOpenGLTexture(QImage(":/textureFiles/minion2.png").mirrored());
    texture->setMinificationFilter(QOpenGLTexture::Nearest);
    texture->setMagnificationFilter(QOpenGLTexture::Linear);
    texture->setWrapMode(QOpenGLTexture::ClampToEdge);
    m_textures.push_back(texture);

    texture = new QOpenGLTexture(QImage(":/textureFiles/minion3.png").mirrored());
    texture->setMinificationFilter(QOpenGLTexture::Nearest);
    texture->setMagnificationFilter(QOpenGLTexture::Linear);
    texture->setWrapMode(QOpenGLTexture::ClampToEdge);
    m_textures.push_back(texture);

    texture = new QOpenGLTexture(QImage(":/textureFiles/minion4.png").mirrored());
    texture->setMinificationFilter(QOpenGLTexture::Nearest);
    texture->setMagnificationFilter(QOpenGLTexture::Linear);
    texture->setWrapMode(QOpenGLTexture::ClampToEdge);
    m_textures.push_back(texture);

    // Chopper texture
    texture = new QOpenGLTexture(QImage(":/textureFiles/chopper.png").mirrored());
    texture->setMinificationFilter(QOpenGLTexture::Nearest);
    texture->setMagnificationFilter(QOpenGLTexture::Linear);
    texture->setWrapMode(QOpenGLTexture::ClampToEdge);
    m_textures.push_back(texture);
}

void MyGLWidget::resizeGL(int w, int h)
{
    // Calculate aspect ratio
    qreal aspect = qreal(w) / qreal(h ? h : 1);

    // Set near plane to 0.1, far plane to 10.0, field of view 45 degrees
    const qreal zNear = 0.8, zFar = 10.0, fov = 45.0;

    // Reset projection
    p.setToIdentity();

    // Set perspective projection
    p.perspective(fov, aspect, zNear, zFar);
}
//! [5]

void MyGLWidget::paintGL()
{
    // Clear color and depth buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // According to the deform type, change view and load contour data
    if(current_deform_type != MainWindow::deform_type)
    {
        vec_stroke_data3D.clear();
        QVector<QVector<QVector3D>> contour_data;
        switch(MainWindow::deform_type)
        {
            // Eyes
            case 0:
                contour_data = m_strokeDeform->VHandle_to_point(mVec_eye_contour_idx);
                rotateAngle = 0;
                current_deform_type = 0;
                break;
            // Ears
            case 1:
                contour_data = m_strokeDeform->VHandle_to_point(mVec_ear_contour_idx);
                rotateAngle = 90;
                current_deform_type = 1;
                break;
            // Mouth
            case 2:
                contour_data = m_strokeDeform->VHandle_to_point(mVec_mouth_contour_idx);
                rotateAngle = 0;
                current_deform_type = 2;
                break;
            // Front view of nose
            case 3:
                contour_data = m_strokeDeform->VHandle_to_point(mVec_front_nose_contour_idx);
                rotateAngle = 0;
                current_deform_type = 3;
                break;
            // Side view of nose
            case 4:
                contour_data = m_strokeDeform->VHandle_to_point(mVec_side_nose_contour_idx);
                rotateAngle = 90;
                current_deform_type = 4;
                break;
            // chin
            case 5:
                contour_data = m_strokeDeform->VHandle_to_point(mVec_chin_contour_idx);
                rotateAngle = 0;
                current_deform_type = 5;
                break;
            default:
                break;
        }

        mVec_origin_contour = QVector<vector<QVector3D>> (contour_data.length());

        for(int i=0; i<contour_data.length(); i++)
        {
            for(int j=0; j<contour_data[i].length(); j++)
            {
                mVec_origin_contour[i].push_back(contour_data[i][j]);
            }
        }
    }

    // Calculate model view transformation
    m.setToIdentity();
    m.translate(0.0, 0.2, MainWindow::cameraZ);
    m.rotate(rotateAngle, QVector3D(0, 1, 0));

    mvp = p*v*m;

    if(MainWindow::Instance != NULL)
    {
        if ( MainWindow::Instance->openMeshObject && MainWindow::Instance->openMeshObject->meshRenderer )
        {
            if(MainWindow::b_resetModel)
            {
                MainWindow::Instance->openMeshObject->resetRenderData();
                MainWindow::Instance->openMeshObject->meshRenderer->initMeshGeometry();
                MainWindow::b_resetModel = false;
            }

            if(MainWindow::b_reset)
            {
                reset();
                m_strokeDeform = new StrokeDeform();
                deform_mesh = m_strokeDeform->Get_mesh();
                MainWindow::Instance->openMeshObject->meshRenderer->TriMesh_to_Mesh(deform_mesh);
                MainWindow::Instance->openMeshObject->meshRenderer->initMeshGeometry();
                MainWindow::b_reset = false;
            }

            if(!isSkinColor)
            {
                isSkinColor = true;
                int imgSize = 0;
                if(source_img.width()>source_img.height())
                {
                    imgSize = source_img.width();
                }
                else
                {
                    imgSize = source_img.height();
                }
                // Face mask
                std::vector<BaseMesh::Point> FaceMask = MainWindow::Instance->openMeshObject->meshRenderer->Get_mask();
                for(size_t i = 0; i<FaceMask.size(); i++)
                {
                    FaceMask[i] *= imgSize;
                }
                FaceTex *facetex;
                facetex = new FaceTex();
                QColor color = QColor(135, 86, 56, 255);
                QRgb skinColor = qRgb(color.red(), color.green(), color.blue());
                m_textures[1] = facetex->Tex(FaceSquare, skinColor, FaceMask);
            }
            program.bind();
            // Set modelview-projection matrix
            program.setUniformValue("mvp_matrix", p * v * m);
            QMatrix3x3 normalMatrix = m.normalMatrix();
            program.setUniformValue("normal_matrix", normalMatrix);
            program.setUniformValue("light_position", QVector3D(0.0, 0.0, 10.0));
            // Use texture unit 0 which contains *.png (texture)
            program.setUniformValue("texture", 0);

            // Draw model geometry, including face texture
            // 0: body, 1: face
            for(int i=0; i<2; i++)
            {
                if(i==0)
                    program.setUniformValue("b_face", false);
                else
                    program.setUniformValue("b_face", false);
                m_textures[i]->bind();
                MainWindow::Instance->openMeshObject->meshRenderer->drawMeshGeometry(&program,i+1);
            }
            for(int i=2; i<MainWindow::Instance->openMeshObject->meshRenderer->num_model; i++)
            {
                program.setUniformValue("b_face", false);
                m_textures[MainWindow::Instance->openMeshObject->meshRenderer->mVec_texture_order[i-2]]->bind();
                MainWindow::Instance->openMeshObject->meshRenderer->drawMeshGeometry(&program,i+1);
            }
            program.release();
        }
    }

    for(int i=0; i<vec_stroke_data2D.length(); i++)
    {
        if(vec_stroke_data2D[i].size() > 2)
        {
            draw_stroke_on_screen(vec_stroke_data2D[i], 1, QVector3D(1.0, 0.0, 0.0));
            draw_stroke_on_screen(test_data2D[i], 0, QVector3D(1.0, 0.0, 0.0));
        }
    }

    // Draw the contour line
    for(int i=0; i<mVec_origin_contour.length(); i++)
    {
        draw_in_world_coordinata(mVec_origin_contour[i], 0, QVector3D(1.0, 0.0, 0.0));
        if(mVec_origin_contour[i].size() > 2)
            draw_in_world_coordinata(mVec_origin_contour[i], 1, QVector3D(1.0, 0.0, 0.0));
    }
}

/*
 * data  |  所要畫的資料
 * type  |  畫點(0),線(1)
 * color |  顏色
*/
void MyGLWidget::draw_in_world_coordinata(vector<QVector3D> data, int type, QVector3D color)
{
    worldCoordinate_program.bind();

    worldCoordinate_program.setUniformValue("mvp", mvp);
    worldCoordinate_program.setUniformValue("pointSize", 10.0f);
    worldCoordinate_program.setUniformValue("color", color);

    vbo.create();
    vbo.bind();
    vbo.allocate(data.data(), data.size()*sizeof(QVector3D));

    int positionLocation = worldCoordinate_program.attributeLocation("a_position");

    worldCoordinate_program.enableAttributeArray(positionLocation);
    worldCoordinate_program.setAttributeBuffer(positionLocation, GL_FLOAT, 0, 3, sizeof(QVector3D));

    if(type == 0)
    {
        glDrawArrays(GL_POINTS, 0, data.size());
    }
    else
    {
        glDrawArrays(GL_LINE_STRIP, 0, data.size());
    }

    worldCoordinate_program.disableAttributeArray(positionLocation);
    worldCoordinate_program.release();
}

/*
 * stoke_data   |   2D data of stroke on the screen
 * type         |   Draw points(0), Draw lines(1)
 * color        |   Color of points or lines
*/
void MyGLWidget::draw_stroke_on_screen(vector<QVector2D> stroke_data, int type, QVector3D color)
{
    glDisable(GL_DEPTH_TEST);
    stroke_on_screen_program.bind();

    stroke_on_screen_program.setUniformValue("color", color);
    stroke_on_screen_program.setUniformValue("pointSize", 10.0f);
    stroke_on_screen_program.setUniformValue("screen_halfWidth", (float)this->size().width()/2);
    stroke_on_screen_program.setUniformValue("screen_halfHeight", (float)this->size().height()/2);

    vbo.create();
    vbo.bind();
    vbo.allocate(stroke_data.data(), stroke_data.size() * sizeof(QVector2D));

    int positionLocation = stroke_on_screen_program.attributeLocation("a_position");

    stroke_on_screen_program.enableAttributeArray(positionLocation);
    stroke_on_screen_program.setAttributeBuffer(positionLocation, GL_FLOAT, 0, 2, sizeof(QVector2D));

    glLineWidth(5);

    if(type == 0)
    {
        glDrawArrays(GL_POINTS, 0, stroke_data.size());
    }
    else
    {
        glDrawArrays(GL_LINE_STRIP, 0, stroke_data.size());
    }
    stroke_on_screen_program.disableAttributeArray(positionLocation);
    vbo.release();
    stroke_on_screen_program.release();
    glEnable(GL_DEPTH_TEST);
}

vector<QVector3D> MyGLWidget::screen_to_world(HSSSpline::PathPoints<2> data2D, vector<QVector3D> origin_data)
{
    vector<QVector3D> result;
    switch(MainWindow::deform_type)
    {
        /* eye */
        case 0:
            for(int i=0; i<origin_data.size(); i++)
            {
                QVector4D tmp = mvp * QVector4D(origin_data[i].x(), origin_data[i].y(), origin_data[i].z(), 1.0);
                tmp /= tmp[3];

                float newX = ((float)data2D[i][0]/this->size().width()) * 2 - 1;
                float newY = ((this->size().height() - (float)data2D[i][1]) / this->size().height()) * 2 - 1;

                QVector4D repro_tmp = QVector4D(newX, newY, tmp[2], 1.0);
                repro_tmp = mvp.inverted() * repro_tmp;

                repro_tmp /= repro_tmp[3];
                repro_tmp[2] = origin_data[i].z();

                result.push_back(QVector3D(repro_tmp[0], repro_tmp[1], repro_tmp[2]));
            }
            break;
        /* ear */
        case 1:
            for(int i=0; i<origin_data.size(); i++)
            {
                QVector4D tmp = mvp * QVector4D(origin_data[i].x(), origin_data[i].y(), origin_data[i].z(), 1.0);
                tmp /= tmp[3];

                float newX = ((float)data2D[i][0]/this->size().width()) * 2 - 1;
                float newY = ((this->size().height() - (float)data2D[i][1]) / this->size().height()) * 2 - 1;

                QVector4D repro_tmp = QVector4D(newX, newY, tmp[2], 1.0);
                repro_tmp = mvp.inverted() * repro_tmp;

                repro_tmp /= repro_tmp[3];
                repro_tmp[0] = origin_data[i].x();

                result.push_back(QVector3D(repro_tmp[0], repro_tmp[1], repro_tmp[2]));
            }
            break;
        /* mouth */
        case 2:
            for(int i=0; i<origin_data.size(); i++)
            {
                QVector4D tmp = mvp * QVector4D(origin_data[i].x(), origin_data[i].y(), origin_data[i].z(), 1.0);
                tmp /= tmp[3];

                float newX = ((float)data2D[i][0]/this->size().width()) * 2 - 1;
                float newY = ((this->size().height() - (float)data2D[i][1]) / this->size().height()) * 2 - 1;

                QVector4D repro_tmp = QVector4D(newX, newY, tmp[2], 1.0);
                repro_tmp = mvp.inverted() * repro_tmp;

                repro_tmp /= repro_tmp[3];
                repro_tmp[2] = origin_data[i].z();

                result.push_back(QVector3D(repro_tmp[0], repro_tmp[1], repro_tmp[2]));
            }
            break;
        /* The front of nose */
        case 3:
            for(int i=0; i<origin_data.size(); i++)
            {
                QVector4D tmp = mvp * QVector4D(origin_data[i].x(), origin_data[i].y(), origin_data[i].z(), 1.0);
                tmp /= tmp[3];

                float newX = ((float)data2D[i][0]/this->size().width()) * 2 - 1;
                float newY = ((this->size().height() - (float)data2D[i][1]) / this->size().height()) * 2 - 1;

                QVector4D repro_tmp = QVector4D(newX, newY, tmp[2], 1.0);
                repro_tmp = mvp.inverted() * repro_tmp;

                repro_tmp /= repro_tmp[3];
                repro_tmp[2] = origin_data[i].z();

                result.push_back(QVector3D(repro_tmp[0], repro_tmp[1], repro_tmp[2]));
            }
            break;
        /* The side of nose */
        case 4:
            for(int i=0; i<origin_data.size(); i++)
            {
                QVector4D tmp = mvp * QVector4D(origin_data[i].x(), origin_data[i].y(), origin_data[i].z(), 1.0);
                tmp /= tmp[3];

                float newX = ((float)data2D[i][0]/this->size().width()) * 2 - 1;
                float newY = ((this->size().height() - (float)data2D[i][1]) / this->size().height()) * 2 - 1;

                QVector4D repro_tmp = QVector4D(newX, newY, tmp[2], 1.0);
                repro_tmp = mvp.inverted() * repro_tmp;

                repro_tmp /= repro_tmp[3];
                repro_tmp[0] = origin_data[i].x();

                result.push_back(QVector3D(repro_tmp[0], repro_tmp[1], repro_tmp[2]));
            }
            break;
        /* chin */
        case 5:
            for(int i=0; i<origin_data.size(); i++)
            {
                QVector4D tmp = mvp * QVector4D(origin_data[i].x(), origin_data[i].y(), origin_data[i].z(), 1.0);
                tmp /= tmp[3];

                float newX = ((float)data2D[i][0]/this->size().width()) * 2 - 1;
                float newY = ((this->size().height() - (float)data2D[i][1]) / this->size().height()) * 2 - 1;

                QVector4D repro_tmp = QVector4D(newX, newY, tmp[2], 1.0);
                repro_tmp = mvp.inverted() * repro_tmp;

                repro_tmp /= repro_tmp[3];
                repro_tmp[2] = origin_data[i].z();

                result.push_back(QVector3D(repro_tmp[0], repro_tmp[1], repro_tmp[2]));
            }
            break;
        default:
            break;
    }

    return result;
}

vector<QVector3D> MyGLWidget::reverse(vector<QVector3D> ctrlPoints, QVector3D reverse_vector)
{
    vector<QVector3D> result;
    for(int i=0; i<ctrlPoints.size(); i++)
    {
        result.push_back(ctrlPoints[i]*reverse_vector);
    }
    return result;
}

void MyGLWidget::reset()
{
    rotateAngle = 0;
    vec_stroke_data3D.clear();
    mVec_origin_contour.clear();
}

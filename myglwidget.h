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

#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include "meshrenderer.h"
#include "plugin.h"
#include "HSplinePath.h"
#include "strokedeform.h"
#include "mymesh.h"

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QMatrix4x4>
#include <QQuaternion>
#include <QVector2D>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QTouchEvent>

typedef QTouchEvent::TouchPoint QTouchPoint;
typedef QList<QTouchPoint> QTouchPoints;

class MyGLWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    explicit MyGLWidget(QWidget *parent = 0);
    ~MyGLWidget();

    void set_variable(int d, int m, float c, QString a);
    void reset();

protected:
#ifdef Q_OS_WIN
    void mousePressEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
#endif // Q_OS_WIN
    int preTouchCount;
    void onTouchBegin(const QList<QTouchPoint> &touchPoints);
    void onTouchUpdate(const QList<QTouchPoint> &touchPoints);
    void onTouchEnd(const QList<QTouchPoint> &touchPoints);
    bool event(QEvent *event);

    void initializeGL() Q_DECL_OVERRIDE;
    void resizeGL(int w, int h) Q_DECL_OVERRIDE;
    void paintGL() Q_DECL_OVERRIDE;

    void initShaders();
    void initTextures();

    /* Draw points or lines in the world coordinate */
    void draw_in_world_coordinata(vector<QVector3D> data, int type, QVector3D color);

    /*
    * stoke_data   |   2D data of stroke on the screen
    * type         |   Draw points(0), Draw lines(1)
    * color        |   Color of points or lines
   */
    void draw_stroke_on_screen(vector<QVector2D> stroke_data, int type, QVector3D color);
    /* Reproject 2D data to 3D */
    vector<QVector3D> screen_to_world(HSSSpline::PathPoints<2> data2D, vector<QVector3D> origin_data);

    /* Reverse the order of control */
    vector<QVector3D> reverse(vector<QVector3D> ctrlPoints, QVector3D reverse_vector);

private:
    /* The shader program of render 3D points or lines */
    QOpenGLShaderProgram program;
    /* The shader program of render 3D points or lines in the world coordinata */
    QOpenGLShaderProgram worldCoordinate_program;
    /* The shader program of render points or lines on the screen */
    QOpenGLShaderProgram stroke_on_screen_program;

    QImage source_img;
    bool isSkinColor;
    QOpenGLTexture *texture;
    std::vector<QOpenGLTexture*> m_textures;
    QImage FaceSquare;

    QMatrix4x4  m, v, p, mvp;
    QVector3D eye, center, up;

    QVector2D mousePressPosition;
    QVector3D rotationAxis;
    qreal angularSpeed;
    QQuaternion rotation;
    float rotateAngle;

    QOpenGLBuffer vbo;

    /* The 2D data of stroke drawn by user */
    vector<QVector2D> stroke_data2D;
    QVector<vector<QVector2D>> vec_stroke_data2D;
    QVector<vector<QVector2D>> test_data2D;
    QVector<vector<QVector3D>> vec_stroke_data3D;
    QVector<vector<QVector3D>> mVec_origin_contour;
    /* The number of strokes which can be drawn by user ex: chin: 1, eye: 2*/
    int num_stroke;
    QVector<int> vec_contour_points;
    /* Record the current stroke's index */
    int currentStroke_idx;
    /* Record the current deform type */
    int current_deform_type;

    QVector<QVector<QVector3D>> eye_data;
    QVector<QVector<QVector3D>> ear_data;
    QVector<QVector<QVector3D>> mouth_data;
    QVector<QVector<QVector3D>> front_nose_data;
    QVector<QVector<QVector3D>> side_nose_data;
    QVector<QVector<QVector3D>> chin_data;

    QVector<QVector<int>> mVec_eye_contour_idx;
    QVector<QVector<int>> mVec_ear_contour_idx;
    QVector<QVector<int>> mVec_mouth_contour_idx;
    QVector<QVector<int>> mVec_front_nose_contour_idx;
    QVector<QVector<int>> mVec_side_nose_contour_idx;
    QVector<QVector<int>> mVec_chin_contour_idx;


    /* The data of stroke in the data structure of spline */
    HSSSpline::PathPoints<2> m_points;
    HSSSpline::HSpline2D* m_spline;
    /* The control points */
    HSSSpline::PathPoints<2> m_CtrlPoints;

    StrokeDeform *m_strokeDeform;

    TriMesh* deform_mesh;
};

#endif // MAINWIDGET_H

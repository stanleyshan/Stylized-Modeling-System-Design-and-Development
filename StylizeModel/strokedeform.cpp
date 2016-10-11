#include <QOpenGLWidget>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <strokedeform.h>
#include <vector>
#include <list>

#include <QOpenGLFunctions>
#include <QMatrix4x4>
#include <QQuaternion>
#include <QBasicTimer>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QOpenGLTexture>

#include <QtMath>

#include <QFile>
#include <QTextStream>
#include <QObject>
#include <QDir>
#include <QDebug>

#include "HSplinePath.h"
#include "plugin.h"

StrokeDeform::StrokeDeform() :
    m_appFileDirectory(Plugin::m_appFileDirectory)
{
    // Load mesh
    QString fileName = m_appFileDirectory + "/modelFiles/deformModel.obj";
    mesh = new TriMesh(fileName.toStdString());
}

StrokeDeform::~StrokeDeform()
{
}

void StrokeDeform::main(QVector<vector<QVector3D>> model_contour, QVector<vector<QVector3D>> vec_stroke_data3D)
{
    /* 3D contour data on model (red line)*/
    m_model_contour = model_contour;
    /* 3D stroke data (blue line) */
    m_vec_stroke_data3D = vec_stroke_data3D;
    // Stroke deform
    buttom_press_v();
}

void StrokeDeform::buttom_press_v()
{
    /* According to the 3D contour data, find ROI */
    Read_Find_ROI();
    //計算ROI上面的頂點群以及擴充成6-ring的ROI
    mesh->compute_selvex(m_vec_model_contour_fh);
    //set origin 3D contour data
    mesh->set_model_contour(m_model_contour);
    //計算ROI上面靠近輪廓點的頂點群
    mesh->compute_vertex_on_ROI();
    //進行Biharmonic的初始化
    mesh->IGL_Biharmonic_init();

    mesh->create_vector(m_vec_stroke_data3D);

    mesh->IGL_Biharmonic_exe();
}

void StrokeDeform::Read_Find_ROI()
{
    m_vec_model_contour_fh = QVector<vector<TriMesh::FaceHandle>>(m_model_contour.length());

    for(int i=0; i<m_model_contour.length(); i++)
    {
        for(int j=0; j<m_model_contour[i].size(); j++)
        {
            Find_ROI(m_model_contour[i][j].x(), m_model_contour[i][j].y(), m_model_contour[i][j].z(), i);
        }
    }
}

void StrokeDeform::Find_ROI(float objx, float objy, float objz, int contour_idx)
{
    QVector3D click_p = QVector3D(objx,objy,objz);
    double error = 1.0f;

    select_fh = TriMesh::InvalidFaceHandle;

    /* Find the face handle which is closest to the input point */
    for(TriMesh::FIter f_it = mesh->faces_begin(); f_it != mesh->faces_end(); ++f_it)
    {
        QVector3D p[3];
        int i = 0;
        for(TriMesh::FVIter fv_it = mesh->fv_iter( *f_it ); fv_it; ++fv_it)
        {
            TriMesh::Point tem_p = mesh->point(fv_it);
            p[i++] = QVector3D(tem_p[0],tem_p[1],tem_p[2]);
        }
        QVector3D ab = p[1] - p[0], bc = p[2] - p[1], ca = p[0] - p[2];
        QVector3D norm = QVector3D::crossProduct(ab,-ca);
        norm.normalize();
        double t = QVector3D::dotProduct(norm, p[0] - click_p) / QVector3D::dotProduct(norm, norm);
        QVector3D project_p = click_p + t * norm;
        if( QVector3D::dotProduct(norm, QVector3D::crossProduct(ab, project_p - p[0])) >= 0 &&
                    QVector3D::dotProduct(norm, QVector3D::crossProduct(bc, project_p - p[1])) >= 0 &&
                    QVector3D::dotProduct(norm, QVector3D::crossProduct(ca, project_p - p[2])) >= 0 )
        {
            double distance = (project_p - click_p).length();
            if(distance < error)
            {
                error = distance;
                select_fh = f_it;
            }
        }
        else
        {
            continue;
        }
    }
    /* If select_fh is valid, put it into the correspoding m_vec_model_contour_fh[contour_idx] */
    if(select_fh!=TriMesh::InvalidVertexHandle)
    {
        vector<TriMesh::FaceHandle>::iterator iter = std::find(m_vec_model_contour_fh[contour_idx].begin(), m_vec_model_contour_fh[contour_idx].end(), select_fh);
        /*If face handel isn't in the list, add it*/
        if(iter == m_vec_model_contour_fh[contour_idx].end())
            m_vec_model_contour_fh[contour_idx].push_back(select_fh);
    }
}

TriMesh* StrokeDeform::Get_mesh()
{
    return mesh;
}

/*Input the index of handle, Output the point data of these handle*/
QVector<QVector<QVector3D>> StrokeDeform::VHandle_to_point(QVector<QVector<int>> vec_points_idx)
{
    QVector<QVector<QVector3D>> result;
    QVector<QVector3D> tmp;
    for(int i=0; i<vec_points_idx.length(); i++)
    {
        tmp.clear();
        for(int j=0; j<vec_points_idx[i].length(); j++)
        {
            VertexHandle vh(vec_points_idx[i][j]);
            BaseMesh::Point p = mesh->point(vh);
            tmp.push_back(QVector3D(p[0], p[1], p[2]));
        }
        result.push_back(tmp);
    }
    return result;
}

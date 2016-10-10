#ifndef STROKEDEFORM
#define STROKEDEFORM

#include "mymesh.h"

#include <vector>
#include <QtMath>
#include <QFile>
#include <QTextStream>

using namespace std;
using namespace OpenMesh;

class StrokeDeform
{
public:
    StrokeDeform();
    ~StrokeDeform();

    void main(QVector<vector<QVector3D>> model_contour, QVector<vector<QVector3D>> vec_stroke_data3D);
    void buttom_press_v();
    void Read_Find_ROI();
    void Find_ROI(float objx, float objy, float objz, int contour_idx);
    TriMesh* Get_mesh();
    QVector<QVector<QVector3D>> VHandle_to_point(QVector<QVector<int>> vec_points_idx);

    TriMesh* mesh;
    TriMesh::FaceHandle select_fh;

    QVector<float> jaw, ear_left, ear_right, eye_left_down, eye_left_up, eye_right_down, eye_right_up, mouth_down, mouth_up, nose_front_total, nose_side_total;
    vector< QVector<float> >files;
    int deform_mode;

    //計算執行次數
    int v_count;
    int shift_flag;

    // View mode (front or side)
    int view_mode;
    bool two_file;
    QVector<vector<QVector3D>> m_model_contour, m_vec_stroke_data3D;
    QVector<vector<TriMesh::FaceHandle>> m_vec_model_contour_fh;
    QString m_appFileDirectory;
};

#endif // STROKEDEFORM


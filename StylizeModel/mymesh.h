#ifndef MYMESH
#define MYMESH
#define _USE_MATH_DEFINES

#include "HSplinePath.h"

#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>
#include <string>
#include <QOpenGLFunctions>
#include <QDebug>
#include <QtMath>
#include <QVector3D>

#include <iostream>
#include <vector>
#include <list>
#include <queue>
#include <time.h>

#include <Eigen/Core>
#include <igl/colon.h>
#include <igl/harmonic.h>
#include <igl/readOBJ.h>
#include <igl/writeOBJ.h>
#include <igl/readDMAT.h>
#include <igl/barycenter.h>
#include <igl/cotmatrix.h>
#include <igl/massmatrix.h>
#include <igl/grad.h>
#include <igl/doublearea.h>
#include <igl/repdiag.h>
#include <igl/jet.h>
#include <igl/per_vertex_normals.h>

using namespace std;
using namespace OpenMesh;

struct MyTraits : DefaultTraits
{
    typedef Vec3f Point;
    typedef Vec3f Normal;
    typedef Vec2f TexCoord;
    typedef Vec3f Color;

    VertexAttributes(Attributes::Normal | Attributes::Color | Attributes::Status);
    HalfedgeAttributes(0);
    EdgeAttributes(Attributes::Status);
    FaceAttributes(Attributes::Normal | Attributes::Status);
};


struct Coordinate
{
   float x;
   float y;
   float z;
   float depth;
   float deg_x;
   float deg_y;
   float number;
};


struct spline_data_set{
    //畫線專用
    vector<float> m_cubic_spline;
    vector<float> m_ctrl_points;

    //控制點
    HSSSpline::PathPoints<2> control_point;

    //輪廓線資料
    QList<Coordinate> line;
    vector<Coordinate>  chin_line;

    //輪廓線+深度資訊
    vector<Coordinate> depth;
};

struct vertex_data{

    //ROI的頂點群資訊
    vector<VertexHandle> ser_vertex;

    //靠近輪廓線的頂點群
    vector<VertexHandle> real_vertex;

    //每個頂點的移動向量
    vector<Coordinate> create_vec;

    //ROI的資訊
    vector<FaceHandle> select_ROI;

    //輪廓線+深度資訊
    vector<Coordinate> depth_record;
};

class TriMesh : public TriMesh_ArrayKernelT<>, QOpenGLFunctions
{
public:
    vector<Coordinate> vertex_depth, create_vec, record_position, depth, depth_s, create_vec_s;
    vector<int> vertex_idx, vertex_idx_s;
    vector<float> m_deform_vertex, m_select_mfh, m_deform_vertex_s;
    std::vector<VertexHandle> real_vertex, sel_vertex, real_vertex_s, sel_vertex_s;
    std::vector<FaceHandle> select_mfh, select_mfh_one;

    std::vector<FaceHandle> select_ROI_jaw, select_ROI_Ear_left, select_ROI_Ear_right, select_ROI_Mouth_up, select_ROI_Mouth_down
                            , select_ROI_nose_front, select_ROI_nose, select_ROI_nose_side, select_ROI_eye_left_up, select_ROI_eye_left_down
                            , select_ROI_eye_right_up, select_ROI_eye_right_down;


    std::vector<VertexHandle> jaw_vertex, Ear_left_vertex, Ear_right_vertex, Mouth_up_vertex, Mouth_down_vertex,
                              nose_front_vertex, nose_side_vertex, eye_left_up, eye_left_down, eye_right_up,
                              eye_right_down;

    int mode;


    vector<Point> sil_point, sil_point_s;
    double Max_error;
    double Min_error;
    // libigl data
    // mesh vertices in 109
    Eigen::MatrixXd V, V_record;
    // mesh faces in 110
    Eigen::MatrixXi F;
    Eigen::MatrixXd U;
    Eigen::SparseMatrix<double> L;
    Eigen::VectorXd dblA;
    // biharmonic
    double bc_frac;
    double bc_dir;
    bool deformation_field;
    //Xd is double
    Eigen::MatrixXd V_bc,U_bc, U_bc_record;
    Eigen::VectorXd Z;
    //Xi is int
    Eigen::VectorXi b;
    Eigen::VectorXi S1;

    int all_clear;

    float const_number;
    int filestate;
    vector<vertex_data> vertex_data_set;

    /* origin 3D contour data on model */
    QVector<vector<QVector3D>> m_model_contour;

    TriMesh(string filename)
    {
        IO::Options opt;
        opt += IO::Options::VertexColor;
        IO::read_mesh(*this, filename, opt);

        float x_min = FLT_MAX, x_max = FLT_MIN, y_min = FLT_MAX, y_max = FLT_MIN, z_min = FLT_MAX, z_max = FLT_MIN;
        for(VertexIter v_it = vertices_begin(); v_it != vertices_end(); ++v_it) {
            Point p = point(*v_it);
            x_min = min(x_min, p[0]);
            x_max = max(x_max, p[0]);
            y_min = min(y_min, p[1]);
            y_max = max(y_max, p[1]);
            z_min = min(z_min, p[2]);
            z_max = max(z_max, p[2]);
        }
        OMT_to_VF();
        select_mfh.clear();
        select_mfh_one.clear();
        sil_point.clear();
    }

    ~TriMesh()
    {
    }

    void set_model_contour(QVector<vector<QVector3D>> model_contour)
    {
        m_model_contour = model_contour;
    }

    void compute_selvex(QVector<vector<TriMesh::FaceHandle>> vec_model_contour_fh)
    {
        vector<FaceHandle> expand_sel_faces ;
        vertex_data_set.clear();
        for(int i=0; i<vec_model_contour_fh.length(); i++)
        {
            vertex_data v_data;
            vertex_data_set.push_back(v_data);
            vertex_data_set[i].ser_vertex = FacesToVertex(vec_model_contour_fh[i]);
            expand_sel_faces = vec_model_contour_fh[i];
            for(int i=0; i<5; i++)
                expand_sel_faces = OneRingExpandFaces(expand_sel_faces);
            vertex_data_set[i].select_ROI = expand_sel_faces;
        }
    }


    void compute_vertex_on_ROI()
    {
        for(size_t i = 0 ; i < vertex_data_set.size() ; i++ )
        {
            real_vertex.clear();
            vertex_data v = vertex_data_set[i];
            vector<VertexHandle> s = v.ser_vertex;

            /* 3D data on contour */
            for(size_t j=0; j<m_model_contour[i].size(); j++)
            {
                double minDist = 99999999;
                VertexHandle minVH;
                /* Find the closest vertex handle in s */
                for(size_t k=0; k<s.size(); k++)
                {
                    QVector3D contour_point = m_model_contour[i][j];
                    Point p = point(s[k]);
                    double dist = qPow((contour_point.x() - p[0])*1000, 2) + qPow((contour_point.y()- p[1])*1000, 2) + qPow((contour_point.z() - p[2])*1000, 2);
                    if(dist < minDist)
                    {
                        minDist = dist;
                        minVH = s[k];
                    }
                }
                real_vertex.push_back(minVH);
            }
            vertex_data_set[i].real_vertex = real_vertex;
        }
     }

    vector<float> get(){
         return m_deform_vertex_s;
    }

    void OMT_to_VF()
    {
        int cnt=0;
        V.resize(n_vertices(),3);
        F.resize(n_faces(),3);
        for(VIter vit = vertices_begin(); vit!=vertices_end(); ++vit)
        {
            Point vit_point = point(vit.handle());
            V(cnt,0) = vit_point[0];
            V(cnt,1) = vit_point[1];
            V(cnt,2) = vit_point[2];
            cnt++;
        }
        cnt = 0;
        for(FIter f_it = faces_begin(); f_it != faces_end(); ++f_it)
        {

            int i = 0;
            for(FVIter fv_it = fv_iter(f_it); i < 3; i++, ++fv_it)
            {
                F(cnt,i)=fv_it.handle().idx();
            }
            cnt++;
        }
    }

    void VF_to_OMT()
    {
        int i;
        Point p;
        i=0;
        for(VIter vit = vertices_begin(); vit!=vertices_end(); ++vit)
        {
            p[0] = V(i,0);
            p[1] = V(i,1);
            p[2] = V(i,2);
            set_point(vit,p);
            i++;
        }
        update_normals();
    }
    // press C do clear_select
    void clear_select()
    {
        select_mfh.clear();
        select_mfh_one.clear();
        sil_point.clear();
        sil_point_s.clear();
        vertex_idx.clear();
        vertex_idx_s.clear();
        real_vertex.clear();
        real_vertex_s.clear();
        filestate = 0;
        m_deform_vertex_s.clear();
        m_deform_vertex.clear();
        vertex_data_set.clear();
    }
    vector<VertexHandle> FacesToVertex(vector<FaceHandle> faces)
    {
        vector<VertexHandle> vertex;
        for(vector<FaceHandle>::iterator f = faces.begin(); f != faces.end(); f++)
        {
            HalfedgeHandle he_start = halfedge_handle(*f);
            HalfedgeHandle he = he_start;
            int i = 0;
            do
            {
                vector<VertexHandle>::iterator iter = find(vertex.begin(),vertex.end(),from_vertex_handle(he));
                if(iter == vertex.end())
                {
                    vertex.push_back(from_vertex_handle(he));
                }
                he = next_halfedge_handle(he);
                i++;
            } while(i < 10 && he != he_start);
        }
        return vertex;
    }
    vector<FaceHandle> OneRingExpandFaces(const vector<FaceHandle>& faces)
    {
        vector<FaceHandle> result;
        result = faces;
        for(unsigned int j = 0;j < faces.size(); j++)
        {
            if(is_boundary(faces[j])) {
                continue;
            }
            //halfedge_handle(): Get a halfedge belonging to the face.
            HalfedgeHandle he_start = halfedge_handle(faces[j]);
            HalfedgeHandle he = he_start;

            int i = 0;

            do {
                vector<FaceHandle>::iterator iter = find(result.begin(),result.end(),face_handle(opposite_halfedge_handle(he)));
                if(iter == result.end()) {
                    //face_handle(): Get the i'th item.
                    //opposite_halfedge_handle(): Get the opposite halfedge.
                    result.push_back(face_handle(opposite_halfedge_handle(he)));
                }

               // cout << "push mesh count: "  << i << endl;

                i++;

                he = next_halfedge_handle(he);
            } while(he != he_start);
        }
        return result;
    }
    void OneRingExpand()
    {
        select_mfh = OneRingExpandFaces(select_mfh);
    }
    void saveObj(){
        OMT_to_VF();
        igl::writeOBJ("out.obj",V,F);
    }

    void clearmesh(){
        U_bc = U_bc_record;
        V = V_record ;
        all_clear = 1;

        for(size_t i = 0 ; i < vertex_data_set.size() ; i++){
            vector<Coordinate> create_vec;
            vertex_data_set[i].create_vec = create_vec;
        }
    }

    void set_all_clear(){
        all_clear = 0;
    }

    void set_clear(){
        all_clear = 1;
    }

    void set_mode_const(int m, float c){
        mode = m;
        const_number = c;
    }

    void IGL_Laplacian_init()
    {
        // Compute Laplace-Beltrami operator: #V by #V
        igl::cotmatrix(V,F,L);

        // Diagonal per-triangle "mass matrix"
        igl::doublearea(V,F,dblA);


        U = V;
    }

    void IGL_Laplacian_exe()
    {
        // Recompute just mass matrix on each step
        Eigen::SparseMatrix<double> M;
        igl::massmatrix(U,F,igl::MASSMATRIX_TYPE_BARYCENTRIC,M);
        // Solve (M-delta*L) U = M*U
        const auto & S = (M - 0.01*L);
        Eigen::SimplicialLLT<Eigen::SparseMatrix<double > > solver(S);
        assert(solver.info() == Eigen::Success);
        U = solver.solve(M*U).eval();
        // Compute centroid and subtract (also important for numerics)
        Eigen::VectorXd dblA;
        igl::doublearea(U,F,dblA);
        double area = 0.5*dblA.sum();
        Eigen::MatrixXd BC;
        igl::barycenter(U,F,BC);
        Eigen::RowVector3d centroid(0,0,0);
        for(int i = 0;i<BC.rows();i++)
        {
            centroid += 0.5*dblA(i)/area*BC.row(i);
        }
        U.rowwise() -= centroid;
        // Normalize to unit surface area (important for numerics)
        U.array() /= sqrt(area);

        V = U;
        VF_to_OMT();
    }
    double sel_xMin, sel_xMax, sel_yMin, sel_yMax, sel_zMin, sel_zMax;
    double x_cen,y_cen,z_cen;

    void set_depth(vector<Coordinate> depth_record)
    {
        vertex_data v;
        v.depth_record = depth_record;
        vertex_data_set.push_back(v);
    }

    //press V do init
    void IGL_Biharmonic_init()
    {
        qDebug() << "IGL_Biharmonic_init" << endl;
        all_clear = 0;
        sel_xMin = DBL_MAX;
        sel_xMax = DBL_MIN;
        sel_yMin = DBL_MAX;
        sel_yMax = DBL_MIN;
        sel_zMin = DBL_MAX;
        sel_zMax = DBL_MIN;
        U = V;
        V_record = V;
        // S(i) = j: j<0 (vertex i not in handle), j >= 0 (vertex i in handle j)
        // S(V.rows()) means row:V.rows() col: 1
        Eigen::VectorXi S(V.rows());
        for(int i = 0;i<S.rows();i++)
            S(i) = 0;


        //這邊是對模型做初始化,該變形的點就設為1,不該變形就設為-1
        for(size_t i = 0 ; i < vertex_data_set.size() ; i++)
        {
            vertex_data v = vertex_data_set[i];
            vector<VertexHandle> exp_vertex = FacesToVertex(v.select_ROI);
            vector<VertexHandle> r = v.real_vertex;
            vector<VertexHandle> s = v.ser_vertex;

            for(size_t i = 0;i<exp_vertex.size();i++)
            {
                if(S(exp_vertex[i].idx())!=1)
                    S(exp_vertex[i].idx()) = -1;
            }

            if(r.size() > 1)
            {
                for(size_t i = 0;i<r.size();i++)
                {
                     S(r[i].idx()) = 1;
                }
            }
            else
            {
                for(size_t i = 0;i<s.size();i++)
                {
                     S(s[i].idx()) = 1;
                }
            }
        }
        igl::colon<int>(0,V.rows()-1,b);
        b.conservativeResize(stable_partition( b.data(), b.data()+b.size(),
        [&S](int i)->bool{return S(i)>=0;})-b.data());
        // Boundary conditions directly on deformed positions
        // U_bc and V_bc use b.size() as row, V.cols() as columns
        U_bc.resize(b.size(),V.cols());
        V_bc.resize(b.size(),V.cols());
        S1 = S;
        U_bc_record = U_bc;
    }

    void create_vector(QVector<vector<QVector3D>> vec_stroke_data3D)
    {
        for(int i=0; i<vec_stroke_data3D.length(); i++)
        {
            vertex_data v = vertex_data_set[i];
            vector<VertexHandle> r = v.real_vertex;
            create_vec.clear();
            for(size_t j=0; j<vec_stroke_data3D[i].size(); j++)
            {
                Coordinate vec;
                Point p = point(r[j]);
                //計算在mesh上面的頂點需要移動到使用者曲線的ctrl points的移動向量
                vec.x = vec_stroke_data3D[i][j].x() - p[0];
                vec.y = vec_stroke_data3D[i][j].y() - p[1];
                vec.z = vec_stroke_data3D[i][j].z() - p[2];
                create_vec.push_back(vec);
            }
            vertex_data_set[i].create_vec = create_vec;
        }
    }

    //Deform的部分
    void IGL_Biharmonic_exe()
    {
        using namespace Eigen;
        for(int bi = 0;bi<b.size();bi++)
        {
            Coordinate v;
            if(all_clear == 1)
            {
                v.x = 0;
                v.y = 0;
                v.z = 0;
            }
            else if(vertex_data_set.size() > 0)
            {
                v.x = 0;
                v.y = -0.1;
                v.z = 0;
                //將模型上的點,與已經存好的位移像量與變型頂點做匹配,如果符合變型頂點的條件，則賦予位移向量
                for(size_t i = 0 ; i < vertex_data_set.size() ; i++)
                {
                    vertex_data vertex = vertex_data_set[i];
                    vector<VertexHandle> r = vertex.real_vertex;
                    vector<Coordinate> c = vertex.create_vec;

                    for(size_t i = 0 ; i < r.size() ; i++)
                    {
                        //找到要位移的頂點(藉由編號比對),給予位移向量
                        if(b(bi) == r[i].idx())
                        {
                            v = c[i];
                            break;
                        }
                    }
                }
            }
            else
            {
                v.x = 0;
                v.y = 0.0;
                v.z = 0;
            }

            V_bc.row(bi) = V.row(b(bi));
            switch(S1(b(bi)))
            {
                case 0:
                    // Don't move handle 0 constrain
                    U_bc.row(bi) = V.row(b(bi));
                    break;
                case 1:
                    U_bc.row(bi) = V.row(b(bi)) + Eigen::RowVector3d(v.x,v.y,v.z);
                    break;
                // do nothing
                default:
                    break;
            }
        }
        const MatrixXd U_bc_anim = U_bc;
        MatrixXd D;
        MatrixXd D_bc = U_bc_anim - V_bc;
        igl::harmonic(V,F,b,D_bc,2,D);
        V = V+D;
        VF_to_OMT(); //V to OMT temp
    }

    typedef struct str_P{
        double h;
        Point x;
        Normal n;
        double dot;
    } S_P;
    struct myclass {
      bool operator() (S_P i,S_P j) { return (i.h<j.h);}
    } myobject;

};

#endif // MYMESH


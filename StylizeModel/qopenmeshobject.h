#ifndef QOPENMESHOBJECT_H
#define QOPENMESHOBJECT_H

#include "meshrenderer.h"
#include "expmap.h"
#include "plugin.h"
#include "facetex.h"

#include <QString>
#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <unsupported/Eigen/SparseExtra>

class OpenMeshObject
{
public:
    void LoadMesh( QString objFilePath, bool isRender);
//    void loadObject( QString objFilePath );
    MeshData* loadMesh(QString objFilePath);
    void SaveMesh( BaseMesh mesh );
    void resetRenderData();
//    void initMeshRender();
    OpenMeshObject( QString objFilePath );
    ~OpenMeshObject();

    void PickVertex (QVector3D worldPos);
    BaseMesh MoveVector(BaseMesh mesh, double m_x, double m_y, double m_z);
    void Update_control_points();

    void InitData();
    void InitFHidx();
    void InitMatrix();
    void CalculateMoveFeatures();
    void Deform();

    template <typename DerivedV, typename DerivedF, typename Derivedb, typename Derivedbc, typename DerivedW>
    bool harmonic(const Eigen::PlainObjectBase<DerivedV> & V, const Eigen::PlainObjectBase<DerivedF> & F, const Eigen::PlainObjectBase<Derivedb> & b,
        const Eigen::PlainObjectBase<Derivedbc> & bc, const int k, Eigen::PlainObjectBase<DerivedW> & W);

    template <typename DerivedV, typename DerivedF, typename ScalarS>
    void cotmatrix(const Eigen::PlainObjectBase<DerivedV> & V, const Eigen::PlainObjectBase<DerivedF> & F, Eigen::SparseMatrix<ScalarS>& L);

    template <typename DerivedV, typename DerivedF, typename DerivedC>
    void cotmatrix_entries(const Eigen::PlainObjectBase<DerivedV>& V, const Eigen::PlainObjectBase<DerivedF>& F, Eigen::PlainObjectBase<DerivedC>& C);

    template <typename DerivedV, typename DerivedF, typename DerivedL>
    void edge_lengths(const Eigen::PlainObjectBase<DerivedV>& V,const Eigen::PlainObjectBase<DerivedF>& F, Eigen::PlainObjectBase<DerivedL>& L);

    template <typename Derivedl, typename DeriveddblA>
    void doublearea(const Eigen::PlainObjectBase<Derivedl> & ul, Eigen::PlainObjectBase<DeriveddblA> & dblA);

    template <typename DerivedX, typename DerivedIX>
    void sort(const Eigen::PlainObjectBase<DerivedX>& X, const int dim, const bool ascending, Eigen::PlainObjectBase<DerivedX>& Y, Eigen::PlainObjectBase<DerivedIX>& IX);

    template <class T>
    void sort(const std::vector<T> & unsorted, const bool ascending, std::vector<T> & sorted, std::vector<size_t> & index_map);

    // For use with functions like std::sort
    template<class T> struct IndexLessThan
    {
        IndexLessThan(const T arr) : arr(arr) {}
        bool operator()(const size_t a, const size_t b) const
        {
            return arr[a] < arr[b];
        }
        const T arr;
    };

    template< class T >
    void reorder(const std::vector<T> & unordered, std::vector<size_t> const & index_map, std::vector<T> & ordered);

    template <typename DerivedV, typename DerivedF, typename ScalarS>
    void massmatrix(const Eigen::MatrixBase<DerivedV> & V, const Eigen::MatrixBase<DerivedF> & F, Eigen::SparseMatrix<ScalarS>& M);

    template <typename DerivedA, typename DerivedB>
    void normalize_row_sums(const Eigen::MatrixBase<DerivedA>& A, Eigen::MatrixBase<DerivedB> & B);

    template <class IndexVector, class ValueVector, typename T>
    void sparse(const IndexVector & I, const IndexVector & J, const ValueVector & V, const size_t m, const size_t n, Eigen::SparseMatrix<T>& X);

    template <typename T>
    void invert_diag(const Eigen::SparseMatrix<T>& X, Eigen::SparseMatrix<T>& Y);

    template <typename T>
    struct min_quad_with_fixed_data
    {
        // Size of original system: number of unknowns + number of knowns
        int n;
        // Whether A(unknown,unknown) is positive definite
        bool Auu_pd;
        // Whether A(unknown,unknown) is symmetric
        bool Auu_sym;
        // Indices of known variables
        Eigen::VectorXi known;
        // Indices of unknown variables
        Eigen::VectorXi unknown;
        // Indices of lagrange variables
        Eigen::VectorXi lagrange;
        // Indices of unknown variable followed by Indices of lagrange variables
        Eigen::VectorXi unknown_lagrange;
        // Matrix multiplied against Y when constructing right hand side
        Eigen::SparseMatrix<T> preY;
        enum SolverType
        {
            LLT = 0,
            LDLT = 1,
            LU = 2,
            QR_LLT = 3,
            NUM_SOLVER_TYPES = 4
        } solver_type;
        // Solvers
        Eigen::SimplicialLLT <Eigen::SparseMatrix<T > > llt;
        Eigen::SimplicialLDLT<Eigen::SparseMatrix<T > > ldlt;
        Eigen::SparseLU<Eigen::SparseMatrix<T, Eigen::ColMajor>, Eigen::COLAMDOrdering<int> >   lu;
        // QR factorization
        // Are rows of Aeq linearly independent?
        bool Aeq_li;
        // Columns of Aeq corresponding to unknowns
        int neq;
        Eigen::SparseQR<Eigen::SparseMatrix<T>, Eigen::COLAMDOrdering<int> >  AeqTQR;
        Eigen::SparseMatrix<T> Aeqk;
        Eigen::SparseMatrix<T> Aequ;
        Eigen::SparseMatrix<T> Auu;
        Eigen::SparseMatrix<T> AeqTQ1;
        Eigen::SparseMatrix<T> AeqTQ1T;
        Eigen::SparseMatrix<T> AeqTQ2;
        Eigen::SparseMatrix<T> AeqTQ2T;
        Eigen::SparseMatrix<T> AeqTR1;
        Eigen::SparseMatrix<T> AeqTR1T;
        Eigen::SparseMatrix<T> AeqTE;
        Eigen::SparseMatrix<T> AeqTET;
        // Debug
        Eigen::SparseMatrix<T> NA;
        Eigen::Matrix<T,Eigen::Dynamic,Eigen::Dynamic> NB;
    };

    template <typename T, typename Derivedknown>
    bool min_quad_with_fixed_precompute(const Eigen::SparseMatrix<T>& A2, const Eigen::PlainObjectBase<Derivedknown> & known, const Eigen::SparseMatrix<T>& Aeq,
        const bool pd, min_quad_with_fixed_data<T> & data);

    template <typename T, typename DerivedB, typename DerivedY, typename DerivedBeq, typename DerivedZ, typename Derivedsol>
    bool min_quad_with_fixed_solve(const min_quad_with_fixed_data<T> & data, const Eigen::PlainObjectBase<DerivedB> & B, const Eigen::PlainObjectBase<DerivedY> & Y,
        const Eigen::PlainObjectBase<DerivedBeq> & Beq, Eigen::PlainObjectBase<DerivedZ> & Z, Eigen::PlainObjectBase<Derivedsol> & sol);

    template <typename T, typename DerivedB, typename DerivedY, typename DerivedBeq, typename DerivedZ>
    bool min_quad_with_fixed_solve(const min_quad_with_fixed_data<T> & data, const Eigen::PlainObjectBase<DerivedB> & B, const Eigen::PlainObjectBase<DerivedY> & Y,
        const Eigen::PlainObjectBase<DerivedBeq> & Beq, Eigen::PlainObjectBase<DerivedZ> & Z);

    template <typename T>
    void slice(const Eigen::SparseMatrix<T>& X, const Eigen::Matrix<int,Eigen::Dynamic,1> & R, const Eigen::Matrix<int,Eigen::Dynamic,1> & C, Eigen::SparseMatrix<T>& Y);

    template <typename DerivedX>
    void slice(const Eigen::PlainObjectBase<DerivedX> & X, const Eigen::Matrix<int,Eigen::Dynamic,1> & R, const Eigen::Matrix<int,Eigen::Dynamic,1> & C,
        Eigen::PlainObjectBase<DerivedX> & Y);

    template <typename DerivedX>
    void slice(const Eigen::PlainObjectBase<DerivedX> & X, const Eigen::Matrix<int,Eigen::Dynamic,1> & R, Eigen::PlainObjectBase<DerivedX> & Y);

    template <typename ScalarS>
    void cat(const int dim, const Eigen::SparseMatrix<ScalarS> & A, const Eigen::SparseMatrix<ScalarS> & B, Eigen::SparseMatrix<ScalarS> & C);

    template <class Mat>
    Mat cat(const int dim, const Mat & A, const Mat & B);

    template <typename DerivedA, typename DerivedB>
    void repmat(const Eigen::PlainObjectBase<DerivedA> & A, const int r, const int c, Eigen::PlainObjectBase<DerivedB> & B);

public:
    BaseMesh mesh;
    MeshRenderer *meshRenderer;
    //CModel m_model;
    bool isDeform;
    int deform_times;

    std::vector< BaseMesh::FaceIter > allfaces;
    std::vector< int > interV;
    // Control_pts
    std::vector< BaseMesh::Point > control_pts;
    std::vector< BaseMesh::VertexHandle > control_pts_VH;
    // igl parameter & matrix/vector
    double bc_frac;
    Eigen::MatrixXd V,U,V_bc,U_bc;
    Eigen::MatrixXi F;
    Eigen::VectorXi b;
    // Features
    std::vector< Eigen::RowVector3d >FaceFeatures;
    // 存放臉部特徵點的2D位置
    std::vector< BaseMesh::Point > pos_facefeatures;

    std::vector< BaseMesh >all_Mesh;
    //std::vector< CModel >all_m_model;

    // 存放貼的面, 點是否有使用, uv座標
    std::vector< BaseMesh::FaceHandle > m_expFHandle;
    std::vector< bool > b_Mapping;
    std::vector< BaseMesh::Point > VertexUVProp;
    std::vector<BaseMesh::Point> FaceMask;

    //照片存放路徑
    QString m_imagePath;
    //臉部特徵點
    QList<double>    m_featurePos;

    QString m_appFileDirectory;
};

#endif // QOPENMESHOBJECT_H

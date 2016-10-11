#ifndef MESHRENDERER_H
#define MESHRENDERER_H

#include "mymesh.h"

#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <OpenMesh/Core/Mesh/PolyMesh_ArrayKernelT.hh>


struct CommonShaderData
{
    QVector3D position;
    QVector3D normal;
    QVector2D texcoord;
};

struct MeshData
{
    QVector<CommonShaderData> datas;
    QVector<GLushort> indices;
};

typedef OpenMesh::PolyMesh_ArrayKernelT<>  BaseMesh;

class MeshRenderer : protected QOpenGLFunctions
{
public:
    MeshRenderer();
    virtual ~MeshRenderer();

    void drawMeshGeometry( QOpenGLShaderProgram *program, int models_idx );

    void initMeshGeometry();
    void initMeshGeometry(BaseMesh mesh);
    void TriMesh_to_Mesh(TriMesh* mesh);
    void initFaceGeometry( std::vector<BaseMesh::FaceHandle> expFH, std::vector< bool > Mapping, std::vector<BaseMesh::Point> VertexUV, std::vector<BaseMesh::Point> f_mask);
    std::vector<BaseMesh::Point> Get_uv();
    std::vector<BaseMesh::Point> Get_mask();

    static int main_costume_idx, sub_constume_idx;

    // Store the bool value of each costume which should be shown
    QVector<QVector<bool>> mVec_costume;
    // Store model's file path
    QVector<QVector<QString>> mVec_Model_filePath;
    // Stpre the value whether the costume has been loaded.
    QVector<QVector<bool>> mVec_load_costume;

    // Store model's data
    QVector<QVector<CommonShaderData>> model_data;
    QVector<QVector<CommonShaderData>> mVec_hatModel_data;
    QVector<QVector<CommonShaderData>> mVec_hairModel_data;
    QVector<QVector<CommonShaderData>> mVec_clothModel_data;
    QVector<QVector<CommonShaderData>> mVec_pantsModel_data;
    QVector<QVector<CommonShaderData>> mVec_shoesModel_data;
    QVector<QVector<CommonShaderData>> mVec_partModel_data;
    QVector<QVector<QVector<CommonShaderData>>> mVec_clothsetModel_data;

    // Store model's indices
    QVector<QVector<GLushort>> model_indices;
    QVector<QVector<GLushort>> mVec_hatModel_indices;
    QVector<QVector<GLushort>> mVec_hairModel_indices;
    QVector<QVector<GLushort>> mVec_clothModel_indices;
    QVector<QVector<GLushort>> mVec_pantsModel_indices;
    QVector<QVector<GLushort>> mVec_shoesModel_indices;
    QVector<QVector<GLushort>> mVec_partModel_indices;
    QVector<QVector<QVector<GLushort>>> mVec_clothsetModel_indices;

    int num_model;
    QVector<int> mVec_texture_order;

private:
    std::vector< bool > isRender;
    // Face uv coordinate
    std::vector<BaseMesh::FaceHandle> m_expFHandle;
    std::vector< bool > b_Mapping;
    std::vector<BaseMesh::Point> VertexUVProp;
    std::vector<BaseMesh::Point> FaceMask;

    int nFace;
    int nVertex;
    int nIndex;
    std::vector<int> m_nIndex;

    vector<BaseMesh::Normal> model_normal;
    // Store the amount of each costume
    QVector<int> mVec_num_costume;

    QOpenGLBuffer arrayBuf;
    QOpenGLBuffer indexBuf;
};

#endif // MESHRENDERER_H

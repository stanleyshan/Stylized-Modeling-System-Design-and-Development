#ifndef EXPMAP
#define EXPMAP

#include "qopenmeshobject.h"

#include <vector>
#include <queue>

class ExpMap
{
public:
    ExpMap();
    ~ExpMap();
    // Main function
    void TextureMapping(BaseMesh deformMesh, std::vector< BaseMesh::Point > cp, std::vector< BaseMesh::VertexHandle > cp_VH, std::vector< BaseMesh::Point > pf, int imageSize);
    // Initialize
    void Init();
    void Reset_Property();
    void ExpMap_Boundary();
    // Exponential Maps
    void ExpMapFace();
    void Collect_ExpMapFHandle();
    void Compute_Parameter();
    void ReComp_ExpMapFace();
    void Recalculate_uv();
    // Functions
    BaseMesh::Point RotatePointIntoPlane(BaseMesh::Point pt, BaseMesh::Point planeOrigin, BaseMesh::Point planeNormal);
    BaseMesh::Point ProjectPointToPlane (BaseMesh::Point pt, BaseMesh::Point planeNormal, double planeConstant);
    double DistanceTo (BaseMesh::Point pt, BaseMesh::Point planeNormal, double planeConstant);
    // Rotate UV
    void uvRotate(float angle);
    // Operations
    BaseMesh::Point Get_Cross(BaseMesh::Point p1, BaseMesh::Point p2);
    double Get_Dot(BaseMesh::Point a, BaseMesh::Point b);
    double Get_Theta(BaseMesh::Point n1, BaseMesh::Point n2);
    double Get_Norm(BaseMesh::Point p);
    // Return functions
    std::vector< BaseMesh::FaceHandle > Get_expFH();
    std::vector< bool > Get_Mapping();
    std::vector< BaseMesh::Point > Get_uv();
    std::vector< BaseMesh::Point > Get_mask();
    // Face mask
    void Face_mask();
public:
    BaseMesh mesh;
    // 存放control points
    std::vector< BaseMesh::Point > control_pts;
    std::vector< BaseMesh::VertexHandle > control_pts_VH;
    // 存放臉部特徵點的2D位置
    std::vector< BaseMesh::Point > pos_facefeatures;
    // 存放所有點
    std::vector< BaseMesh::VertexIter > m_vertex;

    // uv點是否在貼圖上(property(b_Mapping, v))
    std::vector< bool > b_Mapping;
    // 點是否有用過(property(isUseV,vv_it.handle()))
    std::vector< bool > isUseV;
    // 切平面上的點座標(property(surfacePoint, vv_it.handle()))
    std::vector< BaseMesh::Point > surfacePoint;
    // 根據哪個鄰居點翻轉
    std::vector< BaseMesh::VertexHandle > VertexNeighbor;
    // uv-coordinate
    std::vector< BaseMesh::Point > VertexUVProp;

    // 存放點的queue
    std::queue< BaseMesh::VertexHandle > vertexQueue;
    // 存放Weight
    std::vector< BaseMesh::FaceHandle > m_expFHandle;
    // Parameter
    int    m_Vertex;
    int    m_Face;
    int    m_imageSize;
    double m_radis;
    double m_angle;

    // Face mask points
    std::vector< BaseMesh::Point > mask;
};

#endif // EXPMAP

#include "expmap.h"

ExpMap::ExpMap()
{

}

ExpMap::~ExpMap()
{

}

void ExpMap::TextureMapping(BaseMesh deformMesh, std::vector< BaseMesh::Point > cp, std::vector< BaseMesh::VertexHandle > cp_VH, std::vector< BaseMesh::Point > pf, int imageSize)
{
    mesh = deformMesh;
    control_pts = cp;
    control_pts_VH = cp_VH;
    pos_facefeatures = pf;
    m_imageSize = imageSize;
    Init();
    ExpMapFace();
    Compute_Parameter();
    Recalculate_uv();
    // Face mask points
    Face_mask();
}

void ExpMap::Init()
{
    int m_Vertex = 0, m_Face = 0;
    for ( BaseMesh::VertexIter v_it = mesh.vertices_begin(); v_it != mesh.vertices_end(); ++v_it )
    {
        m_Vertex++;
        m_vertex.push_back(v_it);
    }
    for( BaseMesh::FaceIter f_it = mesh.faces_begin(); f_it != mesh.faces_end(); ++f_it )
    {
        m_Face++;
    }
    b_Mapping.resize(m_Vertex, false);
    isUseV.resize(m_Vertex, false);
    VertexNeighbor.resize(m_Vertex);
    VertexUVProp.resize(m_Vertex, BaseMesh::Point(0,0,0));
}

void ExpMap::Reset_Property()
{
    // Clear all labels
    for ( BaseMesh::VertexIter v_it = mesh.vertices_begin(); v_it != mesh.vertices_end(); ++v_it )
    {
        if(b_Mapping[v_it->idx()])
        {
            b_Mapping[v_it->idx()] = false;
        }
        isUseV[v_it->idx()] = false;
    }
    ExpMap_Boundary();
}

void ExpMap::ExpMap_Boundary()
{
    // Face
//    int vBoundary[] = {349,304,305,130,128,127,126,815,669,110,342,565,31,2302,2079,1847,2406,2552,1863,1864,1865,1867,2042,2041,2086,2013,2094,2516,3579,2095,3590,2351,3600,2352,3613,2353,2354,2360,2364,2506,2358,2356,2371,2367,1846,3,109,630,634,619,621,769,627,623,617,616,3624,615,3636,614,3650,358,3663,779,357,276};
    int vBoundary[] = {279,260,261,131,129,128,127,720,594,111,272,490,31,1982,1819,1817,2260,1805,1803,1822,1983,3424,3429,3427,1989,1979,1977,3371,3393,3402,3362,3349,3334,3325,3299,3297,2205,1813,2235,2237,2227,2228,2231,2242,1802,3,110,555,559,544,546,675,552,548,542,541,1727,540,1739,539,1753,287,1766,685,286,239};
    for ( BaseMesh::VertexIter v_it = mesh.vertices_begin(); v_it != mesh.vertices_end(); ++v_it )
    {
        for( size_t i = 0; i<sizeof(vBoundary)/sizeof(*vBoundary); i++ )
        {
            if( v_it->idx() == vBoundary[i] )
            {
                isUseV[v_it->idx()] = true;
            }
        }
    }
}

void ExpMap::ExpMapFace()
{
    Reset_Property();
    m_radis = 1.9;
    BaseMesh::Point planeNormal(0,0,0);
    BaseMesh::Point xyplaneNormal(0,0,1);
    std::vector<int> origin_vertexIdx;
    // ExpMap mid control point (nose mid 36,37)
    int ExpMap_mid_idx = 37;
    BaseMesh::Point clickPoint = BaseMesh::Point ( control_pts[ExpMap_mid_idx].data()[0], control_pts[ExpMap_mid_idx].data()[1], control_pts[ExpMap_mid_idx].data()[2]);

    // 中心點不需要攤，因此將此點設為已攤過
    for ( BaseMesh::VertexIter v_it = mesh.vertices_begin(); v_it != mesh.vertices_end(); ++v_it )
    {
        if(v_it->idx() == control_pts_VH[ExpMap_mid_idx].idx())
        {
            // 該點已攤
            isUseV[v_it->idx()] = true;
            b_Mapping[v_it->idx()] = true;
            // UV-coordinate
            BaseMesh::Point planeP(m_radis * (mesh.point(*v_it)[0] - clickPoint[0]), 0, m_radis * (mesh.point(*v_it)[1] - clickPoint[1]));
            VertexUVProp[v_it->idx()] = planeP;
            // 根據鄰近點翻轉到下個平面
            VertexNeighbor[v_it->idx()] = *v_it;
            vertexQueue.push(*v_it);

            // 一開始平面的normal
            planeNormal += mesh.normal(*v_it);
            // 一開始攤的面的點index
            origin_vertexIdx.push_back(v_it->idx());
            break;
        }
    }

    while(!vertexQueue.empty())
    {
        BaseMesh::VertexHandle vh = vertexQueue.front();

        // Vertex oneRing
        for( BaseMesh::VertexVertexIter vv_it = mesh.vv_iter(vh); vv_it.is_valid(); vv_it++ )
        {
            // 判斷此點是否已經被攤過
            if(!b_Mapping[vv_it->idx()] && !isUseV[vv_it->idx()])
            {
                // 該點已攤過
                isUseV[vv_it->idx()] = true;
                // 根據鄰居的點旋轉
                VertexNeighbor[vv_it->idx()] = vh;
                // neighbor vertex rotate
                int nextV = vh.idx();
                BaseMesh::Point rotateV = mesh.point(*vv_it);
                BaseMesh::Point projectP = RotatePointIntoPlane(rotateV, mesh.point(*m_vertex[nextV]), mesh.normal(*m_vertex[nextV]));
                // surfacePoint
                BaseMesh::Point surfacePoint = RotatePointIntoPlane(projectP, clickPoint, xyplaneNormal);
                // surfaceVec
                BaseMesh::Point surfaceVec(m_radis * (surfacePoint[0]-clickPoint[0]), 0, m_radis * (surfacePoint[1]-clickPoint[1]));
                // UV-coordinate
                VertexUVProp[vv_it->idx()] = surfaceVec;
                double uv_x = VertexUVProp[vv_it->idx()][0];
                double uv_y = VertexUVProp[vv_it->idx()][2];

                if(uv_x<-0.5)
                {
                    uv_x = -0.5;
                }
                else if(uv_x>0.5)
                {
                    uv_x = 0.5;
                }
                if(uv_y<-0.5)
                {
                    uv_y = -0.5;
                }
                else if(uv_y>0.5)
                {
                    uv_y = 0.5;
                }
                // 鄰居可繼續翻轉的點
                vertexQueue.push(*vv_it);
                // 將點的狀態設定為已攤過
                b_Mapping[vv_it->idx()] = true;
            }
        }
        vertexQueue.pop();
    }
    Collect_ExpMapFHandle();
}

void ExpMap::Collect_ExpMapFHandle()
{
    m_expFHandle.clear();
    BaseMesh::VertexHandle v1, v2, v3;
    for( BaseMesh::FaceIter f_it = mesh.faces_begin(); f_it != mesh.faces_end(); ++f_it )
    {
        BaseMesh::FaceVertexIter fv_it = mesh.fv_iter( *f_it );
        v1 = *fv_it;
        v2 = *(++fv_it);
        v3 = *(++fv_it);

        // Make sure three points of a triangle have its' uv
        if(b_Mapping[v1.idx()] && b_Mapping[v2.idx()] && b_Mapping[v3.idx()])
        {
            m_expFHandle.push_back(*f_it);
        }
    }
}

void ExpMap::Compute_Parameter()
{
    // 計算Rotate & Scale
    BaseMesh::Point vec_a = pos_facefeatures[43] - pos_facefeatures[19];
    vec_a.data()[1] *= -1;
    BaseMesh::VertexHandle mid, mouth_up, eyes_left;
    int times = 0;
    for ( BaseMesh::VertexIter v_it = mesh.vertices_begin(); v_it != mesh.vertices_end(); ++v_it )
    {
        if(v_it->idx()==control_pts_VH[37].idx())
        {
            mid = *v_it;
            times++;
        }
        else if(v_it->idx()==control_pts_VH[65].idx())
        {
            mouth_up = *v_it;
            times++;
        }
        else if(v_it->idx()==control_pts_VH[42].idx())
        {
            eyes_left = *v_it;
            times++;
        }
        if(times==3)
        {
            break;
        }
    }
    BaseMesh::Point vec_b = BaseMesh::Point((VertexUVProp[mouth_up.idx()][0] - VertexUVProp[mid.idx()][0])*m_imageSize, (VertexUVProp[mouth_up.idx()][2] - VertexUVProp[mid.idx()][2])*m_imageSize, 0);
    BaseMesh::Point vec_c = pos_facefeatures[24] - pos_facefeatures[19];
    BaseMesh::Point vec_d = BaseMesh::Point((VertexUVProp[eyes_left.idx()][0] - VertexUVProp[mid.idx()][0])*m_imageSize, (VertexUVProp[eyes_left.idx()][2] - VertexUVProp[mid.idx()][2])*m_imageSize, 0);
    m_angle = Get_Theta(vec_a, vec_b);
    uvRotate(m_angle);

    // Compute the correct m_radis & Recompute the ExpMap
    m_radis *= Get_Norm(vec_c) / Get_Norm(vec_d);

    // Fitting the face
    for ( BaseMesh::VertexIter v_it = mesh.vertices_begin(); v_it != mesh.vertices_end(); ++v_it )
    {
        if(b_Mapping[v_it->idx()])
        {
            VertexUVProp[v_it->idx()] = VertexUVProp[v_it->idx()] / 1.6 * m_radis;
        }
    }
}

void ExpMap::ReComp_ExpMapFace()
{
    Reset_Property();
    BaseMesh::Point planeNormal(0,0,0);
    BaseMesh::Point xyplaneNormal(0,0,1);
    std::vector<int> origin_vertexIdx;
    // ExpMap mid control point (nose mid 36,37)
    int ExpMap_mid_idx = 37;
    BaseMesh::Point clickPoint = BaseMesh::Point ( control_pts[ExpMap_mid_idx].data()[0], control_pts[ExpMap_mid_idx].data()[1], control_pts[ExpMap_mid_idx].data()[2]);
    // 中心點不需要攤，因此將此點設為已攤過
    for ( BaseMesh::VertexIter v_it = mesh.vertices_begin(); v_it != mesh.vertices_end(); ++v_it )
    {
        if(v_it->idx()==control_pts_VH[ExpMap_mid_idx].idx())
        {
            // 該點已攤
            isUseV[v_it->idx()] = true;
            b_Mapping[v_it->idx()] = true;
            // UV-coordinate
            BaseMesh::Point planeP(m_radis * (mesh.point(*v_it)[0] - clickPoint[0]), 0, m_radis * (mesh.point(*v_it)[1] - clickPoint[1]));
            VertexUVProp[v_it->idx()] = planeP;
            // 根據鄰近點翻轉到下個平面
            VertexNeighbor[v_it->idx()] = *v_it;
            vertexQueue.push(*v_it);

            // 一開始平面的normal
            planeNormal += mesh.normal(*v_it);
            // 一開始攤的面的點index
            origin_vertexIdx.push_back(v_it->idx());
            break;
        }
    }

    while(!vertexQueue.empty())
    {
        BaseMesh::VertexHandle vh = vertexQueue.front();

        // vertex oneRing
        for( BaseMesh::VertexVertexIter vv_it = mesh.vv_iter(vh); vv_it.is_valid(); vv_it++ )
        {
            // 判斷此點是否已經被攤過
            if(!b_Mapping[vv_it->idx()] && !isUseV[vv_it->idx()])
            {
                // 該點已攤過
                isUseV[vv_it->idx()] = true;
                // 根據鄰居的點旋轉
                VertexNeighbor[vv_it->idx()] = vh;
                // neighbor vertex rotate
                int nextV = vh.idx();
                BaseMesh::Point rotateV = mesh.point(*vv_it);
                BaseMesh::Point projectP = RotatePointIntoPlane(rotateV, mesh.point(*m_vertex[nextV]), mesh.normal(*m_vertex[nextV]));
                // surfacePoint
                BaseMesh::Point surfacePoint = RotatePointIntoPlane(projectP, clickPoint, xyplaneNormal);
                // surfaceVec
                BaseMesh::Point surfaceVec(m_radis * (surfacePoint[0]-clickPoint[0]), 0, m_radis * (surfacePoint[1]-clickPoint[1]));
                // UV-coordinate
                VertexUVProp[vv_it->idx()] = surfaceVec;
                double uv_x = VertexUVProp[vv_it->idx()][0];
                double uv_y = VertexUVProp[vv_it->idx()][2];
                if(uv_x>-0.5 && uv_x<0.5 && uv_y>-0.5 && uv_y<0.5)
                {
                    // 鄰居可繼續翻轉的點
                    vertexQueue.push(*vv_it);
                    // 將點的狀態設定為已攤過
                    b_Mapping[vv_it->idx()] = true;
                }
            }
        }
        vertexQueue.pop();
    }
    Collect_ExpMapFHandle();
}

void ExpMap::Recalculate_uv()
{
    // According to the position of the 37th feature point, recalculate the uv
    float bias_x = (m_imageSize / 2 - pos_facefeatures[19].data()[0]) / m_imageSize;
    float bias_y = (pos_facefeatures[19].data()[1] - m_imageSize / 2) / m_imageSize;
    for( size_t i = 0; i<b_Mapping.size(); i++ )
    {
        // z value to y
        VertexUVProp[i][1] = VertexUVProp[i][2];
        VertexUVProp[i][2] = 0;
        // Move uv to correct position
        if(b_Mapping[i])
        {
            VertexUVProp[i][0] = VertexUVProp[i][0] + 0.5 - bias_x;
            VertexUVProp[i][1] = VertexUVProp[i][1] + 0.5 - bias_y;
        }
    }
}

BaseMesh::Point ExpMap::RotatePointIntoPlane(BaseMesh::Point pt, BaseMesh::Point planeOrigin, BaseMesh::Point planeNormal)
{
    double planeConstant = Get_Dot(planeNormal, planeOrigin);
    // Find projected point in plane
    BaseMesh::Point projectPt = ProjectPointToPlane(pt, planeNormal, planeConstant);
    // Find angle between projected point and original point
    BaseMesh::Point org = pt - planeOrigin;
    BaseMesh::Point proj = projectPt - planeOrigin;
    if( Get_Norm(proj) < std::exp(-20.0) )
        return projectPt;
    double scale = Get_Norm(org) / Get_Norm(proj);
    return planeOrigin + proj * scale;
}

BaseMesh::Point ExpMap::ProjectPointToPlane (BaseMesh::Point pt, BaseMesh::Point planeNormal, double planeConstant)
{
    return pt - planeNormal * DistanceTo(pt, planeNormal, planeConstant);
}

double ExpMap::DistanceTo (BaseMesh::Point pt, BaseMesh::Point planeNormal, double planeConstant)
{
    return Get_Dot(planeNormal, pt) - planeConstant;
}

void ExpMap::uvRotate(float angle)
{
    for ( BaseMesh::VertexIter v_it = mesh.vertices_begin(); v_it != mesh.vertices_end(); ++v_it )
    {
        if(b_Mapping[v_it->idx()])
        {
            BaseMesh::Point newUV = VertexUVProp[v_it->idx()];
            BaseMesh::Point temp = newUV;

            newUV[0] = temp[0] * cos(angle) - temp[2] * sin(angle);
            newUV[2] = temp[0] * sin(angle) + temp[2] * cos(angle);

            VertexUVProp[v_it->idx()] = newUV;
        }
    }
}

BaseMesh::Point ExpMap::Get_Cross(BaseMesh::Point p1, BaseMesh::Point p2)
{
    return BaseMesh::Point(
            p1[1] * p2[2] - p1[2] * p2[1],
            p1[2] * p2[0] - p1[0] * p2[2],
            p1[0] * p2[1] - p1[1] * p2[0]);
}

double ExpMap::Get_Dot(BaseMesh::Point a, BaseMesh::Point b)
{
    return ( a[0]*b[0] + a[1]*b[1] + a[2]*b[2] );
}

double ExpMap::Get_Theta(BaseMesh::Point n1, BaseMesh::Point n2)
{
    return acos(Get_Dot(n1,n2)/(Get_Norm(n1)*Get_Norm(n2)));
}

double ExpMap::Get_Norm(BaseMesh::Point p)
{
    return sqrt(p[0]*p[0] + p[1]*p[1] +p[2]*p[2]);
}

std::vector< BaseMesh::FaceHandle > ExpMap::Get_expFH()
{
    return m_expFHandle;
}

std::vector< bool > ExpMap::Get_Mapping()
{
    return b_Mapping;
}

std::vector< BaseMesh::Point > ExpMap::Get_uv()
{
    return VertexUVProp;
}

void ExpMap::Face_mask()
{
    mask.push_back(VertexUVProp[129]);
    for(size_t i = 0; i<15; i++)
    {
        BaseMesh::Point tmp(pos_facefeatures[i][0]/m_imageSize, pos_facefeatures[i][1]/m_imageSize ,0);
        mask.push_back(tmp);
    }
    mask.push_back(VertexUVProp[1866]);
}

std::vector< BaseMesh::Point > ExpMap::Get_mask()
{
    return mask;
}

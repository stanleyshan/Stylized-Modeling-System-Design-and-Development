#ifndef FACETEX
#define FACETEX

#include "qopenmeshobject.h"
#include <opencv2/imgproc/imgproc.hpp>
#include <vector>
#include <QOpenGLTexture>

using namespace cv;

class FaceTex
{
public:
    FaceTex();
    ~FaceTex();
    QOpenGLTexture* Tex(QImage src, QRgb color, vector<BaseMesh::Point> ff);
    bool inRegion(BaseMesh::Point p);
    double Heron(BaseMesh::Point va, BaseMesh::Point vb, BaseMesh::Point vc);
    void boundaryP();
    //void minCut(BaseMesh::Point p, int dir);
    void minCut(int start, int end, vector< BaseMesh::Point > useP, int dir);
    double colorDistance(QRgb p1, QRgb p2);
    void linear_interpolation(BaseMesh::Point p, int dir);
    void catchSkin(Mat pDst, Mat pSkinSrc);

    BaseMesh *mesh;
    double area;
    QImage img;
    QRgb skinColor;
    QOpenGLTexture *texture;
    vector< BaseMesh::Point > allFeatures;
    vector< BaseMesh::Point > maskBoundary;
};

#endif // FACETEX

#include "facetex.h"
#include <cmath>
#include <QPainter>
#include <QColor>

FaceTex::FaceTex()
{

}

FaceTex::~FaceTex()
{

}

QOpenGLTexture* FaceTex::Tex(QImage src, QRgb color, std::vector<BaseMesh::Point> ff)
{
    QImage afterBlendingImage = QImage(Plugin::m_appFileDirectory+"/afterBlending.png").mirrored();
//    QImage afterBlendingImage = QImage("/mnt/sdcard/Download/chess.png").mirrored();
    // Move to correct uv coordinate
    for(size_t i = 1; i<16 ;i++)
    {
        ff[i][1] = src.width() - ff[i][1];
    }
    img = src;
    // Scale
    if(img.width()>800)
    {
        QTransform transform;
        float s = 800.0f / img.height();
        transform.scale(s, s);

        for(int i=0; i<ff.size(); i++)
        {
            ff[i][0] *= s;
            ff[i][1] *= s;
        }
        img = img.transformed(transform);
    }
    // initialize
    skinColor = color;
    allFeatures = ff;
    // Compute the area of the face in 2D image
    area = 0;
    BaseMesh::Point mid(0,0,0);
    int v_size = 0;
    for(size_t i = 0; i<allFeatures.size(); i++)
    {
        mid += allFeatures[i];
        v_size++;
    }
    mid /= v_size;
    for(size_t i = 0; i<allFeatures.size(); i++)
    {
        if(i!=allFeatures.size()-1)
        {
            area += Heron(mid,allFeatures[i],allFeatures[i+1]);
        }
        else
        {
            area += Heron(mid,allFeatures[i],allFeatures[0]);
        }
    }
    QPainter painter(&img);
    QRectF targetRect, srcRect;

    if(src.width()>800)
    {
        //face
        srcRect = QRectF(0, 0, afterBlendingImage.width(), afterBlendingImage.height());
        int sx = Plugin::m_featurePos[2]*800.0f/src.width();
        int sy = img.height() - Plugin::m_featurePos[15]*800.0f/src.width();
        int ex = Plugin::m_featurePos[26]*800.0f/src.width();
        int ey = img.height() - Plugin::m_featurePos[41]*800.0f/src.width();
        targetRect = QRectF(sx, sy, ex-sx, ey-sy);

        QImage oldFace = img.copy(sx, sy, ex-sx, ey-sy);
        oldFace.save(Plugin::m_appFileDirectory+"/oldFace.png");
        painter.drawImage(targetRect, afterBlendingImage, srcRect);
        QImage newFace = img.copy(sx, sy, ex-sx, ey-sy);
        newFace.save(Plugin::m_appFileDirectory+"/newFace.png");

        oldFace = oldFace.convertToFormat(QImage::Format_RGB888);
        newFace = newFace.convertToFormat(QImage::Format_RGB888);

        Mat matOldFace(oldFace.height(), oldFace.width(), CV_8UC3, (uchar*)oldFace.bits(), oldFace.bytesPerLine());
        Mat matNewFace(newFace.height(), newFace.width(), CV_8UC3, (uchar*)newFace.bits(), newFace.bytesPerLine());

        catchSkin(matNewFace, matOldFace);
        newFace = QImage(matNewFace.data, matNewFace.cols, matNewFace.rows, matNewFace.step, QImage::Format_RGB888).copy();
        newFace.save(Plugin::m_appFileDirectory+"/newFace1.png");
        srcRect = QRectF(0, 0, newFace.width(), newFace.height());
        painter.drawImage(targetRect, newFace, srcRect);

        int tmp_sx, tmp_sy, tmp_ex, tmp_ey;
        //sketch the area over eyebrow
        tmp_sx = sx;
        tmp_sy = sy;
        tmp_ex = ex;
        tmp_ey = sy+20;
        srcRect = QRectF(tmp_sx, tmp_sy, tmp_ex-tmp_sx, tmp_ey-tmp_sy);
        targetRect = QRectF(sx, ey, ex-sx, 20);
        painter.drawImage(targetRect, img, srcRect);

        //sketch the area next to the right chin
        tmp_sx = (Plugin::m_featurePos[56]*800.0f/src.width()+ex)/2;
        tmp_sy = sy;
        tmp_ex = ex;
        tmp_ey = ey;
        srcRect = QRectF(tmp_sx, tmp_sy, tmp_ex-tmp_sx, tmp_ey-tmp_sy);
        targetRect = QRectF(ex, sy, 30, ey-sy);
        painter.drawImage(targetRect, img, srcRect);

        //sketch the area below chin
        tmp_sx = sx;
        tmp_sy = sy;
        tmp_ex = ex;
        tmp_ey = sy+20;
        srcRect = QRectF(tmp_sx, tmp_sy, tmp_ex-tmp_sx, tmp_ey-tmp_sy);
        targetRect = QRectF(sx, sy-20, ex-sx, 20);
        painter.drawImage(targetRect, img, srcRect);

        //sketch the area next to the left chin
        tmp_sx = sx;
        tmp_sy = sy;
        tmp_ex = (Plugin::m_featurePos[46]*800.0f/src.width()+sx)/2;
        tmp_ey = ey;
        srcRect = QRectF(tmp_sx, tmp_sy, tmp_ex-tmp_sx, tmp_ey-tmp_sy);
        targetRect = QRectF(sx-30, sy, 30, ey-sy);
        painter.drawImage(targetRect, img, srcRect);
    }
    else
    {
        //face
        srcRect = QRectF(0, 0, afterBlendingImage.width(), afterBlendingImage.height());
        int sx = Plugin::m_featurePos[2];
        int sy = img.height() - Plugin::m_featurePos[15];
        int ex = Plugin::m_featurePos[26];
        int ey = img.height() - Plugin::m_featurePos[41];
        targetRect = QRectF(sx, sy, ex-sx, ey-sy);QImage oldFace = img.copy(sx, sy, ex-sx, ey-sy);

        oldFace.save(Plugin::m_appFileDirectory+"/oldFace.png");
        painter.drawImage(targetRect, afterBlendingImage, srcRect);
        QImage newFace = img.copy(sx, sy, ex-sx, ey-sy);
        newFace.save(Plugin::m_appFileDirectory+"/newFace.png");

        Mat matOldFace(oldFace.height(), oldFace.width(), CV_8UC3, (uchar*)oldFace.bits(), oldFace.bytesPerLine());
        Mat matNewFace(newFace.height(), newFace.width(), CV_8UC3, (uchar*)newFace.bits(), newFace.bytesPerLine());

        catchSkin(matNewFace, matOldFace);
        newFace = QImage(matNewFace.data, matNewFace.cols, matNewFace.rows, matNewFace.step, QImage::Format_RGB888).copy();
        srcRect = QRectF(0, 0, newFace.width(), newFace.height());
        painter.drawImage(targetRect, newFace, srcRect);

        int tmp_sx, tmp_sy, tmp_ex, tmp_ey;
        //sketch the area over eyebrow
        tmp_sx = sx;
        tmp_sy = ((img.height() - Plugin::m_featurePos[49]) + ey) / 3.0;
        tmp_ex = ex;
        tmp_ey = ey;
        srcRect = QRectF(tmp_sx, tmp_sy, tmp_ex-tmp_sx, tmp_ey-tmp_sy);
        targetRect = QRectF(sx, ey, ex-sx, 20);
        painter.drawImage(targetRect, img, srcRect);

        //sketch the area next to the right chin
        tmp_sx = (Plugin::m_featurePos[56]+ex)/2;
        tmp_sy = sy;
        tmp_ex = ex;
        tmp_ey = ey;
        srcRect = QRectF(tmp_sx, tmp_sy, tmp_ex-tmp_sx, tmp_ey-tmp_sy);
        targetRect = QRectF(ex, sy, 30, ey-sy);
        painter.drawImage(targetRect, img, srcRect);

        //sketch the area below chin
        tmp_sx = sx;
        tmp_sy = ((img.height() - Plugin::m_featurePos[49]) + ey) / 2.0;
        tmp_ex = ex;
        tmp_ey = ey;
        srcRect = QRectF(tmp_sx, tmp_sy, tmp_ex-tmp_sx, tmp_ey-tmp_sy);
        targetRect = QRectF(sx, ey, ex-sx, 20);
        painter.drawImage(targetRect, img, srcRect);

        //sketch the area next to the left chin
        tmp_sx = sx;
        tmp_sy = sy;
        tmp_ex = (Plugin::m_featurePos[46]+sx)/2;
        tmp_ey = ey;
        srcRect = QRectF(tmp_sx, tmp_sy, tmp_ex-tmp_sx, tmp_ey-tmp_sy);
        targetRect = QRectF(sx-30, sy, 30, ey-sy);
        painter.drawImage(targetRect, img, srcRect);
    }
    // For debug -------------------------------------------------------------------
    // Save image
    qDebug() << img.save(Plugin::m_appFileDirectory+"/face.jpg");
    // -----------------------------------------------------------------------------
    texture = new QOpenGLTexture(img);
    texture->setMinificationFilter(QOpenGLTexture::Nearest);
    texture->setMagnificationFilter(QOpenGLTexture::Linear);
    texture->setWrapMode(QOpenGLTexture::ClampToEdge);
    return texture;
}

bool FaceTex::inRegion(BaseMesh::Point p)
{
    double sumArea = 0;
    for(size_t i = 0; i<allFeatures.size(); i++)
    {
        if(i!=allFeatures.size()-1)
        {
            sumArea += Heron(p,allFeatures[i],allFeatures[i+1]);
        }
        else
        {
            sumArea += Heron(p,allFeatures[i],allFeatures[0]);
        }
    }
    if(fabs(area-sumArea)<pow(10,-1))
    {
        return true;
    }
    else
    {
        return false;
    }
}

double FaceTex::Heron(BaseMesh::Point va, BaseMesh::Point vb, BaseMesh::Point vc)
{
    double triangleArea, ae, be, ce, pe;
    ae = sqrt(pow(vb[0]-va[0],2)+pow(vb[1]-va[1],2)+pow(vb[2]-va[2],2));
    be = sqrt(pow(vc[0]-vb[0],2)+pow(vc[1]-vb[1],2)+pow(vc[2]-vb[2],2));
    ce = sqrt(pow(vc[0]-va[0],2)+pow(vc[1]-va[1],2)+pow(vc[2]-va[2],2));
    pe = (ae+be+ce)/2;
    triangleArea = sqrt(pe*(pe-ae)*(pe-be)*(pe-ce));
    return triangleArea;
}

void FaceTex::boundaryP()
{
    // Find every boundary points
    for(size_t i = 0; i<allFeatures.size(); i++)
    {
        if(i!=allFeatures.size()-1)
        {
            double width, height;
            width = fabs(allFeatures[i][0]-allFeatures[i+1][0]);
            height = fabs(allFeatures[i][1]-allFeatures[i+1][1]);
            if(width>height)
            {
                // start to end (x-axis small to large)
                int start, end;
                if(round(allFeatures[i][0])<round(allFeatures[i+1][0]))
                {
                    start = round(allFeatures[i][0]);
                    end = round(allFeatures[i+1][0]);
                }
                else
                {
                    start = round(allFeatures[i+1][0]);
                    end = round(allFeatures[i][0]);
                }
                // linear equation parameter (y unknown)
                double a, b;
                a = (allFeatures[i][1]-allFeatures[i+1][1]) / (allFeatures[i][0]-allFeatures[i+1][0]);
                b = allFeatures[i][1] - a * allFeatures[i][0];
                // Every points from start to end
                vector< BaseMesh::Point > useP;
                for(int j = start; j<=end; j++)
                {
                    BaseMesh::Point boundary(j, round(j*a+b), 0);
                    maskBoundary.push_back(boundary);
                    useP.push_back(boundary);
                    // Minimum cut
                    //minCut(boundary, 2);
                    // Linear interpolation
                    //linear_interpolation(boundary, 2);
                }
                minCut(start, end, useP, 2);
            }
            else
            {
                // start to end (y-axis small to large)
                int start, end;
                if(round(allFeatures[i][1])<round(allFeatures[i+1][1]))
                {
                    start = round(allFeatures[i][1]);
                    end = round(allFeatures[i+1][1]);
                }
                else
                {
                    start = round(allFeatures[i+1][1]);
                    end = round(allFeatures[i][1]);
                }
                // linear equation parameter (x unknown)
                double a, b;
                a = (allFeatures[i][1]-allFeatures[i+1][1]) / (allFeatures[i][0]-allFeatures[i+1][0]);
                b = allFeatures[i][1] - a * allFeatures[i][0];
                // Every points from start to end
                vector< BaseMesh::Point > useP;
                for(int j = start; j<=end; j++)
                {
                    BaseMesh::Point boundary(round((j-b)/a), j, 0);
                    maskBoundary.push_back(boundary);
                    useP.push_back(boundary);
                    // Minimum cut
                    //minCut(boundary, 1);
                    // Linear interpolation
                    //linear_interpolation(boundary, 1);
                }
                minCut(start, end, useP, 1);
            }
        }
        else
        {
            // start to end (x-axis small to large)
            int start, end;
            if(round(allFeatures[i][0])<round(allFeatures[0][0]))
            {
                start = round(allFeatures[i][0]);
                end = round(allFeatures[0][0]);
            }
            else
            {
                start = round(allFeatures[0][0]);
                end = round(allFeatures[i][0]);
            }
            // linear equation parameter (y unknown)
            double a, b;
            a = (allFeatures[i][1]-allFeatures[0][1]) / (allFeatures[i][0]-allFeatures[0][0]);
            b = allFeatures[i][1] - a * allFeatures[i][0];
            // Every points from start to end
            vector< BaseMesh::Point > useP;
            for(int j = start; j<=end; j++)
            {
                BaseMesh::Point boundary(j, round(j*a+b), 0);
                maskBoundary.push_back(boundary);
                useP.push_back(boundary);
                // Minimum cut
                //minCut(boundary, 2);
                // Linear interpolation
                //linear_interpolation(boundary, 2);
            }
            minCut(start, end, useP, 2);
        }
    }
}

void FaceTex::minCut(int start, int end, vector< BaseMesh::Point > useP, int dir)
{
    Q_UNUSED(start);
    Q_UNUSED(end);

    int block_size = 30;
    int imgSize = img.width();
    // Last minimum neighbor error (x,y)
    Eigen::MatrixXd minNeighbor_x, minNeighbor_y;
    minNeighbor_x.resize(imgSize,imgSize);
    minNeighbor_y.resize(imgSize,imgSize);
    // Minimum cost
    Eigen::MatrixXd minCost;
    minCost.resize(imgSize,imgSize);
    // Average cost
    double average_cost = 1000;
    // Patch image
    QImage patch(imgSize, imgSize, QImage::Format_RGB32);
    for(int i = 0; i<imgSize; i++)
    {
        for(int j = 0; j<imgSize; j++)
        {
            patch.setPixel(i, j, skinColor);
        }
    }
    // dir = 1 left&right || dir = 2 up&down (equals where is not the skin color?)
    if(dir==1)
    {
        QRgb right = img.pixel(useP[0][0]+block_size,useP[0][1]);
        QRgb left = img.pixel(useP[0][0]-block_size,useP[0][1]);
        double err1 = colorDistance(skinColor, right);
        double err2 = colorDistance(skinColor, left);
        // up or down move ?
        int moveDirection;
        if(useP[1][1]-useP[0][1]>0)
        {
            // up move (previous y)
            moveDirection = -1;
        }
        else
        {
            // down move (previous y)
            moveDirection = 1;
        }
        // Determine left or right case
        if(err1<err2)
        {
            // Left
            for(int idx = 0; idx<useP.size(); idx++)
            {
                for(int i = 1; i<block_size; i++)
                {
                    QRgb left_pixel = img.pixel(useP[idx][0]-i,useP[idx][1]);
                    QRgb patch_pixel = patch.pixel(useP[idx][0]-i,useP[idx][1]);
                    double d = colorDistance(patch_pixel, left_pixel);
                    if(idx==0)
                    {
                        if(d==0)
                        {
                            d = 1;
                        }
                        minCost(useP[idx][0]-i, useP[idx][1]) = d;
                    }
                    else
                    {
                        double cost1 = minCost(useP[idx][0]-i-1, useP[idx][1]+moveDirection);
                        double cost2 = minCost(useP[idx][0]-i  , useP[idx][1]+moveDirection);
                        double cost3 = minCost(useP[idx][0]-i+1, useP[idx][1]+moveDirection);
                        if(cost1<=cost2 && cost1<=cost3 && cost1!=0)
                        {
                            d += cost1;
                            minNeighbor_x(useP[idx][0]-i, useP[idx][1]) = useP[idx][0]-i-1;
                            minNeighbor_y(useP[idx][0]-i, useP[idx][1]) = useP[idx][1]+moveDirection;
                        }
                        else if(cost2<=cost1 && cost2<=cost3 && cost2!=0)
                        {
                            d += cost2;
                            minNeighbor_x(useP[idx][0]-i, useP[idx][1]) = useP[idx][0]-i;
                            minNeighbor_y(useP[idx][0]-i, useP[idx][1]) = useP[idx][1]+moveDirection;
                        }
                        else if(cost3<=cost1 && cost3<=cost2 && cost3!=0)
                        {
                            d += cost3;
                            minNeighbor_x(useP[idx][0]-i, useP[idx][1]) = useP[idx][0]-i+1;
                            minNeighbor_y(useP[idx][0]-i, useP[idx][1]) = useP[idx][1]+moveDirection;
                        }
                        else
                        {
                            // No neighbors
                            d += 999999999;
                            minNeighbor_x(useP[idx][0]-i, useP[idx][1]) = useP[idx][0]-i;
                            minNeighbor_y(useP[idx][0]-i, useP[idx][1]) = useP[idx][1]+moveDirection;
                        }
                        minCost(useP[idx][0]-i, useP[idx][1]) = d;
                    }
                }
            }
            // Find minimum path
            int min_end = 1;
            for(int i = 2; i<block_size; i++)
            {
                // Find minimum cost
                if(minCost(useP[useP.size()-1][0]-i,useP[useP.size()-1][1]) < minCost(useP[useP.size()-1][0]-min_end,useP[useP.size()-1][1]))
                {
                    min_end = i;
                }
            }
            double cost = minCost(useP[useP.size()-1][0]-min_end,useP[useP.size()-1][1]) / useP.size();
            if(cost<average_cost)
            {
                for(int i = 0; i<useP.size(); i++)
                {
                    for(int j = 0; j<min_end ; j++)
                    {
                        img.setPixel(useP[useP.size()-1-i][0]-j, useP[useP.size()-1-i][1], skinColor);
                        //img.setPixel(useP[useP.size()-1-i][0]+j, useP[useP.size()-1-i][1], img.pixel(useP[useP.size()-1-i][0]-j, useP[useP.size()-1-i][1]));
                    }
                    // Next minimum neighbor
                    min_end = useP[useP.size()-1-i-1][0] - minNeighbor_x(useP[useP.size()-1-i][0]-min_end, useP[useP.size()-1-i][1]);
                }
            }
            else
            {
                for(size_t i = 0; i<useP.size(); i++)
                {
                    linear_interpolation(useP[i],1);
                }
            }
        }
        else
        {
            // Right
            for(int idx = 0; idx<useP.size(); idx++)
            {
                for(int i = 1; i<block_size; i++)
                {
                    QRgb right_pixel = img.pixel(useP[idx][0]+i,useP[idx][1]);
                    QRgb patch_pixel = patch.pixel(useP[idx][0]+i,useP[idx][1]);
                    double d = colorDistance(patch_pixel, right_pixel);
                    if(idx==0)
                    {
                        if(d==0)
                        {
                            d = 1;
                        }
                        minCost(useP[idx][0]+i, useP[idx][1]) = d;
                    }
                    else
                    {
                        double cost1 = minCost(useP[idx][0]+i-1, useP[idx][1]+moveDirection);
                        double cost2 = minCost(useP[idx][0]+i  , useP[idx][1]+moveDirection);
                        double cost3 = minCost(useP[idx][0]+i+1, useP[idx][1]+moveDirection);
                        if(cost1<=cost2 && cost1<=cost3 && cost1!=0)
                        {
                            d += cost1;
                            minNeighbor_x(useP[idx][0]+i, useP[idx][1]) = useP[idx][0]+i-1;
                            minNeighbor_y(useP[idx][0]+i, useP[idx][1]) = useP[idx][1]+moveDirection;
                        }
                        else if(cost2<=cost1 && cost2<=cost3 && cost2!=0)
                        {
                            d += cost2;
                            minNeighbor_x(useP[idx][0]+i, useP[idx][1]) = useP[idx][0]+i;
                            minNeighbor_y(useP[idx][0]+i, useP[idx][1]) = useP[idx][1]+moveDirection;
                        }
                        else if(cost3<=cost1 && cost3<=cost2 && cost3!=0)
                        {
                            d += cost3;
                            minNeighbor_x(useP[idx][0]+i, useP[idx][1]) = useP[idx][0]+i+1;
                            minNeighbor_y(useP[idx][0]+i, useP[idx][1]) = useP[idx][1]+moveDirection;
                        }
                        else
                        {
                            // No neighbors
                            d += 999999999;
                            minNeighbor_x(useP[idx][0]+i, useP[idx][1]) = useP[idx][0]+i;
                            minNeighbor_y(useP[idx][0]+i, useP[idx][1]) = useP[idx][1]+moveDirection;
                        }
                        minCost(useP[idx][0]+i, useP[idx][1]) = d;
                    }
                }
            }
            // Find minimum path
            int min_end = 1;
            for(int i = 2; i<block_size; i++)
            {
                // Find minimum cost
                if(minCost(useP[useP.size()-1][0]+i,useP[useP.size()-1][1]) < minCost(useP[useP.size()-1][0]+min_end,useP[useP.size()-1][1]))
                {
                    min_end = i;
                }
            }
            double cost = minCost(useP[useP.size()-1][0]+min_end,useP[useP.size()-1][1]) / useP.size();
            if(cost<average_cost)
            {
                for(int i = 0; i<useP.size(); i++)
                {
                    for(int j = 0; j<min_end ; j++)
                    {
                        img.setPixel(useP[useP.size()-1-i][0]+j, useP[useP.size()-1-i][1], skinColor);
                    }
                    // Next minimum neighbor
                    min_end = minNeighbor_x(useP[useP.size()-1-i][0]+min_end, useP[useP.size()-1-i][1]) - useP[useP.size()-1-i-1][0];
                }
            }
            else
            {
                for(size_t i = 0; i<useP.size(); i++)
                {
                    linear_interpolation(useP[i],1);
                }
            }
        }
    }
    else if(dir==2)
    {
        QRgb up = img.pixel(useP[0][0],useP[0][1]+block_size);
        QRgb down = img.pixel(useP[0][0],useP[0][1]-block_size);
        double err1 = colorDistance(skinColor, up);
        double err2 = colorDistance(skinColor, down);
        // left or right move ?
        int moveDirection;
        if(useP[1][0]-useP[0][0]>0)
        {
            // right move (previous x)
            moveDirection = -1;
        }
        else
        {
            // left move (previous x)
            moveDirection = 1;
        }
        // Determine down or up case
        if(err1<err2)
        {
            // Down
            for(int idx = 0; idx<useP.size(); idx++)
            {
                for(int i = 1; i<block_size; i++)
                {
                    QRgb down_pixel = img.pixel(useP[idx][0],useP[idx][1]-i);
                    QRgb patch_pixel = patch.pixel(useP[idx][0],useP[idx][1]-i);
                    double d = colorDistance(patch_pixel, down_pixel);
                    if(idx==0)
                    {
                        if(d==0)
                        {
                            d = 1;
                        }
                        minCost(useP[idx][0], useP[idx][1]-i) = d;
                    }
                    else
                    {
                        double cost1 = minCost(useP[idx][0]+moveDirection, useP[idx][1]-i-1);
                        double cost2 = minCost(useP[idx][0]+moveDirection, useP[idx][1]-i);
                        double cost3 = minCost(useP[idx][0]+moveDirection, useP[idx][1]-i+1);
                        if(cost1<=cost2 && cost1<=cost3 && cost1!=0)
                        {
                            d += cost1;
                            minNeighbor_x(useP[idx][0], useP[idx][1]-i) = useP[idx][0]+moveDirection;
                            minNeighbor_y(useP[idx][0], useP[idx][1]-i) = useP[idx][1]-i-1;
                        }
                        else if(cost2<=cost1 && cost2<=cost3 && cost2!=0)
                        {
                            d += cost2;
                            minNeighbor_x(useP[idx][0], useP[idx][1]-i) = useP[idx][0]+moveDirection;
                            minNeighbor_y(useP[idx][0], useP[idx][1]-i) = useP[idx][1]-i;
                        }
                        else if(cost3<=cost1 && cost3<=cost2 && cost3!=0)
                        {
                            d += cost3;
                            minNeighbor_x(useP[idx][0], useP[idx][1]-i) = useP[idx][0]+moveDirection;
                            minNeighbor_y(useP[idx][0], useP[idx][1]-i) = useP[idx][1]-i+1;
                        }
                        else
                        {
                            // No neighbors
                            d += 999999999;
                            minNeighbor_x(useP[idx][0], useP[idx][1]-i) = useP[idx][0]+moveDirection;
                            minNeighbor_y(useP[idx][0], useP[idx][1]-i) = useP[idx][1]-i;
                        }
                        minCost(useP[idx][0], useP[idx][1]-i) = d;
                    }
                }
            }
            // Find minimum path
            int min_end = 1;
            for(int i = 2; i<block_size; i++)
            {
                // Find minimum cost
                if(minCost(useP[useP.size()-1][0],useP[useP.size()-1][1]-i) < minCost(useP[useP.size()-1][0],useP[useP.size()-1][1]-min_end))
                {
                    min_end = i;
                }
            }
            double cost = minCost(useP[useP.size()-1][0],useP[useP.size()-1][1]-min_end) / useP.size();
            if(cost<average_cost)
            {
                for(int i = 0; i<useP.size(); i++)
                {
                    for(int j = 0; j<min_end ; j++)
                    {
                        img.setPixel(useP[useP.size()-1-i][0], useP[useP.size()-1-i][1]-j, skinColor);
                    }
                    // Next minimum neighbor
                    min_end = useP[useP.size()-1-i-1][1] - minNeighbor_y(useP[useP.size()-1-i][0], useP[useP.size()-1-i][1]-min_end);
                }
            }
            else
            {
                for(size_t i = 0; i<useP.size(); i++)
                {
                    linear_interpolation(useP[i],2);
                }
            }
        }
        else
        {
            // Up
            for(int idx = 0; idx<useP.size(); idx++)
            {
                for(int i = 1; i<block_size; i++)
                {
                    QRgb up_pixel = img.pixel(useP[idx][0],useP[idx][1]+i);
                    QRgb patch_pixel = patch.pixel(useP[idx][0],useP[idx][1]+i);
                    double d = colorDistance(patch_pixel, up_pixel);
                    if(idx==0)
                    {
                        if(d==0)
                        {
                            d = 1;
                        }
                        minCost(useP[idx][0], useP[idx][1]+i) = d;
                    }
                    else
                    {
                        double cost1 = minCost(useP[idx][0]+moveDirection, useP[idx][1]+i-1);
                        double cost2 = minCost(useP[idx][0]+moveDirection, useP[idx][1]+i);
                        double cost3 = minCost(useP[idx][0]+moveDirection, useP[idx][1]+i+1);
                        if(cost1<=cost2 && cost1<=cost3 && cost1!=0)
                        {
                            d += cost1;
                            minNeighbor_x(useP[idx][0], useP[idx][1]+i) = useP[idx][0]+moveDirection;
                            minNeighbor_y(useP[idx][0], useP[idx][1]+i) = useP[idx][1]+i-1;
                        }
                        else if(cost2<=cost1 && cost2<=cost3 && cost2!=0)
                        {
                            d += cost2;
                            minNeighbor_x(useP[idx][0], useP[idx][1]+i) = useP[idx][0]+moveDirection;
                            minNeighbor_y(useP[idx][0], useP[idx][1]+i) = useP[idx][1]+i;
                        }
                        else if(cost3<=cost1 && cost3<=cost2 && cost3!=0)
                        {
                            d += cost3;
                            minNeighbor_x(useP[idx][0], useP[idx][1]+i) = useP[idx][0]+moveDirection;
                            minNeighbor_y(useP[idx][0], useP[idx][1]+i) = useP[idx][1]+i+1;
                        }
                        else
                        {
                            // No neighbors
                            d += 999999999;
                            minNeighbor_x(useP[idx][0], useP[idx][1]+i) = useP[idx][0]+moveDirection;
                            minNeighbor_y(useP[idx][0], useP[idx][1]+i) = useP[idx][1]+i;
                        }
                        minCost(useP[idx][0], useP[idx][1]+i) = d;
                    }
                }
            }
            // Find minimum path
            int min_end = 1;
            for(int i = 2; i<block_size; i++)
            {
                // Find minimum cost
                if(minCost(useP[useP.size()-1][0],useP[useP.size()-1][1]+i) < minCost(useP[useP.size()-1][0],useP[useP.size()-1][1]+min_end))
                {
                    min_end = i;
                }
            }
            double cost = minCost(useP[useP.size()-1][0],useP[useP.size()-1][1]+min_end) / useP.size();
            if(cost<average_cost)
            {
                for(int i = 0; i<useP.size(); i++)
                {
                    for(int j = 0; j<min_end ; j++)
                    {
                        img.setPixel(useP[useP.size()-1-i][0], useP[useP.size()-1-i][1]+j, skinColor);
                    }
                    // Next minimum neighbor
                    min_end = minNeighbor_y(useP[useP.size()-1-i][0], useP[useP.size()-1-i][1]+min_end) - useP[useP.size()-1-i-1][1];
                }
            }
            else
            {
                for(size_t i = 0; i<useP.size(); i++)
                {
                    linear_interpolation(useP[i],2);
                }
            }
        }
    }
}

double FaceTex::colorDistance(QRgb p1, QRgb p2)
{
    double dis = sqrt(pow(qRed(p1)-qRed(p2),2) + pow(qGreen(p1)-qGreen(p2),2) + pow(qBlue(p1)-qBlue(p2),2));
    return dis;
}

void FaceTex::linear_interpolation(BaseMesh::Point p, int dir)
{
    int size = 10;
    // dir = 1 left&right || dir = 2 up&down
    if(dir==1)
    {
        if(p[0]-size<0 || p[0]+size>img.width())
        {
            return;
        }
        QRgb mid = img.pixel(p[0],p[1]);
        QRgb right = img.pixel(p[0]+size,p[1]);
        QRgb left = img.pixel(p[0]-size,p[1]);
        if(colorDistance(mid,skinColor)==0)
        {
            if(colorDistance(right,skinColor)==0)
            {
                img.setPixel(p[0],p[1],img.pixel(p[0]-1,p[1]));
                mid = img.pixel(p[0],p[1]);
            }
            else
            {
                img.setPixel(p[0],p[1],img.pixel(p[0]+1,p[1]));
                mid = img.pixel(p[0],p[1]);
            }
        }
        for(int i = 1; i<size; i++)
        {
            QRgb inter_right = qRgb(qRed(mid)*(size-i)/size + qRed(right)*i/size, qGreen(mid)*(size-i)/size + qGreen(right)*i/size, qBlue(mid)*(size-i)/size + qBlue(right)*i/size);
            img.setPixel(p[0]+i,p[1],inter_right);
            QRgb inter_left = qRgb(qRed(mid)*(size-i)/size + qRed(left)*i/size, qGreen(mid)*(size-i)/size + qGreen(left)*i/size, qBlue(mid)*(size-i)/size + qBlue(left)*i/size);
            img.setPixel(p[0]-i,p[1],inter_left);
        }
    }
    else if (dir==2)
    {
        if(p[1]-size<0 || p[1]+size>img.height())
        {
            return;
        }
        QRgb mid = img.pixel(p[0],p[1]);
        QRgb up = img.pixel(p[0],p[1]+size);
        QRgb down = img.pixel(p[0],p[1]-size);
        if(colorDistance(mid,skinColor)==0)
        {
            if(colorDistance(up,skinColor)==0)
            {
                img.setPixel(p[0],p[1],img.pixel(p[0],p[1]-1));
                mid = img.pixel(p[0],p[1]);
            }
            else
            {
                img.setPixel(p[0],p[1],img.pixel(p[0],p[1]+1));
                mid = img.pixel(p[0],p[1]);
            }
        }
        for(int i = 1; i<size; i++)
        {
            QRgb inter_up = qRgb(qRed(mid)*(size-i)/size + qRed(up)*i/size, qGreen(mid)*(size-i)/size + qGreen(up)*i/size, qBlue(mid)*(size-i)/size + qBlue(up)*i/size);
            img.setPixel(p[0],p[1]+i,inter_up);
            QRgb inter_down = qRgb(qRed(mid)*(size-i)/size + qRed(down)*i/size, qGreen(mid)*(size-i)/size + qGreen(down)*i/size, qBlue(mid)*(size-i)/size + qBlue(down)*i/size);
            img.setPixel(p[0],p[1]-i,inter_down);
        }
    }
}

void FaceTex::catchSkin(Mat pDst, Mat pSkinSrc)
{
    cvtColor(pDst, pDst, cv::COLOR_BGR2RGB);
    cvtColor(pSkinSrc, pSkinSrc, cv::COLOR_BGR2RGB);

    cvtColor(pDst, pDst, cv::COLOR_BGR2YCrCb);
    cvtColor(pSkinSrc, pSkinSrc, cv::COLOR_BGR2YCrCb);

    for (int i = pSkinSrc.rows*3/4; i<pSkinSrc.rows; i++)
    {
        for (int j = 0; j<pSkinSrc.cols; j++)
        {
            double Y = pDst.at<cv::Vec3b>(i, j)[0];
            double Cr = pDst.at<cv::Vec3b>(i, j)[1];
            double Cb = pDst.at<cv::Vec3b>(i, j)[2];

            if (133<Cr && Cr<170 && 12<Cb && Cb<135 && 80<Y)
            {
            }
            else
            {
                pDst.at<cv::Vec3b>(i, j)[0] = (pSkinSrc.at<cv::Vec3b>(i, j)[0]+pDst.at<cv::Vec3b>(i, j)[0])/2.0;
                pDst.at<cv::Vec3b>(i, j)[1] = (pSkinSrc.at<cv::Vec3b>(i, j)[1]+pDst.at<cv::Vec3b>(i, j)[1])/2.0;
                pDst.at<cv::Vec3b>(i, j)[2] = (pSkinSrc.at<cv::Vec3b>(i, j)[2]+pDst.at<cv::Vec3b>(i, j)[2])/2.0;
            }
        }
    }
    cvtColor(pDst, pDst, cv::COLOR_YCrCb2BGR);
    cvtColor(pDst, pDst, cv::COLOR_RGB2BGR);
}

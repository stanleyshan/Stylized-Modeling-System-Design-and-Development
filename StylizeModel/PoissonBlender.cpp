#include "PoissonBlender.h"

#include <QDebug>
// constructor
PoissonBlender::PoissonBlender()
{

}

PoissonBlender::PoissonBlender(const cv::Mat &src, const cv::Mat &target, const cv::Mat &mask=cv::Mat())
    : _src(src),
      _target(target),
      _mask(mask),
      count(0)
{
    CV_Assert(_mask.channels()==1);
    CV_Assert(_src.cols==_mask.cols && _src.rows==_mask.rows);
}

PoissonBlender::~PoissonBlender()
{

}

// set source, tareget and destination images
bool PoissonBlender::setImages(const cv::Mat &src, const cv::Mat &target, const cv::Mat &mask=cv::Mat())
{
    _src = src;
    _target = target;
    _mask = mask;

    CV_Assert(_mask.channels()==1);
    CV_Assert(_src.cols==_mask.cols && _src.rows==_mask.rows);

    return true;
}

void PoissonBlender::copyTo(PoissonBlender &b) const
{
    b.setImages(_src, _target, _mask);
}

inline PoissonBlender PoissonBlender::clone() const
{
    PoissonBlender b;
    copyTo(b);
    return b;
}

bool PoissonBlender::seamlessClone(cv::Mat &_dst, const int offx, const int offy, const bool mix=false)
{
    Mat originTarget = _target.clone();
    _target = colorTransfer(_src, _target);

    QImage newTargetImage = QImage(_target.data, _target.cols, _target.rows, _target.step, QImage::Format_RGB888).copy();
    qDebug() << newTargetImage.save(Plugin::m_appFileDirectory+"/target1.png");
    // Recover the color of hair
    catchHair(_target, originTarget);

    newTargetImage = QImage(_target.data, _target.cols, _target.rows, _target.step, QImage::Format_RGB888).copy();
    qDebug() << newTargetImage.save(Plugin::m_appFileDirectory+"/target.png");

    ch = _target.channels();
    cv::Point offset(offx, offy);
    cv::Point tl(_mask.size()), br(-1,-1);

    // calc bounding box
    for(int y=0; y<_mask.rows; ++y)
    {
        uchar *p = _mask.ptr(y);
        for(int x=0; x<_mask.cols; ++x,++p)
        {
            if(*p==0) continue;
            if(tl.x>x) tl.x=x;
            if(tl.y>y) tl.y=y;
            if(br.x<x) br.x=x;
            if(br.y<y) br.y=y;
        }
    }
    br.x += 1;
    br.y += 1;

    // add borders
    cv::Rect mask_roi2(tl-cv::Point(2,2), br+cv::Point(2,2));
    cv::Mat _srcUp, _targetUp, _maskUp, _dstUp;
    cv::copyMakeBorder(_src, _srcUp, 2,2,2,2, cv::BORDER_REPLICATE);
    cv::copyMakeBorder(_target, _targetUp, 2,2,2,2, cv::BORDER_REPLICATE);
    cv::copyMakeBorder(_mask, _maskUp, 1,1,1,1, cv::BORDER_CONSTANT);

    // allocate destination image
    _dstUp = _targetUp.clone();
    _dst = cv::Mat(_dstUp, cv::Rect(2,2,_dstUp.cols-2, _dstUp.rows-2));

    mask_roi1 = cv::Rect(tl-cv::Point(1,1), br+cv::Point(1,1));
    mask1 = cv::Mat(_mask, mask_roi1);
    target1 = cv::Mat(_targetUp, mask_roi1+offset-cv::Point(1,1));
    dst1 = cv::Mat(_dstUp, mask_roi1+offset-cv::Point(1,1));
    cv::Mat src(_srcUp, mask_roi2);
    cv::Mat target(_targetUp, mask_roi2+offset-cv::Point(2,2));
    cv::Mat dst(_dstUp, mask_roi2+offset-cv::Point(2,2));
    CV_Assert(src.cols==dst.cols && src.rows==dst.rows);

    // calc differential image
    cv::Mat src64, target64;
    int pw = mask_roi2.width-1, ph = mask_roi2.height-1;
    src.convertTo(src64, CV_64F);
    target.convertTo(target64, CV_64F);
    cv::Rect roi00(0,0,pw,ph), roi10(1,0,pw,ph), roi01(0,1,pw,ph);
    cv::Mat _src64_00(src64, roi00), _target64_00(target64, roi00);
    cv::Mat src_dx = cv::Mat(src64, roi10) - _src64_00;
    cv::Mat src_dy = cv::Mat(src64, roi01) - _src64_00;
    cv::Mat target_dx = cv::Mat(target64,roi10) - _target64_00;
    cv::Mat target_dy = cv::Mat(target64,roi01) - _target64_00;

    // gradient mixture
    cv::Mat Dx, Dy;
    // with gradient mixture
    if(mix)
    {
        cv::Mat *pdx_src = new cv::Mat[ch];
        cv::Mat *pdy_src = new cv::Mat[ch];
        cv::Mat *pdx_target = new cv::Mat[ch];
        cv::Mat *pdy_target = new cv::Mat[ch];
        cv::split(src_dx, pdx_src);
        cv::split(src_dy, pdy_src);
        cv::split(target_dx, pdx_target);
        cv::split(target_dy, pdy_target);

        cv::Mat _masks_dx, _masks_dy;
        for(int i=0; i<ch; i++)
        {
          _masks_dx = cv::abs(pdx_src[i]) < cv::abs(pdx_target[i]);
          _masks_dy = cv::abs(pdy_src[i]) < cv::abs(pdy_target[i]);
          pdx_target[i].copyTo(pdx_src[i], _masks_dx);
          pdy_target[i].copyTo(pdy_src[i], _masks_dy);
        }
        cv::merge(pdx_src, ch, Dx);
        cv::merge(pdy_src, ch, Dy);
    }
    // without gradient mixture
    else
    {
        Dx = src_dx;
        Dy = src_dy;
    }

    // lapilacian
    int w = pw-1, h = ph-1;
    drvxy = cv::Mat(Dx,cv::Rect(1,0,w,h)) - cv::Mat(Dx,cv::Rect(0,0,w,h))
    + cv::Mat(Dy,cv::Rect(0,1,w,h)) - cv::Mat(Dy,cv::Rect(0,0,w,h));

    //
    // solve an poisson's equation
    //
    Eigen::SparseMatrix<double> A;
    Eigen::VectorXd b;
    Eigen::VectorXd u;

    // build right-hand and left-hand matrix
    buildMatrix<double>(A, b, u);

    // solve sparse linear system
    solve<double>(A, b, u);

    // copy computed result to destination image
    copyResult<double>(u);

    return true;
}

// build matrix as linear system
template <typename T>
bool PoissonBlender::buildMatrix(Eigen::SparseMatrix<T> &A, Eigen::Matrix<T, Eigen::Dynamic,1> &b,
                                 Eigen::Matrix<T, Eigen::Dynamic, 1> &u)
{
    int w = mask_roi1.width;
    int h = mask_roi1.height;
    int nz=0;
    for(int y=0; y<h-1; ++y)
    {
        uchar *p = mask1.ptr(y);
        for(int x=0; x<w-1; ++x, ++p)
        {
            if(*p==0) continue;

            int id = y*(w*ch)+(x*ch);
            mp[id] = nz++;   // r
            mp[++id] = nz++; // g
            mp[++id] = nz++; // b
        }
    }

    A = Eigen::SparseMatrix<double>(nz, nz);
    b = Eigen::VectorXd(nz);
    u = Eigen::VectorXd(nz);
    int rowA = 0;

    A.reserve(5*nz);
    for(int y=1; y<h-1; ++y)
    {
        uchar *p = mask1.ptr(y)+1;
        cv::Vec3d *drv = drvxy.ptr<cv::Vec3d>(y)+1;
        for(int x=1; x<w-1; ++x, ++p, ++drv)
        {
            if(*p==0) continue;

            int id = y*(w*ch)+(x*ch);
            int tidx=id-ch*w, lidx=id-ch, ridx=id+ch, bidx=id+ch*w;

            // to omtimize insertion
            uchar tlrb = 15; // 0b1111
            if(mask1.at<uchar>(y-1,x)==0)
            {
                *drv -= target1.at<cv::Vec3b>(y-1,x);
                tlrb &= 7; //0b0111
            }
            if(mask1.at<uchar>(y,x-1)==0)
            {
                *drv -= target1.at<cv::Vec3b>(y,x-1);
                tlrb &= 11; //0b1011
            }
            if(mask1.at<uchar>(y,x+1)==0) {
            *drv -= target1.at<cv::Vec3b>(y,x+1);
            tlrb &= 13; //0b1101
            }
            if(mask1.at<uchar>(y+1,x)==0)
            {
                *drv -= target1.at<cv::Vec3b>(y+1,x);
                tlrb &= 14; //0b1110
            }
            for(int k=0; k<ch; ++k)
            {
                A.startVec(rowA+k);
                if(tlrb&8) A.insertBack(mp[tidx+k], rowA+k) = 1.0; // top
                if(tlrb&4) A.insertBack(mp[lidx+k], rowA+k) = 1.0; // left
                A.insertBack(mp[id  +k], rowA+k) = -4.0;// center
                if(tlrb&2) A.insertBack(mp[ridx+k], rowA+k) = 1.0; // right
                if(tlrb&1) A.insertBack(mp[bidx+k], rowA+k) = 1.0; // bottom
            }
            b(rowA+0) = cv::saturate_cast<double>((*drv)[0]);
            b(rowA+1) = cv::saturate_cast<double>((*drv)[1]);
            b(rowA+2) = cv::saturate_cast<double>((*drv)[2]);
            rowA+=ch;
        }
    }
    A.finalize();
    CV_Assert(nz==rowA);

    return true;
}

// solver sparse linear system
template <typename T>
bool PoissonBlender::solve(const Eigen::SparseMatrix<T> &A, const Eigen::Matrix<T, Eigen::Dynamic, 1> &b,
                           Eigen::Matrix<T, Eigen::Dynamic, 1> &u)
{
    Eigen::SparseLU<Eigen::SparseMatrix<T>> solver;

    solver.analyzePattern(A);
    solver.factorize(A);
    u = solver.solve(b);

    return true;
}

template <typename T>
bool PoissonBlender::copyResult(Eigen::Matrix<T, Eigen::Dynamic, 1> &u)
{
    int w = mask_roi1.width;
    int h = mask_roi1.height;
    for(int y=1; y<h-1; ++y)
    {
        uchar *pd = dst1.ptr(y);
        uchar *pm = mask1.ptr(y)+1;
        for(int x=1; x<w-1; ++x, ++pm)
        {
            if(*pm==0)
            {
                pd += 3;
            }
            else
            {
                int idx = mp[y*(w*ch)+(x*ch)];
                *pd++ = cv::saturate_cast<uchar>(u[idx+0]);
                *pd++ = cv::saturate_cast<uchar>(u[idx+1]);
                *pd++ = cv::saturate_cast<uchar>(u[idx+2]);
            }
        }
    }

    return true;
}

Mat PoissonBlender::colorTransfer(Mat pSrc, Mat pDst)
{
    Mat source_img_cie,
        target_img_cie;

    cvtColor(pSrc, source_img_cie, CV_BGR2Lab );
    cvtColor(pDst, target_img_cie, CV_BGR2Lab );

    /* Split into individual l a b channels */
    vector<Mat> source_channels,
                target_channels;

    split( source_img_cie, source_channels );
    split( target_img_cie, target_channels );

    /* For each of the l, a, b, channel ... */
    for( int i = 0; i < 3; i++ )
    {
        /* ... find the mean and standard deviations */
        /* ... for source image ... */
        Mat temp_mean, temp_stddev;
//        meanStdDev(source_channels[i], temp_mean, temp_stddev);
//        double source_mean     = temp_mean.at<double>(0);
//        double source_stddev   = temp_stddev.at<double>(0);
        double source_mean     = calcMean(pSrc, source_channels[i]);
        double source_stddev   = calcStdDev(pSrc, source_channels[i], source_mean);

        /* ... and for target image */
//        meanStdDev(target_channels[i], temp_mean, temp_stddev);
//        double target_mean     = temp_mean.at<double>(0);
//        double target_stddev   = temp_stddev.at<double>(0);
        double target_mean     = calcMean(pDst, target_channels[i]);
        double target_stddev   = calcStdDev(pDst, target_channels[i], target_mean);

        /* Fit the color distribution from target LAB to our source LAB */
        target_channels[i].convertTo( target_channels[i], CV_64FC1 );
        target_channels[i] -= target_mean;
        target_channels[i] *= (target_stddev / source_stddev);
        target_channels[i] += source_mean;
        target_channels[i].convertTo( target_channels[i], CV_8UC1 );
    }
    /* Merge the lab channels back into a single BGR image */
    Mat output_img;
    merge(target_channels, output_img);
    cvtColor(output_img, output_img, CV_Lab2BGR );
    return output_img;
}
void PoissonBlender::catchHair(Mat pDst, Mat pSrc)
{
    for (int i = 0; i<pSrc.rows; i++)
    {
        for (int j = 0; j<pSrc.cols; j++)
        {
            double B = (pSrc.at<cv::Vec3b>(i, j)[0]+
                        pSrc.at<cv::Vec3b>(i, j)[1]+
                        pSrc.at<cv::Vec3b>(i, j)[2])/3;
            if(B<70)
            {
                pDst.at<cv::Vec3b>(i, j)[0] = pSrc.at<cv::Vec3b>(i, j)[0];
                pDst.at<cv::Vec3b>(i, j)[1] = pSrc.at<cv::Vec3b>(i, j)[1];
                pDst.at<cv::Vec3b>(i, j)[2] = pSrc.at<cv::Vec3b>(i, j)[2];
            }
        }
    }
}

double PoissonBlender::calcMean(Mat pSkinSrc, Mat pSrc)
{
    qDebug() << "PoissonBlender::calcMean()";
    cvtColor(pSkinSrc, pSkinSrc, cv::COLOR_BGR2RGB);
    cvtColor(pSkinSrc, pSkinSrc, cv::COLOR_BGR2YCrCb);

//    Mat tmp = pSkinSrc.clone();

    double mean = 0.0;
    for(int i=0; i<pSrc.rows; i++)
    {
        for(int j=0; j<pSrc.cols; j++)
        {
            double Y = pSkinSrc.at<cv::Vec3b>(i, j)[0];
            double Cr = pSkinSrc.at<cv::Vec3b>(i, j)[1];
            double Cb = pSkinSrc.at<cv::Vec3b>(i, j)[2];
            if (133<Cr && Cr<170 && 12<Cb && Cb<135 && 80<Y)
            {
//                tmp.at<cv::Vec3b>(i, j)[0] = 255;
//                tmp.at<cv::Vec3b>(i, j)[1] = 255;
//                tmp.at<cv::Vec3b>(i, j)[2] = 255;
                mean += pSrc.at<unsigned char>(i,j);
            }
            else
            {
//                tmp.at<cv::Vec3b>(i, j)[0] = 0;
//                tmp.at<cv::Vec3b>(i, j)[1] = 0;
//                tmp.at<cv::Vec3b>(i, j)[2] = 0;
            }
        }
    }
    mean /= pSrc.rows*pSrc.cols;

//    QImage testImg = QImage(tmp.data, tmp.cols, tmp.rows, tmp.step, QImage::Format_RGB888).copy();
//    QString fileName = Plugin::m_appFileDirectory+"/mean"+QString::number(count)+".png";
//    qDebug() << testImg.save(fileName);
//    count++;

    cvtColor(pSkinSrc, pSkinSrc, cv::COLOR_YCrCb2BGR);
    cvtColor(pSkinSrc, pSkinSrc, cv::COLOR_RGB2BGR);
    return mean;
}

double PoissonBlender::calcStdDev(Mat pSkinSrc, Mat pSrc, double mean)
{
    qDebug() << "PoissonBlender::calcStdDev()";
    cvtColor(pSkinSrc, pSkinSrc, cv::COLOR_BGR2RGB);
    cvtColor(pSkinSrc, pSkinSrc, cv::COLOR_BGR2YCrCb);

    double stdDev = 0.0;
    for(int i=0; i<pSrc.rows; i++)
    {
        for(int j=0; j<pSrc.cols; j++)
        {
            double Y = pSkinSrc.at<cv::Vec3b>(i, j)[0];
            double Cr = pSkinSrc.at<cv::Vec3b>(i, j)[1];
            double Cb = pSkinSrc.at<cv::Vec3b>(i, j)[2];
            if (133<Cr && Cr<170 && 12<Cb && Cb<135 && 80<Y)
            {
                stdDev += pow(pSrc.at<unsigned char>(i, j)-mean, 2);
            }
        }
    }
    stdDev /= pSrc.rows*pSrc.cols;
    stdDev = sqrt(stdDev);

    cvtColor(pSkinSrc, pSkinSrc, cv::COLOR_YCrCb2BGR);
    cvtColor(pSkinSrc, pSkinSrc, cv::COLOR_RGB2BGR);
    return stdDev;
}

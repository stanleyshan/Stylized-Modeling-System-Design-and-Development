#ifndef POISSONBLENDER_H
#define POISSONBLENDER_H

#include "plugin.h"

#include <map>
#include <opencv2/imgproc/imgproc.hpp>
#include <Eigen/Core>
#include <Eigen/Sparse>
#include <QImage>

using namespace cv;

class PoissonBlender
{
public:
    PoissonBlender();
    PoissonBlender(const cv::Mat &src, const cv::Mat &target, const cv::Mat &mask);
    ~PoissonBlender();
    bool setImages(const cv::Mat &src, const cv::Mat &target, const cv::Mat &mask);
    void copyTo(PoissonBlender &b) const;
    PoissonBlender clone() const;
    bool seamlessClone(cv::Mat &dst, int offx, int offy, bool mix);
    Mat colorTransfer(Mat pSrc, Mat pDst);
    void catchHair(Mat pDst, Mat pSrc);
    double calcMean(Mat pSkinSrc, Mat pSrc);
    double calcStdDev(Mat pSkinSrc, Mat pSrc, double mean);

private:
    cv::Mat _src, _target, _mask;
    cv::Rect mask_roi1;
    cv::Mat mask1;
    cv::Mat dst1;
    cv::Mat target1;
    cv::Mat drvxy;

    int ch;
    int count;

    std::map<int,int> mp;

    template <typename T>
    bool buildMatrix(Eigen::SparseMatrix<T> &A, Eigen::Matrix<T, Eigen::Dynamic, 1> &b, Eigen::Matrix<T, Eigen::Dynamic, 1> &u);

    template <typename T>
    bool solve(const Eigen::SparseMatrix<T> &A, const Eigen::Matrix<T, Eigen::Dynamic, 1> &b, Eigen::Matrix<T, Eigen::Dynamic, 1> &u);

    template <typename T>
    bool copyResult(Eigen::Matrix<T, Eigen::Dynamic, 1> &u);
};

#endif // POISSONBLENDER_H

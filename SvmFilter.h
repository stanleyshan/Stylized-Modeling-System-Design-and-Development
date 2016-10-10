#ifndef SVMFILTER_H
#define SVMFILTER_H
#include "FFT.h"

#include <QJsonArray>
#include <QVector>
#include <math.h>


/*
 * Function name                    test
 * getResponses()                   OK
 * inplaceNormalizeFilterMatrix()   OK
 * logisticResponse()               OK
 * ifft()                           OK
 * complex_mult_inplace()           OK
 * normalizePatches()               OK
 * fft_inplace()                    OK
 * upperPowerOfTwo()                OK
*/

class SvmFilter
{

public:
    SvmFilter();
    QVector<QVector<double>>* getResponses(QVector<QVector<double>> *patches);
    void init(QJsonArray filter_input, QJsonArray bias_input, int numPatches, int filterWidth, int searchWidth);

private:
    FFT *_fft;
    QVector<QVector<QVector<double>>> fft_filters;
    QVector<double> biases;
    QVector<QVector<double>> *responses;
    int fft_size, filterLength, filter_width, search_width, num_patches;
    QVector<QVector<double>> temp_imag_part;
    QVector<double> temp_real_part;

    void inplaceNormalizeFilterMatrix(QVector<double> &response);
    QVector<double> logisticResponse(QVector<double> response);
    QVector<double> ifft(QVector<double> &rn, QVector<double> &cn);
    void complex_mult_inplace(QVector<QVector<double>> &cn1, QVector<QVector<double>> cn2);
    QVector<double> normalizePatches(QVector<double> patch);
    QVector<QVector<double>> fft_inplace(QVector<double> &array, QVector<double> _im_part);
    int upperPowerOfTwo(int x);
};

#endif // SVMFILTER_H

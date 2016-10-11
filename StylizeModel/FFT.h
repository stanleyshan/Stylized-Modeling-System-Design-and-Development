#ifndef FFT_H
#define FFT_H

#include <QVector>

class FFT
{

public:
    FFT();
    ~FFT();
    void init(int n);
    void fft1d(QVector<double> &_tre2, QVector<double> &_tim2);// 1D-FFT
    void ifft1d(QVector<double> &re, QVector<double> &im);// 1D-IFFT
    void fft2d(QVector<double> &re, QVector<double> &im);// 2D-FFT
    void ifft2d(QVector<double> &re, QVector<double> &im);// 2D-IFFT
    void real_ifft2d(QVector<double> &re, QVector<double> &im);// 2D-IFFT, real-valued, only outputs the real valued part
    void real_fft2d(QVector<double> &re, QVector<double> &im);

private:
    int _n;
    QVector<int> _bitrev;  // bit reversal table
    QVector<double> _cstb;    // sin/cos table
    QVector<double> _tre, _tim;

    void fft(QVector<double> &re, QVector<double> &im, double inv);
    void _setVariables();
    void _makeCosSinTable();
    void _makeBitReversal();

};

#endif // FFT_H

#include "FFT.h"

#include <QDebug>
#include <QtCore>

FFT::FFT()
{
    _n = 0;
}

FFT::~FFT()
{

}

void FFT::fft(QVector<double> &re,QVector<double> &im, double inv)
{
    int d, h, ik, m;
    double wr, wi, xi, xr;
    double tmp;
    int n4 = _n >> 2;
    // bit reversal
    for (int l = 0; l < _n; l++)
    {
        m = _bitrev[l];
        if (l < m)
        {
            tmp = re[l];
            re[l] = re[m];
            re[m] = tmp;
            tmp = im[l];
            im[l] = im[m];
            im[m] = tmp;
        }
    }
    // butterfly operation
    for (int k = 1; k < _n; k <<= 1)
    {
        h = 0;
        d = _n / (k << 1);
        for (int j = 0; j < k; j++)
        {
            wr = _cstb[h + n4];
            wi = inv * _cstb[h];
            for (int i = j; i < _n; i += (k << 1))
            {
                ik = i + k;
                xr = wr * re[ik] + wi * im[ik];
                xi = wr * im[ik] - wi * re[ik];
                re[ik] = re[i] - xr;
                re[i] += xr;
                im[ik] = im[i] - xi;
                im[i] += xi;
            }
            h += d;
        }
    }
}

void FFT::init(int n)
{
    if (n != 0 && (n & (n - 1)) == 0)
    {
        _n = n;
        _setVariables();
        _makeBitReversal();
        _makeCosSinTable();
    }
    else
    {
        qDebug() << "FFT: init: radix-2 required";
    }
}


void FFT::_setVariables()
{
    _bitrev = QVector<int>(_n);
    _cstb = QVector<double>((int) (_n * 1.25));
    _tre = QVector<double>(_n * _n);
    _tim = QVector<double>(_n * _n);
}

// 1D-FFT
void FFT:: fft1d(QVector<double> &_tre2, QVector<double> &_tim2)
{
    fft(_tre2, _tim2, 1.0);
}

// 1D-IFFT
void FFT::ifft1d(QVector<double>  &re, QVector<double> &im)
{
    double n = 1.0 / _n;
    fft(re, im, -1.0);
    for (int i = 0; i < _n; i++)
    {
        re[i] *= n;
        im[i] *= n;
    }
}

// 2D-FFT
void FFT::fft2d(QVector<double> &re, QVector<double> &im)
{
    int i;
    // x-axis
    for (int y = 0; y < _n; y++)
    {
        i = y * _n;
        for (int x1 = 0; x1 < _n; x1++)
        {
            _tre[x1] = re[x1 + i];
            _tim[x1] = im[x1 + i];
        }
        fft1d(_tre, _tim);
        for (int x2 = 0; x2 < _n; x2++)
        {
            re[x2 + i] = _tre[x2];
            im[x2 + i] = _tim[x2];
        }
    }

    // y-axis
    for (int x = 0; x < _n; x++)
    {
        for (int y1 = 0; y1 < _n; y1++)
        {
            i = x + y1 * _n;
            _tre[y1] = re[i];
            _tim[y1] = im[i];
        }
        fft1d(_tre, _tim);
        for (int y2 = 0; y2 < _n; y2++)
        {
            i = x + y2 * _n;
            re[i] = _tre[y2];
            im[i] = _tim[y2];
        }
    }
}

// 2D-IFFT
void FFT::ifft2d(QVector<double> &re, QVector<double> &im)
{
    int i;
    // x-axis
    for (int y = 0; y < _n; y++)
    {
        i = y * _n;
        for (int x1 = 0; x1 < _n; x1++)
        {
            _tre[x1] = re[x1 + i];
            _tim[x1] = im[x1 + i];
        }
        ifft1d(_tre, _tim);
        for (int x2 = 0; x2 < _n; x2++)
        {
            re[x2 + i] = _tre[x2];
            im[x2 + i] = _tim[x2];
        }
    }

    // y-axis
    for (int x = 0; x < _n; x++)
    {
        for (int y1 = 0; y1 < _n; y1++)
        {
            i = x + y1 * _n;
            _tre[y1] = re[i];
            _tim[y1] = im[i];
        }
        ifft1d(_tre, _tim);
        for (int y2 = 0; y2 < _n; y2++)
        {
            i = x + y2 * _n;
            re[i] = _tre[y2];
            im[i] = _tim[y2];
        }
    }
}

// 2D-IFFT, real-valued
// only outputs the real valued part
void FFT::real_ifft2d(QVector<double> &re, QVector<double> &im)
{
    int i2;
    int i;
    // x-axis
    for (int y = 0; y < _n; y++)
    {
        i = y * _n;
        for (int x1 = 0; x1 < _n; x1++)
        {
            _tre[x1] = re[x1 + i];
            _tim[x1] = im[x1 + i];
        }
        ifft1d(_tre, _tim);
        for (int x2 = 0; x2 < _n; x2++)
        {
            re[x2 + i] = _tre[x2];
            im[x2 + i] = _tim[x2];
        }
    }
    // y-axis
    int halfn = _n / 2;
    int rowIdx;
    for (int x = 0; x < _n; x += 2)
    {
        // untangle
        i = x;
        i2 = x + 1;
        _tre[0] = re[i];
        _tim[0] = re[i2];
        _tre[_n / 2] = re[(halfn * _n) + i];
        _tim[_n / 2] = re[(halfn * _n) + i2];
        for (int x2 = 1; x2 < halfn; x2++)
        {
            rowIdx = x2 * _n;
            _tre[x2] = re[rowIdx + i] - im[rowIdx + i2];
            _tre[_n - x2] = re[rowIdx + i] + im[rowIdx + i2];
            _tim[x2] = im[rowIdx + i] + re[rowIdx + i2];
            _tim[_n - x2] = re[rowIdx + i2] - im[rowIdx + i];
        }
        ifft1d(_tre, _tim);
        for (int y2 = 0; y2 < _n; y2++)
        {
            i = x + y2 * _n;
            i2 = (x + 1) + y2 * _n;
            re[i] = _tre[y2];
            re[i2] = _tim[y2];
        }
    }
}

void FFT::real_fft2d(QVector<double> &re, QVector<double> &im)
{
    int i, i2;
    // x-axis
    for (int y = 0; y < _n; y += 2)
    {
        i = y * _n;
        i2 = (y + 1) * _n;
        // tangle
        for (int x1 = 0; x1 < _n; x1++)
        {
            _tre[x1] = re[x1 + i];
            _tim[x1] = re[x1 + i2];
        }
        fft1d(_tre, _tim);
        // untangle
        re[i] = _tre[0];
        re[i2] = _tim[0];
        im[i] = 0;
        im[i2] = 0;
        re[_n / 2 + i] = _tre[_n / 2];
        re[_n / 2 + i2] = _tim[_n / 2];
        im[_n / 2 + i] = 0;
        im[_n / 2 + i2] = 0;
        for (int x2 = 1; x2 < (_n / 2); x2++)
        {
            re[x2 + i] = 0.5f * (_tre[x2] + _tre[_n - x2]);
            im[x2 + i] = 0.5f * (_tim[x2] - _tim[_n - x2]);
            re[x2 + i2] = 0.5f * (_tim[x2] + _tim[_n - x2]);
            im[x2 + i2] = -0.5f * (_tre[x2] - _tre[_n - x2]);
            re[(_n - x2) + i] = re[x2 + i];
            im[(_n - x2) + i] = -im[x2 + i];
            re[(_n - x2) + i2] = re[x2 + i2];
            im[(_n - x2) + i2] = -im[x2 + i2];
        }
    }
    // y-axis
    for (int x = 0; x < _n; x++)
    {
        for (int y1 = 0; y1 < _n; y1++)
        {
            i = x + y1 * _n;
            _tre[y1] = re[i];
            _tim[y1] = im[i];
        }
        fft1d(_tre, _tim);
        for (int y2 = 0; y2 < _n; y2++)
        {
            i = x + y2 * _n;
            re[i] = _tre[y2];
            im[i] = _tim[y2];
        }
    }
}


void FFT::_makeCosSinTable()
{
    int n2 = _n >> 1, n4 = _n >> 2, n8 = _n >> 3, n2p4 = n2 + n4;
    double t = sin(M_PI / _n);
    double dc = 2 * t * t, ds = sqrt(dc * (2 - dc));

    double c = _cstb[n4] = 1;
    double s = _cstb[0] = 0;
    t = 2 * dc;
    for (int i = 1; i < n8; i++)
    {
        c -= dc;
        dc += t * c;
        s += ds;
        ds -= t * s;
        _cstb[i] = s;
        _cstb[n4 - i] = c;
    }
    if (n8 != 0)
    {
        _cstb[n8] = sqrt(0.5);
    }
    for (int j = 0; j < n4; j++)
    {
        _cstb[n2 - j] = _cstb[j];
    }
    for (int k = 0; k < n2p4; k++)
    {
        _cstb[k + n2] = -_cstb[k];
    }
}

void FFT::_makeBitReversal()
{
    int i = 0, j = 0, k;
    _bitrev[0] = 0;
    while (++i < _n)
    {
        k = _n >> 1;
        while (k <= j)
        {
             j -= k;
             k >>= 1;
        }
        j += k;
        _bitrev[i] = j;
    }
}


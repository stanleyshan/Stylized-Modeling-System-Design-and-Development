#ifndef MosseFilter_H
#define MosseFilter_H
#include <Params.h>
#include <FFT.h>
#include <QJsonArray>
#include <QJsonObject>
#include <QBitmap>
#include <QVector>

/*
 * Function name    test
 * complex_div()    not use
 * complex_conj()   not use
 * fft()            not use
 * psr()            not use
 * ifft()           OK
 * complex_mult()   OK
 * fft_inplace()    OK
 * cosine_window()  OK
 * preprocess()     OK
*/

class MosseFilter
{
public:
    MosseFilter(Params *params);
    void load(QJsonObject filter);
    QVector<double> track(QImage element, double left, double top, double width, double height, bool updateFilter, bool gaussianPrior, bool calcPSR);

private:
    Params *m_params;
    QVector<QVector<double>> _filter;
    QVector<QVector<double>> _top;
    QVector<QVector<double>> _bottom;
    FFT *_fft;
    int _w, _h;
    QVector<double> _im_part;
    int _arrlen;
    QVector<double> _image_array;

    double psr_prev;
    double peak_prev;
    bool updateable;

    QVector<QVector<double>> complex_div(QVector<QVector<double>> cn1, QVector<QVector<double>> cn2);
    QVector<QVector<double>> complex_conj(QVector<QVector<double>> cn);
    QVector<QVector<double>> fft(QVector<double> array);
    double psr(QVector<double> array);
    QVector<double> ifft(QVector<double> &rn, QVector<double> &cn);
    QVector<QVector<double>> complex_mult(QVector<QVector<double>> cn1, QVector<QVector<double>> cn2);
    QVector<QVector<double>> fft_inplace(QVector<double> array);
    QVector<double> cosine_window(QVector<double> array);
    QVector<double> preprocess(QVector<double> array);

};

#endif // MosseFilter_H

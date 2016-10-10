#include "MosseFilter.h"

#include <QDebug>
#include <QtCore>

MosseFilter::MosseFilter(Params *params)
{
    _filter = QVector<QVector<double>>(2);
    _top = QVector<QVector<double>>(2);
    _bottom = QVector<QVector<double>>(2);
    updateable = false;

    if (params != NULL)
    {
        params = new Params();

        // setup of canvas for drawing responses, if given
        params->drawResponse = false;
        params->psrThreshold = 10;
        params->eta = 0.10;
        params->convertToGrayscale = true;
        m_params = params;
    }
}

void MosseFilter::load(QJsonObject filter)
{
    // initialize filter width and height
    _w = filter["width"].toInt();
    _h = filter["height"].toInt();
    _arrlen = _w * _h;
    _filter[0] = QVector<double>(filter["real"].toArray().size());
    _filter[1] = QVector<double>(filter["imag"].toArray().size());
    for (int i = 0; i < _filter[0].size(); i++)
        _filter[0][i] = filter["real"].toArray()[i].toDouble();
    for (int i = 0; i < _filter[1].size(); i++)
        _filter[1][i] = filter["imag"].toArray()[i].toDouble();

    // handling top and bottom when they're not presen
    if (filter["top"].toObject().size() != 0 && filter["bottom"].toObject().size() != 0)
    {
        updateable = true;

        _top[0] = QVector<double>(filter["top"].toObject()["real"].toArray().size());
        _top[1] = QVector<double>(filter["top"].toObject()["imag"].toArray().size());
        for (int i = 0; i < _top[0].size(); i++)
            _top[0][i] = filter["top"].toObject()["real"].toArray()[i].toDouble();
        for (int i = 0; i < _top[1].size(); i++)
            _top[1][i] = filter["top"].toObject()["imag"].toArray()[i].toDouble();

        _bottom[0] = QVector<double>(filter["bottom"].toObject()["real"].toArray().size());
        _bottom[1] = QVector<double>(filter["bottom"].toObject()["imag"].toArray().size());
        for (int i = 0; i < _bottom[0].size(); i++)
            _bottom[0][i] = filter["bottom"].toObject()["real"].toArray()[i].toDouble();
        for (int i = 0; i < _bottom[1].size(); i++)
            _bottom[1][i] = filter["bottom"].toObject()["imag"].toArray()[i].toDouble();
    }
    // initialize fft to given width
    _fft = new FFT();
    _fft->init(filter["width"].toInt());

    // set up temporary variables
    _im_part = QVector<double>(_arrlen);
    _image_array = QVector<double>(_arrlen);
}

QVector<double> MosseFilter::track(QImage element, double left, double top, double width, double height, bool updateFilter, bool gaussianPrior, bool calcPSR)
{
    // finds position of filter in input image
    if (_filter.size() == 0)
    {
        qDebug() << "Mosse-filter needs to be initialized or trained before starting tracking.";
        return QVector<double>();
    }

    //crop the image from the rect[left, top, width, height] and scale to 32x32
    QRect rect(round(left), round(top), round(width), round(height));
    QImage scaledImage = element.copy(rect);
    float scaleWidth = 32.0 / scaledImage.width();
    float scaleHeight = 32.0 / scaledImage.height();
    QTransform transform;
    transform.reset();
    transform.scale(scaleWidth, scaleHeight);
    scaledImage = scaledImage.transformed(transform);

    // copy color to one dimension array
    QVector<double> id = QVector<double>(_w * _h * 4);
    for (int i = 0; i < _w; i++)
    {
        for (int j = 0; j < _h; j++)
        {
            QColor color = QColor(scaledImage.pixel(i, j));
            id[i * 4 + j * _w * 4] = color.red();
            id[i * 4 + j * _w * 4 + 1] = color.green();
            id[i * 4 + j * _w * 4 + 2] = color.blue();
            id[i * 4 + j * _w * 4 + 3] = color.alpha();
        }
    }

    if (m_params->convertToGrayscale)
    {
        // convert to grayscale
        for (int i = 0; i < _arrlen; i++)
        {
            _image_array[i] = id[(4 * i)] * 0.3;
            _image_array[i] += id[(4 * i) + 1] * 0.59;
            _image_array[i] += id[(4 * i) + 2] * 0.11;
        }
    }
    else
    {
        // use only one channel
        for (int i = 0; i < _arrlen; i++)
        {
            _image_array[i] = id[(4 * i)];
        }
    }

    // preprocess
    QVector<double> prepImage = preprocess(_image_array);
    prepImage = cosine_window(prepImage);
    // filter
    QVector<QVector<double>> res = fft_inplace(prepImage);
    // elementwise multiplication with filter
    QVector<QVector<double>> nures = complex_mult(res, _filter);
    // do inverse 2d fft
    QVector<double> filtered = ifft(nures[0], nures[1]);
    // find max and min
    double max = 0;
    double min = 0;
    QVector<double> maxpos(2);

    // method using centered gaussian prior
    if (gaussianPrior)
    {
        double prior, dx, dy;
        int variance = 128;
        for (int x = 0; x < _w; x++)
        {
            for (int y = 0; y < _h; y++)
            {
                dx = x - _w / 2;
                dy = y - _h / 2;
                prior = exp(-0.5 * ((dx * dx) + (dy * dy)) / variance);
                if ((filtered[(y * _w) + x] * prior) > max)
                {
                    max = filtered[(y * _w) + x] * prior;
                    maxpos[0] = x;
                    maxpos[1] = y;
                }
                if (filtered[(y * _w) + x] < min)
                {
                    min = filtered[(y * _w) + x];
                }
            }
        }
    }
    else
    {
        for (int x = 0; x < _w; x++)
        {
            for (int y = 0; y < _h; y++)
            {
                if (filtered[(y * _w) + x] > max)
                {
                    max = filtered[(y * _w) + x];
                    maxpos[0] = x;
                    maxpos[1] = y;
                }
                if (filtered[(y * _w) + x] < min)
                {
                    min = filtered[(y * _w) + x];
                }
            }
        }
    }
    peak_prev = max;

    if (updateFilter)
    {
        if (!updateable)
        {
            qDebug() <<  "MosseFilter: The loaded filter does not support updating. Ignoring parameter 'updateFilter'.";
        }
        else
        {
            double psr;
            if (calcPSR)
            {
                psr = psr_prev;
            }
            else
            {
                psr = this->psr(filtered);
            }

            if (psr > m_params->psrThreshold)
            {
                // create target
                QVector<double> target(_arrlen);
                QVector<QVector<double>> target1;

                double nux = maxpos[0];
                double nuy = maxpos[1];
                for (int x = 0; x < _w; x++)
                {
                    for (int y = 0; y < _h; y++)
                    {
                        target[(y * _w) + x] = exp(-(((x - nux) * (x - nux)) + ((y - nuy) * (y - nuy))) / (2 * 2));
                    }
                }

                // fft target
                target1 = fft(target);

                // create filter
                QVector<QVector<double>> res_conj = complex_conj(res);
                QVector<QVector<double>> fuTop = complex_mult(target1, res_conj);
                QVector<QVector<double>> fuBottom = complex_mult(res, res_conj);

                // add up
                double eta = m_params->eta;
                for (int i = 0; i < _arrlen; i++)
                {
                    _top[0][i] = eta * fuTop[0][i] + (1 - eta) * _top[0][i];
                    _top[1][i] = eta * fuTop[1][i] + (1 - eta) * _top[1][i];
                    _bottom[0][i] = eta * fuBottom[0][i] + (1 - eta) * _bottom[0][i];
                    _bottom[1][i] = eta * fuBottom[1][i] + (1 - eta) * _bottom[1][i];
                }

                _filter = complex_div(_top, _bottom);
            }
        }
    }

    maxpos[0] = maxpos[0] * (width / _w);
    maxpos[1] = maxpos[1] * (width / _h);

    // check if output is strong enough
    // if not, return false?
    if (max < 0)
    {
        return QVector<double>();
    }
    else
    {
        return maxpos;
    }
}

QVector<QVector<double>> MosseFilter::complex_div(QVector<QVector<double>> cn1, QVector<QVector<double>> cn2)
{
    QVector<QVector<double>> nucn(2);
    nucn[0] = QVector<double>(_arrlen);
    nucn[1] = QVector<double>(_arrlen);

    for (int r = 0; r < _arrlen; r++)
    {
        nucn[0][r] = ((cn1[0][r] * cn2[0][r]) + (cn1[1][r] * cn2[1][r])) / ((cn2[0][r] * cn2[0][r]) + (cn2[1][r] * cn2[1][r]));
        nucn[1][r] = ((cn1[1][r] * cn2[0][r]) - (cn1[0][r] * cn2[1][r])) / ((cn2[0][r] * cn2[0][r]) + (cn2[1][r] * cn2[1][r]));
    }
    return nucn;
}

QVector<QVector<double>> MosseFilter::complex_conj(QVector<QVector<double>> cn)
{
    QVector<QVector<double>> nucn(2);
    for (int i = 0; i < _arrlen; i++)
    {
        nucn[0][i] = cn[0][i];
        nucn[1][i] = -cn[1][i];
    }
    return nucn;
}

QVector<QVector<double>> MosseFilter::fft(QVector<double> array)
{
    QVector<double> cn(_arrlen);
    for (int i = 0; i < _arrlen; i++)
    {
        cn[i] = 0.0;
    }

    _fft->fft2d(array, cn);

    QVector<QVector<double>> result(2);
    result[0] = array;
    result[1] = cn;
    return result;
}

double MosseFilter::psr(QVector<double> array)
{
    // proper
    double sum = 0;
    double max = 0;
    QVector<double> maxpos(2);
    double sdo = 0;
    double val;
    for (int x = 0; x < _w; x++)
    {
        for (int y = 0; y < _h; y++)
        {
            val = array[(y * _w) + x];
            sum += val;
            sdo += (val * val);
            if (max < val)
            {
                max = val;
                maxpos[0] = x;
                maxpos[1] = y;
            }
        }
    }

    // subtract values around peak
    for (int x = -5; x < 6; x++)
    {
        for (int y = -5; y < 6; y++)
        {
            if (sqrt(x * x + y * y) < 5)
            {
                if (maxpos.size() != 0)
                {
                    val = array[(int) (((maxpos[1] + y) * _w) + (maxpos[0] + x))];
                    sdo -= (val * val);
                    sum -= val;
                }
            }
        }
    }

    double mean = sum / array.size();
    double sd = sqrt((sdo / array.size()) - (mean * mean));

    // get mean/variance of output around peak
    return (max - mean) / sd;
}

QVector<double> MosseFilter::ifft(QVector<double> &rn, QVector<double> &cn)
{
    _fft->ifft2d(rn, cn);
    return rn;
}

QVector<QVector<double>> MosseFilter::complex_mult(QVector<QVector<double>> cn1, QVector<QVector<double>> cn2)
{
    QVector<double> re_part(_arrlen);
    QVector<double> im_part(_arrlen);
    QVector<QVector<double>> nucn(2);
    nucn[0] = re_part;
    nucn[1] = im_part;

    for (int r = 0; r < _arrlen; r++)
    {
        nucn[0][r] = (cn1[0][r] * cn2[0][r]) - (cn1[1][r] * cn2[1][r]);
        nucn[1][r] = (cn1[0][r] * cn2[1][r]) + (cn1[1][r] * cn2[0][r]);
    }
    return nucn;
}

QVector<QVector<double>> MosseFilter::fft_inplace(QVector<double> array)
{

    for (int i = 0; i < _arrlen; i++)
    {
        _im_part[i] = 0.0;
    }

    _fft->fft2d(array, _im_part);

    QVector<QVector<double>> result(2);
    result[0] = array;
    result[1] = _im_part;
    return result;
}

QVector<double> MosseFilter::cosine_window(QVector<double> array)
{
    int pos = 0;
    for (int i = 0; i < _w; i++)
    {
        for (int j = 0; j < _h; j++)
        {
            // pos = (i%_w)+(j*_w);
            double cww = sin((M_PI * i) / (_w - 1));
            double cwh = sin((M_PI * j) / (_h - 1));
            array[pos] = fmin(cww, cwh) * array[pos];
            pos++;
        }
    }
    return array;
}

QVector<double> MosseFilter::preprocess(QVector<double> array)
{
    // log adjusting
    for (int i = 0; i < _arrlen; i++)
    {
        array[i] = log(array[i] + 1);
    }
    // normalize to mean 0 and norm 1
    double mean = 0.0;
    for (int i = 0; i < _arrlen; i++)
    {
        mean += array[i];
    }
    mean /= _arrlen;
    for (int i = 0; i < _arrlen; i++)
    {
        array[i] -= mean;
    }
    double norm = 0.0;
    for (int i = 0; i < _arrlen; i++)
    {
        norm += (array[i] * array[i]);
    }
    norm = sqrt(norm);
    for (int i = 0; i < _arrlen; i++)
    {
        array[i] /= norm;
    }

    return array;
}

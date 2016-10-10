#include "SvmFilter.h"

#include <QDebug>

SvmFilter::SvmFilter()
{

}

QVector<QVector<double>>* SvmFilter::getResponses(QVector<QVector<double>> *patches)
{
    QVector<QVector<double>> twod_response;
    QVector<double> oned_response;
    int edge;
    int patch_width = filter_width - 1 + search_width;
    for (int i = 0; i < num_patches; i++)
    {
        // reset zeroes in temp_real_part
        for (int j = 0; j < fft_size * fft_size; j++)
        {
            temp_real_part[j] = 0.0;
        }
        // normalize patches to 0-1
        (*patches)[i] = normalizePatches((*patches)[i]);
        // patch must be padded (with zeroes) to match fft size
        for (int j = 0; j < patch_width; j++)
        {
            for (int k = 0; k < patch_width; k++)
            {
                temp_real_part[j + (fft_size * k)] = (*patches)[i][k + (patch_width * j)];
            }
        }
        // fft it

        twod_response = this->fft_inplace(temp_real_part, QVector<double>());

        // multiply pointwise with filter
        complex_mult_inplace(twod_response, fft_filters[i]);
        // inverse fft it
        oned_response = this->ifft(twod_response[0], twod_response[1]);
        // crop out edges
        edge = (filter_width - 1) / 2;
        for (int j = 0; j < search_width; j++)
        {
            for (int k = 0; k < search_width; k++)
            {
                (*responses)[i][j + (k * search_width)] = oned_response[edge + k + ((j + edge) * (fft_size))];
            }
        }

        // add bias
        for (int j = 0; j < search_width * search_width; j++)
        {
            (*responses)[i][j] += biases[i];
        }

        // logistic transformation
        (*responses)[i] = logisticResponse((*responses)[i]);

        // normalization?
        inplaceNormalizeFilterMatrix((*responses)[i]);
    }
    return responses;
}

void SvmFilter::inplaceNormalizeFilterMatrix(QVector<double> &response)
{
    // normalize responses to lie within [0,1]
    int msize = response.size();
    double max = 0.0;
    double min = 1.0;
    for (double aResponse : response)
    {
        max = aResponse > max ? aResponse : max;
        min = aResponse < min ? aResponse : min;
    }
    double dist = max - min;

    if (dist == 0.0)
    {

    }
    else
    {
        for (int i = 0; i < msize; i++)
        {
            response[i] = (response[i] - min) / dist;
        }
    }
}

QVector<double> SvmFilter::logisticResponse(QVector<double> response)
{
    // create probability by doing logistic transformation
    for (int j = 0; j < search_width; j++)
    {
        for (int k = 0; k < search_width; k++)
        {
            response[j + (k * search_width)] = 1.0 / (1.0 + exp(-(response[j + (k * search_width)] - 1.0)));
        }
    }
    return response;
}

QVector<double> SvmFilter::ifft(QVector<double> &rn, QVector<double> &cn)
{
    _fft->real_ifft2d(rn, cn);
    return rn;
}

void SvmFilter::complex_mult_inplace(QVector<QVector<double>> &cn1, QVector<QVector<double>> cn2)
{
    // in-place, cn1 is the one modified
    double temp1, temp2;
    for (int r = 0; r < filterLength; r++)
    {
        temp1 = (cn1[0][r] * cn2[0][r]) - (cn1[1][r] * cn2[1][r]);
        temp2 = (cn1[0][r] * cn2[1][r]) + (cn1[1][r] * cn2[0][r]);
        cn1[0][r] = temp1;
        cn1[1][r] = temp2;
    }
}

QVector<double> SvmFilter::normalizePatches(QVector<double> patch)
{
    int patch_width = filter_width-1+search_width;
    double max = 0.0;
    double min = 1000.0;
    double value;
    for (int j = 0;j < patch_width;j++) {
      for (int k = 0;k < patch_width;k++) {
        value = patch[k + (patch_width*j)];
        if (value < min) {
          min = value;
        }
        if (value > max) {
          max = value;
        }
      }
    }
    double scale = max-min;
    if(scale != 0.0)
    {
        for (int j = 0;j < patch_width;j++) {
          for (int k = 0;k < patch_width;k++) {
            patch[k + (patch_width*j)] = (patch[k + (patch_width*j)]-min)/scale;
          }
        }
    }
    return patch;
}

void SvmFilter::init(QJsonArray filter_input, QJsonArray bias_input, int numPatches, int filterWidth, int searchWidth)
{
    // calculate needed size of fft (has to be power of two)
    fft_size = upperPowerOfTwo(filterWidth - 1 + searchWidth);
    filterLength = fft_size * fft_size;
    _fft = new FFT();
    _fft->init(fft_size);
    fft_filters = QVector<QVector<QVector<double>>> (numPatches);
    QVector<QVector<double>> fft_filter;
    int edge = (filterWidth - 1) / 2;

    for (int i = 0; i < numPatches; i++)
    {
        QVector<double> flar_fi0(filterLength);
        QVector<double> flar_fi1(filterLength);

        // load filter
        int xOffset, yOffset;
        for (int j = 0; j < filterWidth; j++)
        {
            for (int k = 0; k < filterWidth; k++)
            {
                // rotate filter

                xOffset = k < edge ? (fft_size - edge) : (-edge);
                yOffset = j < edge ? (fft_size - edge) : (-edge);

                flar_fi0[k + xOffset + ((j + yOffset) * fft_size)] =
                        filter_input[i].toArray()
                        [(filterWidth - 1 - j) + ((filterWidth - 1 - k) * filterWidth)].toDouble();
            }
        }

        // fft it and store
        fft_filter = this->fft_inplace(flar_fi0, flar_fi1);
        fft_filters[i] = fft_filter;
    }

    // set up biases
    biases = QVector<double>(numPatches);
    for (int i = 0; i < numPatches; i++)
    {
        biases[i] = bias_input[i].toDouble();
    }

    responses = new QVector<QVector<double>> (numPatches);
    temp_imag_part = QVector<QVector<double>> (numPatches);
    for (int i = 0; i < numPatches; i++)
    {
        (*responses)[i] = QVector<double>(searchWidth * searchWidth);
        temp_imag_part[i] = QVector<double>(searchWidth * searchWidth);
    }
    temp_real_part = QVector<double>(filterLength);

    num_patches = numPatches;
    filter_width = filterWidth;
    search_width = searchWidth;
}

QVector<QVector<double>> SvmFilter::fft_inplace(QVector<double> &array, QVector<double> _im_part)
{
    if (_im_part.size() != 0)
    {
        for (int i = 0; i < filterLength; i++)
        {
            _im_part[i] = 0.0;
        }

        _fft->real_fft2d(array, _im_part);

        QVector<QVector<double>> result(2);
        result[0] = array;
        result[1] = _im_part;
        return result;
    }
    else
    {
        QVector<double> _t_im_part(filterLength);

        for (int i = 0; i < filterLength; i++)
        {
            _t_im_part[i] = 0.0;
        }

        _fft->real_fft2d(array, _t_im_part);

        QVector<QVector<double>> result(2);
        result[0] = array;
        result[1] = _t_im_part;
        return result;
    }
}

int SvmFilter::upperPowerOfTwo(int x)
{
    x--;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    x++;
    return x;
}


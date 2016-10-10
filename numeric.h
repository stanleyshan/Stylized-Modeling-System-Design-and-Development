#ifndef NUMERIC_H
#define NUMERIC_H

#include <QVector>
#include <math.h>

class numeric
{
public:
    static QVector<QVector<double>> rep(int row, int col, double value)
    {
        QVector<QVector<double>> ret(row);
        for (int i = 0; i < row; i++)
        {
            ret[i] = QVector<double>(col);
            for (int j = 0; j < col; j++)
            {
                ret[i][j] = value;
            }
        }
        return ret;
    }
    static QVector<double> rep(int row, int value)
    {
        QVector<double> ret(row);
        for (int i = 0; i < row; i++)
        {
            ret[i] = value;
        }
        return ret;
    }
    static QVector<QVector<double>> transpose(QVector<QVector<double>> x)
    {
        int i, j, m = x.length(), n = x[0].length();
        QVector<QVector<double>> ret(n);
        QVector<double> A0, A1, *Bj;
        for (j = 0; j < n; j++)
            ret[j] = QVector<double>(m);
        for (i = m - 1; i >= 1; i -= 2)
        {
            A1 = x[i];
            A0 = x[i - 1];
            for (j = n - 1; j >= 1; --j)
            {
                Bj = &ret[j];
                (*Bj)[i] = A1[j];
                (*Bj)[i - 1] = A0[j];
                --j;
                Bj = &ret[j];
                (*Bj)[i] = A1[j];
                (*Bj)[i - 1] = A0[j];
            }
            if (j == 0)
            {
                Bj = &ret[0];
                (*Bj)[i] = A1[0];
                (*Bj)[i - 1] = A0[0];
            }
        }
        if (i == 0)
        {
            A0 = x[0];
            for (j = n - 1; j >= 1; --j)
            {
                ret[j][0] = A0[j];
                --j;
                ret[j][0] = A0[j];
            }
            if (j == 0)
            {
                ret[0][0] = A0[0];
            }
        }
        return ret;
    }
    static QVector<QVector<double>> dot(QVector<QVector<double>> x, QVector<QVector<double>> y)
    {
        if (y.size() < 10)
            return dotMMsmall(x, y);
        else
            return dotMMbig(x, y);
    }

    static QVector<QVector<double>> mul(QVector<QVector<double>> matrix, int value)
    {
        QVector<QVector<double>> ret(matrix.size());
        for (int i = 0; i < matrix.size(); i++)
        {
            ret[i] = QVector<double>(matrix[0].size());
            for (int j = 0; j < matrix[i].size(); j++)
            {
                ret[i][j] = matrix[i][j] * value;
            }
        }
        return ret;
    }
    static QVector<QVector<double>> add(QVector<QVector<double>> a, QVector<QVector<double>> b)
    {
        QVector<QVector<double>> ret(a.size());

        for (int i = 0; i < a.size(); i++)
        {
            ret[i] = QVector<double>(a[0].size());
            for (int j = 0; j < a[i].size(); j++)
            {
                ret[i][j] = a[i][j] + b[i][j];
            }
        }

        return ret;
    }
    static QVector<QVector<double>> sub(QVector<QVector<double>> a, QVector<QVector<double>> b)
    {
        QVector<QVector<double>> ret(a.size());

        for (int i = 0; i < a.size(); i++)
        {
            ret[i] = QVector<double>(a[0].size());
            for (int j = 0; j < a[i].size(); j++)
            {
                ret[i][j] = a[i][j] - b[i][j];
            }
        }

        return ret;
    }
    static QVector<QVector<double>> inv(QVector<QVector<double>> a)
    {
        int m = a.size(), n = a[0].size();
        QVector<QVector<double>> A = clone(a);
        QVector<double> *Ai, *Aj;
        QVector<QVector<double>> I = identity(m);
        QVector<double> *Ii, *Ij;
        int i, j, k;
        double x;

//        for(int ii=0; ii<A.length(); ii++)
//        {
//            qDebug() << "A[" << ii << "]";
//            qDebug() << A[ii];
//        }

        for (j = 0; j < n; ++j)
        {
            int i0 = -1;
            int v0 = -1;
            for (i = j; i != m; ++i)
            {
                k = (int) abs(A[i][j]);
                if (k > v0)
                {
                    i0 = i;
                    v0 = k;
                }
            }
            Aj = &A[i0];
            A[i0] = A[j];
            A[j] = (*Aj);
            Ij = &I[i0];
            I[i0] = I[j];
            I[j] = (*Ij);
            x = (*Aj)[j];
            for (k = j; k != n; ++k)
                (*Aj)[k] /= x;
            for (k = n - 1; k != -1; --k)
                (*Ij)[k] /= x;
            for (i = m - 1; i != -1; --i)
            {
                if (i != j)
                {
                    Ai = &A[i];
                    Ii = &I[i];
                    x = (*Ai)[j];
                    for (k = j + 1; k != n; ++k)
                        (*Ai)[k] -= (*Aj)[k] * x;
                    for (k = n - 1; k > 0; --k)
                    {
                        (*Ii)[k] -= (*Ij)[k] * x;
                        --k;
                        (*Ii)[k] -= (*Ij)[k] * x;
                    }
                    if (k == 0)
                        (*Ii)[0] -= (*Ij)[0] * x;
                }
            }
        }
        return I;
    }

    static QVector<QVector<double>> diag(QVector<double> d)
    {
        int i, i1, j, n = d.size();
        QVector<QVector<double>> A(n);
        QVector<double> Ai;
        for (i = n - 1; i >= 0; i--)
        {
            Ai = QVector<double>(n);
            i1 = i + 2;
            for (j = n - 1; j >= i1; j -= 2)
            {
                Ai[j] = 0;
                Ai[j - 1] = 0;
            }
            if (j > i)
            {
                Ai[j] = 0;
            }
            Ai[i] = d[i];
            for (j = i - 1; j >= 1; j -= 2)
            {
                Ai[j] = 0;
                Ai[j - 1] = 0;
            }
            if (j == 0)
            {
                Ai[0] = 0;
            }
            A[i] = Ai;
        }
        return A;
    }


private:
    static QVector<QVector<double>> dotMMbig(QVector<QVector<double>> x, QVector<QVector<double>> y)
    {
        int p = y.size();
        QVector<double> v(p);
        int m = x.size();
        int n = y[0].size();
        QVector<QVector<double>> A(m);
        QVector<double> xj;
        int i, j;
        --p;
        --m;
        for (i = m; i != -1; --i)
            A[i] = QVector<double>(n);
        --n;
        for (i = n; i != -1; --i)
        {
            _getCol(y, i, v);
            for (j = m; j != -1; --j)
            {
                xj = x[j];
                A[j][i] = dotVV(xj, v);
            }
        }
        return A;
    }
    static double dotVV(QVector<double> x, QVector<double> y)
    {
        int i, n = x.size(), i1;
        double ret = x[n - 1] * y[n - 1];
        for (i = n - 2; i >= 1; i -= 2)
        {
            i1 = i - 1;
            ret += x[i] * y[i] + x[i1] * y[i1];
        }
        if (i == 0)
        {
            ret += x[0] * y[0];
        }
        return ret;
    }
    static void _getCol(QVector<QVector<double>> A, int j, QVector<double> &x)
    {
        int n = A.size(), i;
        if(x.size() == 0)
            x = QVector<double>(n);
        for (i = n - 1; i > 0; --i)
        {
            x[i] = A[i][j];
            --i;
            x[i] = A[i][j];
        }
        if (i == 0)
            x[0] = A[0][j];
    }
    static QVector<QVector<double>> dotMMsmall(QVector<QVector<double>> x, QVector<QVector<double>> y)
    {
        int i, j, k, p, q, r, i0;
        double woo;
        QVector<double> foo, bar;
        p = x.size();
        q = y.size();
        r = y[0].size();
        QVector<QVector<double>> ret(p);

        for (i = p - 1; i >= 0; i--)
        {
            foo = QVector<double>(r);
            bar = x[i];
            for (k = r - 1; k >= 0; k--)
            {
                woo = bar[q - 1] * y[q - 1][k];
                for (j = q - 2; j >= 1; j -= 2)
                {
                    i0 = j - 1;
                    woo += bar[j] * y[j][k] + bar[i0] * y[i0][k];
                }
                if (j == 0)
                {
                    woo += bar[0] * y[0][k];
                }
                foo[k] = woo;
            }
            ret[i] = foo;
        }
        return ret;
    }

    static QVector<QVector<double>> identity(int n)
    {
        return diag(rep(n, 1));
    }
    static QVector<QVector<double>> clone(QVector<QVector<double>> a)
    {
        QVector<QVector<double>> ret(a.size());
        for(int i=0; i<a.size(); i++)
        {
            ret[i] = a[i];
        }
        return ret;
    }
};

#endif // NUMERIC_H

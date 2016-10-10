#pragma once

#include "SplineSample.h"
#include <Eigen/Core>
#include <Eigen/SparseCore>
#include <Eigen/Sparse>
#include <Eigen/SparseQR>

#include <vector>

using namespace Eigen;
using namespace std;

namespace HSSSpline
{
	template <int D>
	class HSplineCore
	{
	protected:
		//Ρ絬把计
		struct Spline_Ele
		{
			double val[D][4];
		};
		std::vector< Spline_Ele > m_LineSeg_List;

	//Spline	
	public:
		int BuildingSpline(PathPoints<D> &_points)
		{
			if (_points().size() < 2)
			{
                //std::cerr << "Cubic spline point number less than 2!\n";
				m_LineSeg_List.clear();
				return 1;
			}

//			using namespace LinearSystemLib;

			/*ミA痻皚*/
            SparseMatrix<double> _matrix(_points().size(), _points().size());
            _matrix = setup_matrixA(_matrix, _points().size());

            //ミX,B痻皚
			// only support double type.
			//-------------------------------------------------
			const int dim = D;//x, y, w, h1, h2舱
            MatrixXd x(_points().size(), dim);
            MatrixXd B(_points().size(), dim);

			for (unsigned int i = 0; i < _points().size(); ++i)
			{
				if (i == 0)
				{
                    for (int d=0;d<dim;d++){B(i,d) = 3*(_points[1][d] - _points[0][d]);}
				}else if (i == _points().size()-1)
				{
                    for (int d=0;d<dim;d++){B(i,d) = 3*(_points[i][d] - _points[i-1][d]);}
				}else{
                    for (int d=0;d<dim;d++){B(i,d) = 3*(_points[i+1][d] - _points[i-1][d]);}
				}
			}

			// create the linear system A x = B.
			//-------------------------------------------------
			// note: ミ A and B memory 常盢パ sls 璽砫恨瞶
			// ps. A and B must allocate from heap.
			// ps. GA 斗锣Θ, stable sparse matrix.

            SparseQR<SparseMatrix<double>,COLAMDOrdering <int>> solver;
            solver.compute(_matrix);

			// solving it !! A * x = B.
			//-------------------------------------------------

            x = solver.solve(B);

            if(solver.info()!=Success) {
                //cout << "fail!!" << endl;
                return 0;
            }
            else{
                m_LineSeg_List.clear();
                for (unsigned int i = 0; i < _points().size()-1; ++i){
                    Spline_Ele ele;
                    for (int d=0;d<dim;d++){
                        ele.val[d][0] = (double)( x(i+1, d) + x(i, d) - 2 * (_points[i+1][d] - _points[i][d]) );
                        ele.val[d][1] = (double)( 3 * (_points[i+1][d] - _points[i][d]) - 2 * x(i,d) - x(i+1, d));
                        ele.val[d][2] = (double)( x(i, d));
                        ele.val[d][3] = (double)( _points[i][d] );
                    }
                    m_LineSeg_List.push_back(ele);
                }
            }

			return 0;
		}

        SparseMatrix<double> setup_matrixA(SparseMatrix<double> _matrix, int _size){
            _matrix.insert(0,0) = 2.0;
            for (int i = 1; i < _size-1; ++i){
                _matrix.insert(i,i) = 4.0;
            }
            for (int i = 0; i < _size-1; ++i){
                _matrix.insert(i,i+1) = 1.0;
                _matrix.insert(i+1,i) = 1.0;
            }
            _matrix.insert(_size-1,_size-1) = 2.0;
            return _matrix;
                }

	//Get Info
	public:
		/*眔琿计*/
		int n_segs(){return m_LineSeg_List.size();}
		double get_value(int dim,int _seg, double _t)
		{
			double t2 = _t*_t, t3 = t2*_t;
			return (double)( m_LineSeg_List[_seg].val[dim][0] * t3 + m_LineSeg_List[_seg].val[dim][1] * t2 + m_LineSeg_List[_seg].val[dim][2] * _t + m_LineSeg_List[_seg].val[dim][3] );
		}

		//Ω稬だ
		double get_D1_value(int dim,int _seg, double _t)
		{
			double t2 = _t*_t;
			return (double)( 3 * m_LineSeg_List[_seg].val[dim][0] * t2 + 2 * m_LineSeg_List[_seg].val[dim][1] * _t + m_LineSeg_List[_seg].val[dim][2] );
		}

		double get_D2_value(int dim,int _seg, double _t)
		{
			return (double)( 6 * m_LineSeg_List[_seg].val[dim][0] * _t + 2 * m_LineSeg_List[_seg].val[dim][1]);
		}

		/*眔spline琘翴, seg琌琿, t琌把计*/
		PathPoint<D>     get_point(int _seg, double _t)
		{
			HSSSpline::PathPoint<D> p;
			for (int i=0;i<D;i++)
			{
				p[i] = get_value(i,_seg,_t);
			}
			return p;
		}
		PathPoint<D>     get_point(const Sample& sample){return get_point(sample.seg_idx,sample._t);}
	};
}

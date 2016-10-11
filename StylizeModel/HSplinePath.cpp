//#include "StdAfx.h"
#include <algorithm>
#include <cmath>
#include "HSplinePath.h"
#include <QDebug>

namespace HSSSpline
{
    PathPoint<2> HSpline2D::get_position(int _seg, double _t)
    {
        PathPoint<2> position(0,0);
        position[0] = get_x(_seg, _t);
        position[1] = get_y(_seg, _t);
        return position;
    }

    PathPoint<2> HSpline2D::get_tangent(int _seg, double _t)
    {
        PathPoint<2> tangent_dir;
        tangent_dir[0] = get_D1_value(0,_seg, _t);
        tangent_dir[1] = get_D1_value(1,_seg, _t);

        //Normalize
        double len = sqrt(tangent_dir[0]*tangent_dir[0] + tangent_dir[1]*tangent_dir[1]);
        tangent_dir[0]/=len;
        tangent_dir[1]/=len;
        return tangent_dir;
    }

    PathPoint<2> HSpline2D::get_normal(int _seg, double _t)
    {
        PathPoint<2> normal_dir;
        float x_temp = get_D1_value(0,_seg, _t);
        float y_temp = get_D1_value(1,_seg, _t);

        normal_dir[0] = (y_temp==0?0:-y_temp);
        normal_dir[1] = (x_temp==0?0:x_temp);

        //Normalize
        double len = sqrt(normal_dir[0]*normal_dir[0] + normal_dir[1]*normal_dir[1]);
        normal_dir[0]/=len;
        normal_dir[1]/=len;

        return normal_dir;
    }

    double HSpline2D::Lenght(const Sample& s0,const Sample& s1 )
    {
        return BeginToSampleLenght(s0)-BeginToSampleLenght(s1);
    }

    double HSpline2D::Lenght()
    {
        double len=0;
        for (unsigned int i=0;i<m_LineSeg_List.size();i++)
        {
            len += SegLenght(i);
        }
        return len;
    }

    double HSpline2D::SegLenght( int seg_idx )
    {
        double len=0;
        double step = 0.00001;
        for (double k=0;k<1;k+=step)
        {
            PathPoint<2> p0 = get_position(seg_idx,k);
            PathPoint<2> p1 = get_position(seg_idx,k+step);
            len += sqrt((p1[0]-p0[0])*(p1[0]-p0[0]) + (p1[1]-p0[1])*(p1[1]-p0[1]));
        }
        return len;
    }

    double HSpline2D::BeginToSampleLenght(const Sample& s )
    {
        double len=0;
        for (unsigned int i=0;i<s.seg_idx;i++)
        {
            len += SegLenght(i);
        }
        len += SegToSampleLenght(s);

        return len;
    }

    double HSpline2D::SampleToEndLenght(const Sample& s )
    {
        return Lenght()-BeginToSampleLenght(s);
    }

    double HSpline2D::SegToSampleLenght(const Sample& sample )
    {
        double len=0;
        double step = 0.01;
        for (double k=0;k<sample._t;k+=step)
        {
            PathPoint<2> p0 = get_position(sample.seg_idx,k);
            PathPoint<2> p1 = get_position(sample.seg_idx,k+step);
            len += sqrt((p1[0]-p0[0])*(p1[0]-p0[0]) + (p1[1]-p0[1])*(p1[1]-p0[1]));
        }
        return len;
    }

    void HSpline2D::PushBack( PathPoint<2>& point )
    {
        m_Piecewise_Points().push_back(PathPoint<2>(point[0],point[1]));
    }

    void HSpline2D::AssignPoints( PathPoints<2>& points)
    {
        m_Piecewise_Points().assign(points().begin(),points().end());
    }

    void HSpline2D::AssignCtrlPoints( PathPoints<2>& ctrl_points )
    {
        m_Ctrl_Points().clear();
        m_Ctrl_Points().assign(ctrl_points().begin(),ctrl_points().end());
        BuildingSpline(m_Ctrl_Points);
    }

    void HSpline2D::AugmentCtrlPoint( int num )
    {
        PathPoints<2> ctrl;
        ctrl().push_back(get_point(0,0));
        for (int i=0;i<n_segs();i++)
        {
            double seg = 1/(double)num;
            for (double t=seg;t<=1;t+=seg)
            {
                ctrl().push_back(get_point(i,t));
            }
        }
        AssignCtrlPoints(ctrl);
    }

    void HSpline2D::FittingCurve( double fit_ratio, bool auto_adjust )
    {
        if (m_Piecewise_Points().empty())return;

        BuildingSpline(m_Piecewise_Points);



        m_Ctrl_Points().clear();
        double lenght = Lenght();

        if (auto_adjust)
        {
            if (lenght<50)
            {
                fit_ratio = 0.5;
            }else if (lenght<150)
            {
                fit_ratio = 0.2;
            }else
            {
                fit_ratio /= (int)(1+lenght/500);
            }
        }

        m_Ctrl_Points = GetUniformSamplePoints(fit_ratio*lenght);
        BuildingSpline(m_Ctrl_Points);
    }

    void HSpline2D::RefittingCurve( double fit_ratio )
    {
        if (Lenght()<30)
        {
            fit_ratio=0.5;
        }
        else if (Lenght()<50)
        {
            fit_ratio=0.25;
        }
        else if(Lenght()<100)
        {
            fit_ratio=0.2;
        }
        else if(Lenght()<150)
        {
            fit_ratio=0.1;
        }

        m_Ctrl_Points = GetUniformSamplePoints(fit_ratio*Lenght());
        BuildingSpline(m_Ctrl_Points);
    }

    HSSSpline::Samples HSpline2D::UniformSampling( double per_len )
    {
        Samples samples;

        Sample sample_0;
        sample_0.seg_idx = 0;
        sample_0._t = 0;
        samples.push_back(sample_0);

        double temp_len = 0;
        for (unsigned int i=0;i<m_LineSeg_List.size();i++)
        {
            double step = 1/SegLenght(i);
            for (double k=0;k<1;k+=step)
            {
                PathPoint<2> p0 = get_position(i,k);
                PathPoint<2> p1;
                if (k+step<=1){p1 = get_position(i,k+step);}
                else {p1 = get_position(i,1);}

                temp_len += sqrt((p1[0]-p0[0])*(p1[0]-p0[0]) + (p1[1]-p0[1])*(p1[1]-p0[1]));

                if (temp_len >= per_len)
                {
                    Sample sample;
                    sample.seg_idx = i;
                    sample._t = k+step;
                    samples.push_back(sample);
                    temp_len = 0;
                }
            }
        }
        return samples;
    }

    HSSSpline::PathPoints<2> HSpline2D::GetUniformSamplePoints( double per_len )
    {
        HSSSpline::PathPoints<2> points;
        double temp_len = 0;

        points().push_back(get_point(0,0));
        for (unsigned int i=0;i<m_LineSeg_List.size();i++)
        {
            double step = 0.01;
            for (double k=0;k<1;k+=step)
            {
                PathPoint<2> p0 = get_point(i,k);
                PathPoint<2> p1;
                if (k+step<=1){p1 = get_point(i,k+step);}
                else {p1 = get_point(i,1);}
                double len = sqrt((p1[0]-p0[0])*(p1[0]-p0[0]) + (p1[1]-p0[1])*(p1[1]-p0[1]));
                temp_len += len;

                if (abs(temp_len - per_len)<0.01)
                {
                    points().push_back(p1);
                    temp_len = 0;
                }else{
                    if (temp_len > per_len)
                    {
                        temp_len-=len;
                        step/=10;
                    }
                }
            }
        }

        if (temp_len>per_len*0.5)
        {
            points().push_back(get_point(m_LineSeg_List.size()-1,1));
        }

        return points;
    }

    double HSpline2D::SelectCtrlPoint( double x,double y,int *select )
    {
        *select = -1;
        double min_dis = 100;
        for (unsigned int i=0;i<m_Ctrl_Points().size();i++)
        {
            double dis = (m_Ctrl_Points[i][0] - x)*(m_Ctrl_Points[i][0] - x) + (m_Ctrl_Points[i][1] - y)*(m_Ctrl_Points[i][1] - y);
            if (min_dis > dis)
            {
                min_dis = dis;
                *select = i;
            }
        }
        return min_dis;
    }

    void HSpline2D::EditCtrlPoint( int c_id,PathPoint<2>& point )
    {
        m_Ctrl_Points[c_id][0] = point[0];
        m_Ctrl_Points[c_id][1] = point[1];
        BuildingSpline(m_Ctrl_Points);
    }

    bool HSpline2D::ShiftSample( Sample& from,double offset,Sample* output )
    {
        if (offset<0)
        {
            return ShiftSampleBackward(from,abs(offset),output);
        }else{
            return  ShiftSampleForeward(from,abs(offset),output);
        }
    }

    bool HSpline2D::ShiftSampleForeward( Sample& from,double offset,Sample* output )
    {
        double k=from._t;
        double step = 0.001;
        double sum_dis = 0;

        for (int i=from.seg_idx;i<m_LineSeg_List.size();i++)
        {
            while (k<1)
            {
                PathPoint<2> p0 = get_position(i,k);
                k+=step;if (k>1)break;
                PathPoint<2> p1 = get_position(i,k);

                double dis = sqrt((p1[0]-p0[0])*(p1[0]-p0[0])+(p1[1]-p0[1])*(p1[1]-p0[1]));
                sum_dis += dis;

                if (offset<=sum_dis)
                {
                    output->seg_idx = i;
                    output->_t = k;
                    return true;
                }
            }
            k=0;
        }

        output->seg_idx = m_LineSeg_List.size()-1;
        return false;
    }

    bool HSpline2D::ShiftSampleBackward( Sample& from,double offset,Sample* output )
    {
        double k=from._t;
        double step = 0.001;
        double sum_dis = 0;
        for (int i=from.seg_idx;i>=0;i--)
        {
            while (k>0)
            {
                PathPoint<2> p0 = get_position(i,k);
                k-=step;if (k<0)break;
                PathPoint<2> p1 = get_position(i,k);

                double dis = sqrt((p1[0]-p0[0])*(p1[0]-p0[0])+(p1[1]-p0[1])*(p1[1]-p0[1]));
                sum_dis += dis;

                if (offset<=sum_dis)
                {
                    output->seg_idx = i;
                    output->_t = k;
                    return true;
                }
            }
            k=1;
        }

        output->seg_idx = 0;
        output->_t = 0;
        return false;
    }

//	void HSpline2D::DrawLine()
//	{
//		Sample begin(0,0);
//		Sample end(m_LineSeg_List.size()-1,1);
//		DrawLine( begin,end );
//	}

//	void HSpline2D::DrawLine( Sample& from,Sample& to )
//	{
//		double step = 0.1;
//		//glBegin(GL_LINES);
//		for (int i=from.seg_idx;i<=to.seg_idx;i++)
//		{
//			double k = 0;
//			if (i==from.seg_idx)k = from._t;
//			double end = 1;
//			if (i==to.seg_idx)end = to._t;

//			while(k<end)
//			{
//				PathPoint<2> p1 = get_position(i,k);
//				k+=step;
//				if (k>end)k=end;
//				PathPoint<2> p2 = get_position(i,k);

//				glVertex2d(p1[0],p1[1]);
//				glVertex2d(p2[0],p2[1]);
//			}
//		}
//		glEnd();
//	}

//	void HSpline2D::DrawCtrlPoint()
//	{
//		glPointSize(5);
//		glColor3d(1.0,0.0,0.0);
//		glBegin(GL_POINTS);
//		for (int i=0;i<m_Ctrl_Points().size();i++)
//		{
//			glVertex2d(m_Ctrl_Points[i][0],m_Ctrl_Points[i][1]);
//		}
//		glEnd();
//	}
    void HSpline2D::PushCtrlPoint(){
//        for (int i=0;i<m_Ctrl_Points().size();i++)
//        {
//            if(mode == 0){
//                m_CtrlPoines.push_back(m_Ctrl_Points[i][0]);
//                m_CtrlPoines.push_back(m_Ctrl_Points[i][1]);
//                m_CtrlPoines.push_back(const_number);
//            }
//            else{
//                m_CtrlPoines.push_back(const_number);
//                m_CtrlPoines.push_back(m_Ctrl_Points[i][0]);
//                m_CtrlPoines.push_back(m_Ctrl_Points[i][1]);
//            }

//        }
    }

    void HSpline2D::PushCubicSplined_init(){
//        Sample begin(0,0);
//        Sample end(m_LineSeg_List.size()-1,1);
//        PushCubicSplined( begin,end );
    }

    void HSpline2D::PushCubicSplined(Sample& from,Sample& to){

//        double step = 0.1;
//        for (int i=from.seg_idx;i<=to.seg_idx;i++)
//        {
//            double k = 0;
//            if (i==from.seg_idx)k = from._t;
//            double end = 1;
//            if (i==to.seg_idx)end = to._t;

//            while(k<end)
//            {
//                PathPoint<2> p1 = get_position(i,k);
//                k+=step;
//                if (k>end)k=end;
//                //PathPoint<2> p2 = get_position(i,k);
//                if(mode == 0){
//                    m_Cubic_Spline.push_back(p1[0]);
//                    m_Cubic_Spline.push_back(p1[1]);
//                    m_Cubic_Spline.push_back(const_number);
//                }
//                else{
//                    m_Cubic_Spline.push_back(const_number);
//                    m_Cubic_Spline.push_back(p1[0]);
//                    m_Cubic_Spline.push_back(p1[1]);
//                }
//                /*
//                m_Cubic_Spline_Depth.push_back(p1[0]);
//                m_Cubic_Spline_Depth.push_back(p1[1]);
//                m_Cubic_Spline_Depth.push_back(depth);*/
//            }
//        }
    }

}

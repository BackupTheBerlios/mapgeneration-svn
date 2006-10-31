/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#ifndef AXISPARALLELRECTANGLE_H
#define AXISPARALLELRECTANGLE_H

//#include <ostream>
#include <cassert>

namespace rangereporting
{
	
	//-------------------------------------------------------------------------//
	//--- Declaration section -------------------------------------------------//
	//-------------------------------------------------------------------------//
	
	template<typename T_Point_2D>
	class AxisParallelRectangle;
	
	
//	template<typename T_Point_2D>
//	class Trapezoid;
	
	
	template<typename T_Point_2D>
	std::ostream&
	operator<<(std::ostream& out,
		const AxisParallelRectangle<T_Point_2D>& segment);
	
}

//#include "trapezoid.h"
#include "polygon.h"

//#define _llc _lower_left_corner
//#define _urc _upper_right_corner

namespace rangereporting
{
	
	//-------------------------------------------------------------------------//
	//--- Definition section --------------------------------------------------//
	//-------------------------------------------------------------------------//
	
	template<typename T_Point_2D>
	class AxisParallelRectangle : public Polygon<T_Point_2D>
	{
		
//		friend class Trapezoid<T_Point_2D>;
		
//		friend std::ostream& operator<< <> (std::ostream& out,
//			const Rectangle<T_Point_2D>& rectangle);
		
		
		public:
			
			AxisParallelRectangle();
			
			
			AxisParallelRectangle(const std::vector<T_Point_2D>& points);
			
			
			virtual inline double
			area_size() const;
			
			
			virtual inline bool
			contains(const T_Point_2D& point) const;
			
			
			inline bool
			contains(const AxisParallelRectangle<T_Point_2D>& rectangle) const;
			
			
			inline bool
			intersects(const AxisParallelRectangle<T_Point_2D>& rectangle) const;
			
			
			inline void
			validate_corners();
			
			
		protected:
			
			mutable bool _corners_are_valid;
			mutable bool _corners_validation_in_progress;
			
			
			virtual inline void
			after_set_point(int index, const T_Point_2D& point);
			
			
			virtual inline void
			after_set_points(const std::vector<T_Point_2D>& points);
			
/*			
			virtual inline void
			before_get_point(int index) const;
			
			
			virtual inline void
			before_get_points() const;
*/			
	};
	
	
	//-------------------------------------------------------------------------//
	//--- Implementation section ----------------------------------------------//
	//-------------------------------------------------------------------------//
	
	template<typename T_Point_2D>
	std::ostream&
	operator<<(std::ostream& out,
		const AxisParallelRectangle<T_Point_2D>& rectangle)
	{
		out << "AxisParallelRectangle:" << std::endl
			<< "\t_upper_right_corner=" << rectangle.get_point(1) << std::endl
			<< "\t _lower_left_corner=" << rectangle.get_point(0);
		
		return out;
	}
	
	
	template<typename T_Point_2D>
	AxisParallelRectangle<T_Point_2D>::AxisParallelRectangle()
	: Polygon<T_Point_2D>(2), _corners_are_valid(true),
		_corners_validation_in_progress(false)
	{
	}
	
	
	template<typename T_Point_2D>
	AxisParallelRectangle<T_Point_2D>::AxisParallelRectangle(
		const std::vector<T_Point_2D>& points)
	: Polygon<T_Point_2D>(points), _corners_are_valid(false),
		_corners_validation_in_progress(false)
	{
	}
	
	template<typename T_Point_2D>
	void
	AxisParallelRectangle<T_Point_2D>::after_set_point(int index,
		const T_Point_2D& point)
	{
		_corners_are_valid = false;
	}
	
	
	template<typename T_Point_2D>
	void
	AxisParallelRectangle<T_Point_2D>::after_set_points(
		const std::vector<T_Point_2D>& points)
	{
		validate_corners();
	}
	
	
	template<typename T_Point_2D>
	double
	AxisParallelRectangle<T_Point_2D>::area_size() const
	{
		double a = this->get_point(1)[0] - this->get_point(0)[0];
		double b = this->get_point(1)[1] - this->get_point(0)[1];
		
		assert(a >= 0.0);
		assert(b >= 0.0);
		
		return a*b;
	}
	
/*	
	template<typename T_Point_2D>
	void
	AxisParallelRectangle<T_Point_2D>::before_get_point(int index) const
	{
		if (!_corners_are_valid && !_corners_validation_in_progress)
		{
			std::cout << "AxisParallelRectangle::before_get_point: Will throw "
				<< "an exception now!" << std::endl;
			throw "AxisParallelRectangle: Corners are invalid!";
		}
	}
	
	
	template<typename T_Point_2D>
	void
	AxisParallelRectangle<T_Point_2D>::before_get_points() const
	{
		if (!_corners_are_valid && !_corners_validation_in_progress)
		{
			std::cout << "AxisParallelRectangle::before_get_point: Will throw "
				<< "an exception now!" << std::endl;
			throw "AxisParallelRectangle: Corners are invalid!";
		}
	}
*/	
	
	template<typename T_Point_2D>
	bool
	AxisParallelRectangle<T_Point_2D>::contains(const T_Point_2D& point) const
	{
		return (
			(this->get_point(0)[0] <= point[0])
			&& (point[0] <= this->get_point(1)[0])
			&& (this->get_point(0)[1] <= point[1])
			&& (point[1] <= this->get_point(1)[1])
		);
	}
	
	
	template<typename T_Point_2D>
	bool
	AxisParallelRectangle<T_Point_2D>::contains(
		const AxisParallelRectangle<T_Point_2D>& rectangle) const
	{
		return ( contains(rectangle.get_point(0))
			&& contains(rectangle.get_point(1)) );
	}
	
	
	template<typename T_Point_2D>
	bool
	AxisParallelRectangle<T_Point_2D>::intersects(
		const AxisParallelRectangle<T_Point_2D>& rectangle) const
	{
		bool dimension_0 = ((this->get_point(0)[0] <= rectangle.get_point(0)[0])
				&& (rectangle.get_point(0)[0] <= this->get_point(1)[0]))
			|| ((rectangle.get_point(0)[0] <= this->get_point(0)[0])
				&& (this->get_point(0)[0] <= rectangle.get_point(1)[0]));
		
		bool dimension_1 = ((this->get_point(0)[1] <= rectangle.get_point(0)[1])
				&& (rectangle.get_point(0)[1] <= this->get_point(1)[1]))
			|| ((rectangle.get_point(0)[1] <= this->get_point(0)[1])
				&& (this->get_point(0)[1] < rectangle.get_point(1)[1]));
		
		return ( dimension_0 && dimension_1 );
	}
	
	
	template<typename T_Point_2D>
	void
	AxisParallelRectangle<T_Point_2D>::validate_corners()
	{
		if (!_corners_are_valid)
		{
			_corners_validation_in_progress = true;
			
			T_Point_2D lower_left_corner;
			lower_left_corner[0] = this->get_point(0)[0];
			lower_left_corner[1] = this->get_point(0)[1];
			
			T_Point_2D upper_right_corner;
			upper_right_corner[0] = this->get_point(1)[0];
			upper_right_corner[1] = this->get_point(1)[1];
			
			if (lower_left_corner[0] > upper_right_corner[0])
			{
				T_Point_2D temp;
				temp[0] = lower_left_corner[0];
				lower_left_corner[0] = upper_right_corner[0];
				upper_right_corner[0] = temp[0];
			}
			
			if (lower_left_corner[1] > upper_right_corner[1])
			{
				T_Point_2D temp;
				temp[1] = lower_left_corner[1];
				lower_left_corner[1] = upper_right_corner[1];
				upper_right_corner[1] = temp[1];
			}
			
			this->set_point(0, lower_left_corner);
			this->set_point(1, upper_right_corner);
			
			_corners_validation_in_progress = false;
		}
		
		_corners_are_valid = true;
	}
	
} // namespace rangereporting

#endif //AXISPARALLELRECTANGLE_H

/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#ifndef TRAPEZOID_H
#define TRAPEZOID_H

#include <ostream>

namespace rangereporting
{
	
	//-------------------------------------------------------------------------//
	//--- Declaration section -------------------------------------------------//
	//-------------------------------------------------------------------------//

	template<typename T_Point_2D>
	class Trapezoid;
	
	
	template<typename T_Point_2D>
	std::ostream&
	operator<<(std::ostream& out, const Trapezoid<T_Point_2D>& segment);
	
}

#include "rectangle.h"

namespace rangereporting
{	
	
	//-------------------------------------------------------------------------//
	//--- Definition section --------------------------------------------------//
	//-------------------------------------------------------------------------//

	template<typename T_Point_2D>
	class Trapezoid
	{
		
		friend std::ostream& operator<< <> (std::ostream& out,
			const Trapezoid<T_Point_2D>& segment);
		
		
		public:
			
			Trapezoid(const T_Point_2D& point_1, const T_Point_2D& point_2,
				const T_Point_2D& point_3, const T_Point_2D& point_4,
				bool please_verify_point_order = false);
			
			
			inline const Rectangle<T_Point_2D>&
			bounding_rectangle() const;
			
			
			inline const T_Point_2D&
			operator[](int point_count) const;
			
			
		protected:
			
			Rectangle<T_Point_2D> _bounding_rectangle;
			
			
			// have to be given clockwise or use verify_point_order!
			std::vector<T_Point_2D> _points;
			
			
			void
			verify_and_correct_point_order();
	};


	//-------------------------------------------------------------------------//
	//--- Implementation section ----------------------------------------------//
	//-------------------------------------------------------------------------//
	
	template<typename T_Point_2D>
	std::ostream&
	operator<<(std::ostream& out, const Trapezoid<T_Point_2D>& trapezoid)
	{
		out << "Trapezoid:" << std::endl
			<< "\tCorner points (no specific order)" << std::endl
			<< "\t" << trapezoid._points[0] << std::endl
			<< "\t" << trapezoid._points[1] << std::endl
			<< "\t" << trapezoid._points[2] << std::endl
			<< "\t" << trapezoid._points[3];
		
		return out;
	}
	
	
	template<typename T_Point_2D>
	Trapezoid<T_Point_2D>::Trapezoid(const T_Point_2D& point_1,
		const T_Point_2D& point_2, const T_Point_2D& point_3,
		const T_Point_2D& point_4, bool please_verify_point_order)
	: _bounding_rectangle(), _points()
	{
		_points.push_back(point_1);
		_points.push_back(point_2);
		_points.push_back(point_3);
		_points.push_back(point_4);
		
		_bounding_rectangle._lower_left_corner = _points[0];
		_bounding_rectangle._upper_right_corner = _points[0];

		for (int i = 1; i < 4; ++i)
		{
			if (_points[i][0] < _bounding_rectangle._lower_left_corner[0])
				_bounding_rectangle._lower_left_corner[0] = _points[i][0];

			if (_points[i][1] < _bounding_rectangle._lower_left_corner[1])
				_bounding_rectangle._lower_left_corner[1] = _points[i][1];

			if (_points[i][0] > _bounding_rectangle._upper_right_corner[0])
				_bounding_rectangle._upper_right_corner[0] = _points[i][0];

			if (_points[i][1] > _bounding_rectangle._upper_right_corner[1])
				_bounding_rectangle._upper_right_corner[1] = _points[i][1];
		}
		

		if (please_verify_point_order)
			verify_and_correct_point_order();
	}
	
	
	template<typename T_Point_2D>
	inline const Rectangle<T_Point_2D>&
	Trapezoid<T_Point_2D>::bounding_rectangle() const
	{
		return _bounding_rectangle;
	}


	template<typename T_Point_2D>
	inline const T_Point_2D&
	Trapezoid<T_Point_2D>::operator[](int point_count) const
	{
		return _points[point_count];
	}


	template<typename T_Point_2D>
	void
	Trapezoid<T_Point_2D>::verify_and_correct_point_order()
	{
		for (int i = 0; i < 4; ++i)
		{
			int relative_pos = relative_position(_points[i % 4],
				_points[(i+1) % 4], _points[(i+2) % 4]);
			
			if (relative_pos < 0)
			{
				T_Point_2D temp = _points[(i+1) % 4];
				_points[(i+1) % 4] = _points[(i+2) % 4];
				_points[(i+2) % 4] = temp;
			}
		}
	}
	
	
}

#endif //TRAPEZOID_H

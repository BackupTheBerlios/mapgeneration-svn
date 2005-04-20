/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#ifndef RECTANGLE_H
#define RECTANGLE_H

#include <ostream>

#include "rangereportingsystem.h"

#define _llc _lower_left_corner
#define _urc _upper_right_corner

namespace rangereporting
{
	
	//-------------------------------------------------------------------------//
	//--- Definition section --------------------------------------------------//
	//-------------------------------------------------------------------------//
	
	template<typename T_Point_2D>
	class Rectangle
	{
		
		friend class Quadtree<T_Point_2D>;
		
		friend class Trapezoid<T_Point_2D>;
		
		friend std::ostream& operator<< <> (std::ostream& out,
			const Quadtree<T_Point_2D>& segment);
		
		friend std::ostream& operator<< <> (std::ostream& out,
			const Rectangle<T_Point_2D>& segment);
		
		
		public:
			
			Rectangle();
			
			
			Rectangle(const T_Point_2D& lower_left_corner,
				const T_Point_2D& upper_right_corner,
				bool please_verify_corners = false);
			
			
			inline bool
			contains(const T_Point_2D& point) const;
			
			
			inline bool
			contains(const Rectangle<T_Point_2D>& rectangle) const;
			
			
			inline bool
			intersects(const Rectangle<T_Point_2D>& rectangle) const;
			
			
		protected:
			
			T_Point_2D _lower_left_corner;
			
			
			T_Point_2D _upper_right_corner;
			
			
			void
			verify_corners();
	};
	
	
	//-------------------------------------------------------------------------//
	//--- Implementation section ----------------------------------------------//
	//-------------------------------------------------------------------------//
	
	template<typename T_Point_2D>
	std::ostream&
	operator<<(std::ostream& out, const Rectangle<T_Point_2D>& rectangle)
	{
		out << "Rectangle:" << std::endl
			<< "\t_upper_right_corner=" << rectangle._upper_right_corner << std::endl
			<< "\t _lower_left_corner=" << rectangle._lower_left_corner;
		
		return out;
	}
	
	
	template<typename T_Point_2D>
	Rectangle<T_Point_2D>::Rectangle()
	: _lower_left_corner(), _upper_right_corner()
	{
	}
	
	
	template<typename T_Point_2D>
	Rectangle<T_Point_2D>::Rectangle(const T_Point_2D& lower_left_corner,
		const T_Point_2D& upper_right_corner, bool please_verify_corners)
	: _lower_left_corner(lower_left_corner),
		_upper_right_corner(upper_right_corner)
	{
		if (please_verify_corners)
			verify_corners();
	}
	
	
	template<typename T_Point_2D>
	inline bool
	Rectangle<T_Point_2D>::contains(const T_Point_2D& point) const
	{
		return (_llc[0] <= point[0] && point[0] < _urc[0])
			&& (_llc[1] <= point[1] && point[1] < _urc[1]);
	}
	
	
	template<typename T_Point_2D>
	inline bool
	Rectangle<T_Point_2D>::contains(
		const Rectangle<T_Point_2D>& rectangle) const
	{
		return (_llc[0] <= rectangle._llc[0]) && (rectangle._llc[0] < _urc[0])
			&& (_llc[0] <= rectangle._urc[0]) && (rectangle._urc[0] < _urc[0])
			&& (_llc[1] <= rectangle._llc[1]) && (rectangle._llc[1] < _urc[1])
			&& (_llc[1] <= rectangle._urc[1]) && (rectangle._urc[1] < _urc[1]);
	}
	
	
	template<typename T_Point_2D>
	inline bool
	Rectangle<T_Point_2D>::intersects(
		const Rectangle<T_Point_2D>& rectangle) const
	{
		bool dimension_0 = 
			   (_llc[0] <= rectangle._llc[0]) && (rectangle._llc[0] < _urc[0])
			|| (rectangle._llc[0] <= _llc[0]) && (_llc[0] < rectangle._urc[0]);
		
		bool dimension_1 = 
			   (_llc[1] <= rectangle._llc[1]) && (rectangle._llc[1] < _urc[1])
			|| (rectangle._llc[1] <= _llc[1]) && (_llc[1] < rectangle._urc[1]);
		
		return ( dimension_0 && dimension_1 );
	}
	
	
	template<typename T_Point_2D>
	void
	Rectangle<T_Point_2D>::verify_corners()
	{
		if (_lower_left_corner[0] > _upper_right_corner[0])
		{
			T_Point_2D temp;
			temp[0] = _lower_left_corner[0];
			_lower_left_corner[0] = _upper_right_corner[0];
			_upper_right_corner[0] = temp[0];
		}
		
		if (_lower_left_corner[1] > _upper_right_corner[1])
		{
			T_Point_2D temp;
			temp[1] = _lower_left_corner[1];
			_lower_left_corner[1] = _upper_right_corner[1];
			_upper_right_corner[1] = temp[1];
		}
	}
}

#endif //RECTANGLE_H

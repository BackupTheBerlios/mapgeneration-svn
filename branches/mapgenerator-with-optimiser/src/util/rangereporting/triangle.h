/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#ifndef TRIANGLE_H
#define TRIANGLE_H

#include <cmath>
//#include <ostream>

#include "polygon.h"
#include "helperfunctions.h"

namespace rangereporting
{
	
	//-------------------------------------------------------------------------//
	//--- Declaration section -------------------------------------------------//
	//-------------------------------------------------------------------------//
	
	template<typename T_Point_2D>
	class Triangle;
	
	
//	template<typename T_Point_2D>
//	std::ostream&
//	operator<<(std::ostream& out, const Quadrangle<T_Point_2D>& segment);
	
}

namespace rangereporting
{
	
	//-------------------------------------------------------------------------//
	//--- Definition section --------------------------------------------------//
	//-------------------------------------------------------------------------//
	
	template<typename T_Point_2D>
	class Triangle : public Polygon<T_Point_2D>
	{
		
		public:
			
			Triangle();
			
			
			Triangle(const std::vector<T_Point_2D>& points);
			
			
			virtual inline double
			area_size() const;
			
			
			virtual inline bool
			contains(const T_Point_2D& point) const;
			
			
			inline bool
			is_left_system() const;
			
		
			inline bool
			is_right_system() const;
			
			
		protected:
			
			virtual inline void
			after_set_point(int index, const T_Point_2D& point);
			
			
			virtual inline void
			after_set_points(const std::vector<T_Point_2D>& points);
			
			
		private:
			
			enum System
			{
				_left_system = 1,
				_right_system = -1,
				_no_system = 0
			};
			
			
			mutable double _cached_area_value;
			mutable bool _cached_area_value_is_valid;
			mutable System _system;
			mutable bool _system_is_valid;
			
	};
	
	
	//-------------------------------------------------------------------------//
	//--- Implementation section ----------------------------------------------//
	//-------------------------------------------------------------------------//
	
/*	template<typename T_Point_2D>
	std::ostream&
	operator<<(std::ostream& out, const Quadrangle<T_Point_2D>& quadtangle)
	{
		out << "Quadrangle:" << std::endl << "points: ";
		for (int i = 0; i < quadtangle.points().size(); ++i)
			out << quadtangle.points()[i] << ", "
		
		return out;
	}*/
	
	
	template<typename T_Point_2D>
	Triangle<T_Point_2D>::Triangle()
	: Polygon<T_Point_2D>(3), _cached_area_value(0.0),
		_cached_area_value_is_valid(true), _system(_no_system),
		_system_is_valid(true)
	{
	}
	
	
	template<typename T_Point_2D>
	Triangle<T_Point_2D>::Triangle(const std::vector<T_Point_2D>& points)
	: Polygon<T_Point_2D>(points),_cached_area_value(0.0),
		_cached_area_value_is_valid(false), _system(_no_system),
		_system_is_valid(false)
	{
	}
	
	
	template<typename T_Point_2D>
	void
	Triangle<T_Point_2D>::after_set_point(int index, const T_Point_2D& point)
	{
		_cached_area_value_is_valid = false;
		_system_is_valid = false;
	}
	
	
	template<typename T_Point_2D>
	void
	Triangle<T_Point_2D>::after_set_points(
		const std::vector<T_Point_2D>& points)
	{
		_cached_area_value_is_valid = false;
		_system_is_valid = false;
	}
	
	
	template<typename T_Point_2D>
	double
	Triangle<T_Point_2D>::area_size() const
	{
		// only uses heron formula to calculate the area size!
		if (!_cached_area_value_is_valid)
		{
			_cached_area_value = herons_area_size(
				this->get_point(0).distance(this->get_point(1)),
				this->get_point(1).distance(this->get_point(2)),
				this->get_point(2).distance(this->get_point(0))
			);
			
			_cached_area_value_is_valid = true;
		}
		
		return _cached_area_value;
	}
	
	
	template<typename T_Point_2D>
	bool
	Triangle<T_Point_2D>::contains(const T_Point_2D& point) const
	{
		Triangle<T_Point_2D> test_triangle;
		test_triangle.set_point(0, point);
		
		for (int i = 0; i < 3; ++i)
		{
			test_triangle.set_point(1, this->get_point(i % 3));
			test_triangle.set_point(2, this->get_point((i+1) % 3));
			
			if (this->is_left_system() != test_triangle.is_left_system())
				return false;
		}
		
		return true;
	}
	
	
	template<typename T_Point_2D>
	bool
	Triangle<T_Point_2D>::is_left_system() const
	{
		if (!_system_is_valid)
		{
			double rel_pos = relative_position(this->get_point(0),
				this->get_point(1), this->get_point(2));
			
			if (rel_pos < 0.0)
				_system = _right_system;
			else if (rel_pos > 0.0)
				_system = _left_system;
			else
				_system = _no_system;
			
			_system_is_valid = true;
		}
		
		return (_system == _left_system);
	}
	
	
	template<typename T_Point_2D>
	bool
	Triangle<T_Point_2D>::is_right_system() const
	{
		if (!_system_is_valid)
		{
			double rel_pos = relative_position(this->get_point(0),
				this->get_point(1), this->get_point(2));
			
			if (rel_pos < 0.0)
				_system = _right_system;
			else if (rel_pos > 0.0)
				_system = _left_system;
			else
				_system = _no_system;
			
			_system_is_valid = true;
		}
		
		return (_system == _right_system);
	}
	
} // namespace rangereporting

#endif //RECTANGLE_H

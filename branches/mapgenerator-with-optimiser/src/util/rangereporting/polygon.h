/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#ifndef POLYGON_H
#define POLYGON_H

//#include <ostream>
#include <vector>

namespace rangereporting
{
	
	//-------------------------------------------------------------------------//
	//--- Declaration section -------------------------------------------------//
	//-------------------------------------------------------------------------//
	
	template<typename T_Point_2D>
	class Polygon;
	
	
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
	class Polygon
	{
		
		public:
			
			Polygon(int point_count);
			
			
			Polygon(const std::vector<T_Point_2D>& points);
			
			
			virtual double
			area_size() const = 0;
			
			
			virtual bool
			contains(const T_Point_2D& point) const = 0;
			
			
			inline const T_Point_2D&
			get_point(int index) const;
			
			
			inline const std::vector<T_Point_2D>&
			get_points() const;
			
			
			inline const T_Point_2D&
			point(int index) const;
			
			
			inline const std::vector<T_Point_2D>&
			points() const;
			
			
			inline void
			set_point(int index, const T_Point_2D& point);
			
			
			inline void
			set_points(const std::vector<T_Point_2D>& points);
			
			
			
		protected:
			
			virtual void
			after_set_point(int index, const T_Point_2D& point);
			
			
			virtual void
			after_set_points(const std::vector<T_Point_2D>& points);
			
			
			virtual void
			before_get_point(int index) const;
			
			
			virtual void
			before_get_points() const;
			
			
		private:
			
			std::vector<T_Point_2D> _points;
			
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
	Polygon<T_Point_2D>::Polygon(int point_count)
	: _points(point_count)
	{
	}
	
	
	template<typename T_Point_2D>
	Polygon<T_Point_2D>::Polygon(const std::vector<T_Point_2D>& points)
	: _points(points)
	{
	}
	
	
	template<typename T_Point_2D>
	inline void
	Polygon<T_Point_2D>::after_set_point(int index, const T_Point_2D& point)
	{
	}
	
	
	template<typename T_Point_2D>
	inline void
	Polygon<T_Point_2D>::after_set_points(const std::vector<T_Point_2D>& points)
	{
	}
	
	
	template<typename T_Point_2D>
	inline void
	Polygon<T_Point_2D>::before_get_point(int index) const
	{
	}
	
	
	template<typename T_Point_2D>
	inline void
	Polygon<T_Point_2D>::before_get_points() const
	{
	}
	
	
	template<typename T_Point_2D>
	inline const T_Point_2D&
	Polygon<T_Point_2D>::get_point(int index) const
	{
		before_get_point(index);
		
		return _points[index];
	}
	
	
	template<typename T_Point_2D>
	inline const std::vector<T_Point_2D>&
	Polygon<T_Point_2D>::get_points() const
	{
		before_get_points();
		
		return _points;
	}
	
	
	template<typename T_Point_2D>
	inline const T_Point_2D&
	Polygon<T_Point_2D>::point(int index) const
	{
		return get_point(index);
	}
	
	
	template<typename T_Point_2D>
	inline const std::vector<T_Point_2D>&
	Polygon<T_Point_2D>::points() const
	{
		return get_points();
	}
	
	
	template<typename T_Point_2D>
	inline void
	Polygon<T_Point_2D>::set_point(int index, const T_Point_2D& point)
	{
		_points[index] = point;
		after_set_point(index, point);
	}
	
	
	template<typename T_Point_2D>
	inline void
	Polygon<T_Point_2D>::set_points(const std::vector<T_Point_2D>& points)
	{
		_points = points;
		after_set_points(points);
	}
	
}

#endif //POLYGON_H

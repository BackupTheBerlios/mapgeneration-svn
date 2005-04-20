/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#ifndef POINTND_H
#define POINTND_H

#include <ostream>
#include <vector>

namespace rangereporting
{

	template<int T_Dimensions, typename T_Elem>
	class Point_nD;
	
	
	template<int T_Dimensions, typename T_Elem>
	std::ostream&
	operator<<(std::ostream& out, const Point_nD<T_Dimensions, T_Elem>& point);
	
	
	template<int T_Dimensions, typename T_Elem>
	class Point_nD
	{
		
		friend std::ostream& operator<< <> (std::ostream& out,
			const Point_nD<T_Dimensions, T_Elem>& point);
		
		
		public:
			
			Point_nD();
			
			
			Point_nD(const std::vector<T_Elem>& dimensions);
			
			
			Point_nD(const Point_nD& point);
			
			
			inline const T_Elem&
			operator[](int dimension) const;
			
			
			inline T_Elem&
			operator[](int dimension);
			
			
			inline bool
			operator==(const Point_nD<T_Dimensions, T_Elem>& point) const;
			
			
			inline bool
			operator!=(const Point_nD<T_Dimensions, T_Elem>& point) const;
			
			
		protected:
			
			std::vector<T_Elem> _dimensions;
			
	};
	
	
	template<int T_Dimensions, typename T_Elem>
	std::ostream&
	operator<<(std::ostream& out, const Point_nD<T_Dimensions, T_Elem>& point)
	{
		out << "(";
		for (int i = 0; i < T_Dimensions; ++i)
		{
			out << point._dimensions[i];
			
			if (i < T_Dimensions -1)
				out << ", ";
		}
		
		out << ")";
		return out;
	}
	
	
	template<int T_Dimensions, typename T_Elem>
	Point_nD<T_Dimensions, T_Elem>::Point_nD()
	: _dimensions(T_Dimensions)
	{
	}
	
	
	template<int T_Dimensions, typename T_Elem>
	Point_nD<T_Dimensions, T_Elem>::Point_nD(const std::vector<T_Elem>& dimensions)
	: _dimensions(dimensions)
	{
	}
	
	
	template<int T_Dimensions, typename T_Elem>
	Point_nD<T_Dimensions, T_Elem>::Point_nD(const Point_nD& point)
	: _dimensions(point._dimensions)
	{
	}
	
	
	template<int T_Dimensions, typename T_Elem>
	inline const T_Elem&
	Point_nD<T_Dimensions, T_Elem>::operator[](int index) const
	{
		return _dimensions[index];
	}
	
	
	template<int T_Dimensions, typename T_Elem>
	inline T_Elem&
	Point_nD<T_Dimensions, T_Elem>::operator[](int index)
	{
		return _dimensions[index];
	}
	
	
	template<int T_Dimensions, typename T_Elem>
	inline bool
	Point_nD<T_Dimensions, T_Elem>::operator==(
		const Point_nD<T_Dimensions, T_Elem>& point) const
	{
		for (int i = 0; i < T_Dimensions; ++i)
		{
			if (_dimensions[i] != point._dimensions[i])
				return false;
		}
		
		return true;
	}
	
	
	template<int T_Dimensions, typename T_Elem>
	inline bool
	Point_nD<T_Dimensions, T_Elem>::operator!=(
		const Point_nD<T_Dimensions, T_Elem>& point) const
	{
		return ( !operator==(point) );
	}
	
}

#endif //POINTND_H

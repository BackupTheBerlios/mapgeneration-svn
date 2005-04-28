/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#ifndef SEGMENT_H
#define SEGMENT_H

#include "rangereportingsystem.h"

namespace rangereporting
{
	
	//-------------------------------------------------------------------------//
	//--- Definition section --------------------------------------------------//
	//-------------------------------------------------------------------------//
	
	template<typename T_Point_2D>
	class Segment
	{
		
		friend class Quadtree<T_Point_2D>;
		
		friend std::ostream& operator<< <> (std::ostream& out,
			const Segment<T_Point_2D>& segment);
		
		
		public:
			
			Segment();
			
			
			Segment(const T_Point_2D& point_1, const T_Point_2D& point_2);
			
			
			inline void
			points(T_Point_2D& point_1, T_Point_2D& point_2) const;
			
			
		protected:
			
			T_Point_2D _point_1;
			
			
			T_Point_2D _point_2;
	};
	
	
	//-------------------------------------------------------------------------//
	//--- Implementation section ----------------------------------------------//
	//-------------------------------------------------------------------------//
	
	template<typename T_Point_2D>
	std::ostream&
	operator<<(std::ostream& out, const Segment<T_Point_2D>& segment)
	{
		out << "Segment:" << std::endl
			<< "\t" << segment._point_1 << " - " << segment._point_2;
		
		return out;
	}
	
	
	template<typename T_Point_2D>
	Segment<T_Point_2D>::Segment()
	: _point_1(), _point_2()
	{
	}
	
	
	template<typename T_Point_2D>
	Segment<T_Point_2D>::Segment(const T_Point_2D& point_1,
		const T_Point_2D& point_2)
	: _point_1(point_1), _point_2(point_2)
	{
	}
	
	
	template<typename T_Point_2D>
	inline void
	Segment<T_Point_2D>::points(T_Point_2D& point_1, T_Point_2D& point_2) const
	{
		point_1 = _point_1;
		point_2 = _point_2;
	}
	
}

#endif //SEGMENT_H

/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#ifndef SEGMENT_H
#define SEGMENT_H

namespace rangereporting
{
	
	template<typename T_Point>
	class Segment;
	
	
//	template<typename T_Point>
//	std::ostream& operator<<(std::ostream& out,
//		const Segment<T_Point>& segment);
	
}

#include "polygon.h"

namespace rangereporting
{
	
	//-------------------------------------------------------------------------//
	//--- Definition section --------------------------------------------------//
	//-------------------------------------------------------------------------//
	
	template<typename T_Point>
	class Segment : public Polygon<T_Point>
	{
		
//		friend std::ostream& operator<< <> (std::ostream& out,
//			const Segment<T_Point>& segment);
		
		
		public:
			
			Segment();
			
			
			Segment(const std::vector<T_Point>& points);
			
			
			inline double
			area_size() const;
			
			
			inline bool
			contains(const T_Point& point) const;
			
	};
	
	
	//-------------------------------------------------------------------------//
	//--- Implementation section ----------------------------------------------//
	//-------------------------------------------------------------------------//
	
/*	template<typename T_Point>
	std::ostream&
	operator<<(std::ostream& out, const Segment<T_Point>& segment)
	{
		out << "Segment: "
			<< segment._point_1 << " - " << segment._point_2;
		
		return out;
	}*/
	
	
	template<typename T_Point>
	Segment<T_Point>::Segment()
	: Polygon<T_Point>(2)
	{
	}
	
	
	template<typename T_Point>
	Segment<T_Point>::Segment(const std::vector<T_Point>& points)
	: Polygon<T_Point>(points)
	{
	}
	
	
	template<typename T_Point>
	double
	Segment<T_Point>::area_size() const
	{
		return 0.0;
	}
	
	
	template<typename T_Point>
	bool
	Segment<T_Point>::contains(const T_Point& point) const
	{
		if (relative_position(this->get_point(0), this->get_point(1), point)
			!= 0.0)
		{
			return false;
		}
		
		try
		{
			int i = 0;
		
			// may raise a segmentation fault...
			while (this->get_point(0)[i] == this->get_point(1)[i])
				++i;
			// ... which is catched below.
			
			double min = this->get_point(0)[i];
			double max = this->get_point(1)[i];
			
			if (min > max)
			{
				double temp = min;
				min = max;
				max = temp;
			}
			
			return ( (min <= point[i]) && (point[i] <= max) );
			
		} catch(...)
		{
			// get_point(0) == get_point(1)
			return true;
		}
	}
}

#endif //SEGMENT_H

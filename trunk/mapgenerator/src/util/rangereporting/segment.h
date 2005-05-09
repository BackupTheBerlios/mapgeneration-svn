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
	
	
	template<typename T_Point>
	std::ostream& operator<<(std::ostream& out,
		const Segment<T_Point>& segment);
	
	
	//-------------------------------------------------------------------------//
	//--- Definition section --------------------------------------------------//
	//-------------------------------------------------------------------------//
	
	template<typename T_Point>
	class Segment
	{
		
		friend std::ostream& operator<< <> (std::ostream& out,
			const Segment<T_Point>& segment);
		
		
		public:
			
			inline
			Segment();
			
			
			inline std::pair<T_Point, T_Point>
			get_points() const;
			
			
			inline void
			set_points(const T_Point& in_point_1, const T_Point& in_point_1);
			
			
		protected:
			
			T_Point _point_1;
			
			
			T_Point _point_2;
	};
	
	
	//-------------------------------------------------------------------------//
	//--- Implementation section ----------------------------------------------//
	//-------------------------------------------------------------------------//
	
	template<typename T_Point>
	std::ostream&
	operator<<(std::ostream& out, const Segment<T_Point>& segment)
	{
		out << "Segment: "
			<< segment._point_1 << " - " << segment._point_2;
		
		return out;
	}
	
	
	template<typename T_Point>
	inline
	Segment<T_Point>::Segment()
	: _point_1(), _point_2()
	{
	}
	
	
	template<typename T_Point>
	inline std::pair<T_Point, T_Point>
	Segment<T_Point>::get_points() const
	{
		return std::make_pair(_point_1, _point_2);
	}
	
	
	template<typename T_Point>
	inline void
	Segment<T_Point>::set_points(const T_Point& point_1, const T_Point& point_2)
	{
		point_1 = _point_1;
		point_2 = _point_2;
	}
	
}

#endif //SEGMENT_H

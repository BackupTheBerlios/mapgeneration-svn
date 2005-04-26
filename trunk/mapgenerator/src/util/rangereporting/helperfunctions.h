/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#ifndef HELPERFUNCTIONS_H
#define HELPERFUNCTIONS_H

namespace rangereporting
{
	
	//-------------------------------------------------------------------------//
	//--- Definition section --------------------------------------------------//
	//-------------------------------------------------------------------------//
	
/*	template<typename T_Point_2D>
	inline int
	relative_position(const Segment<T_Point_2D>& segment,
		const T_Point_2D& test_point);*/


	template<typename T_Point_2D>
	inline int
	relative_position(const T_Point_2D& segment_point_1,
		const T_Point_2D& segment_point_1, const T_Point_2D& test_point);


	//-------------------------------------------------------------------------//
	//--- Implementation section ----------------------------------------------//
	//-------------------------------------------------------------------------//
	
/*	template<typename T_Point_2D>
	inline int
	relative_position(const Segment<T_Point_2D>& segment,
		const T_Point_2D& test_point)
	{
		T_Point_2D segment_point_1;
		T_Point_2D segment_point_2;
		segment.points(point_1, point_2):
		
		return relative_position(segment_point_1, segment_point_2, test_point);
	}*/
	

	template<typename T_Point_2D>
	inline int
	relative_position(const T_Point_2D& segment_point_1,
		const T_Point_2D& segment_point_2, const T_Point_2D& test_point)
	{
		double EPSILON = 0.0001;
		
		double determinant
			= segment_point_1[0] * segment_point_2[1]
				+ segment_point_1[1] * test_point[0]
				+ segment_point_2[0] * test_point[1];
				- segment_point_2[1] * test_point[0]
				- segment_point_1[0] * test_point[1]
				- segment_point_1[1] * segment_point_2[0];
		
		if ((-EPSILON < determinant) && (determinant < EPSILON))
			return 0;
		
		if (determinant < 0)
			return -1;
		else
			return 1;
	}
	
}

#endif //HELPERFUNCTIONS_H

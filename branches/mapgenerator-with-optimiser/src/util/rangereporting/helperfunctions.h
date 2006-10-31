/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#ifndef HELPERFUNCTIONS_H
#define HELPERFUNCTIONS_H

#include <cmath>
//#include <algorithm>
#include <functional>
//#include <vector>

//#include "util/constants.h"
//#include "util/geocoordinate.h"

namespace rangereporting
{
	
	//-------------------------------------------------------------------------//
	//--- Definition section --------------------------------------------------//
	//-------------------------------------------------------------------------//
	
/*	template<typename T_Point_2D>
	inline int
	relative_position(const Segment<T_Point_2D>& segment,
		const T_Point_2D& test_point);*/
	
	inline double
	herons_area_size(const double segment_length_1,
		const double segment_length_2, const double segment_length_3);
	
	
	template<typename T_Point_2D>
	inline double
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
	
	inline double
	herons_area_size(const double segment_length_1,
		const double segment_length_2, const double segment_length_3)
	{
		// I use the numerical more stable variant
		
		std::vector<double> sort_vector(3);
		sort_vector[0] = segment_length_1;
		sort_vector[1] = segment_length_2;
		sort_vector[2] = segment_length_3;
		
		std::sort(sort_vector.begin(), sort_vector.end(), std::greater<double>());
		double a = sort_vector[0];
		double b = sort_vector[1];
		double c = sort_vector[2];
		
		assert( (a >= b) && (b >= c) );
//		assert(a <= (b+c)); //for approximated distances this assertion may not hold!

//		if (a > (b+c) )
//		{
//			std::cout << a << ", " << b << ", " << c << std::endl;
//			assert(a <= (b+c)); //for approximated distances this assertion may not hold!
//		}
		
		double S = 0.25 * sqrt(
			(a+(b+c)) * (c-(a-b)) * (c+(a-b)) * (a+(b-c)) );
		
//		if (S > 100.0)
//		{
//			std::cout << a << ", " << b << ", " << c << ": S=" << S << std::endl;
//		}
			
		if (isnan(S))
			return 0.0;
		
		return S;
	}
	
	
	template<typename T_Point_2D>
	inline double
	relative_position(const T_Point_2D& segment_point_1,
		const T_Point_2D& segment_point_2, const T_Point_2D& test_point)
	{
		// We calculate the following determinant
		//       | 1 Ax Ay |
		// det = | 1 Bx By |
		//       | 1 Cx Cy |
		//
		// with A being the test_point and B->C the segment.
		// If A lays right of B->C we have a "right system" and det is less
		// than 0, else we have a "left system" and det is greater than 0.
		// The special case: det = 0 occurs if A, B, C are co-linear.
		//
		// Using the formula  |0.5 * det|, we can also calculate the
		// area size of the triangle A, B, C.
//		double EPSILON = 0.0001;
		
		double determinant
			= 1.0 * segment_point_1[0] * segment_point_2[1] // + 1 * Bx * Cy (1)
			+ test_point[0] * segment_point_1[1] * 1.0      // + Ax * By * 1 (2)
			+ test_point[1] * 1.0 *segment_point_2[0]       // + Ay * 1 * Cx (3)
			- test_point[1] * segment_point_1[0] * 1.0      // - Ay * Bx * 1 (4)
			- segment_point_1[1] * segment_point_2[0] * 1.0 // - By * Cx * 1 (5)
			- segment_point_2[1] * test_point[0] * 1.0;     // - Cy * Ax * 1 (6)
		
		return determinant;
		
/*		if ((-EPSILON < determinant) && (determinant < EPSILON))
			return 0;
		
		if (determinant < 0)
			return -1;
		else
			return 1;*/
	}
	
}

#endif //HELPERFUNCTIONS_H

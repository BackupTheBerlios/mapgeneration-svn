/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#ifndef RANGEREPORTINGSYSTEM_H
#define RANGEREPORTINGSYSTEM_H

namespace rangereporting
{
	
	//-------------------------------------------------------------------------//
	//--- Definition section --------------------------------------------------//
	//-------------------------------------------------------------------------//
	
	template<typename T_Point_2D>
	class Quadtree;
	
	
	template<typename T_Point_2D>
	class Rectangle;
	
	
	template<typename T_Point_2D>
	class Segment;
	
	
	template<typename T_Point_2D>
	class Trapezoid;
	
	
	template<typename T_Point_2D>
	std::ostream&
	operator<<(std::ostream& out, const Quadtree<T_Point_2D>& segment);
	
	
	template<typename T_Point_2D>
	std::ostream&
	operator<<(std::ostream& out, const Rectangle<T_Point_2D>& segment);
	
	
	template<typename T_Point_2D>
	std::ostream&
	operator<<(std::ostream& out, const Segment<T_Point_2D>& segment);
	
	
	template<typename T_Point_2D>
	std::ostream&
	operator<<(std::ostream& out, const Trapezoid<T_Point_2D>& segment);
	
}

#endif //RANGEREPORTINGSYSTEM_H

/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#include <cstdlib>
#include <exception>
#include <iostream>
#include <list>
//#include "stdint.h"

#include "geocoordinate.h"
#include "util/rangereporting/quadtree.h"

using namespace std;
using namespace rangereporting;
using namespace mapgeneration;


int main(int argc, char* argv[])
{
	GeoCoordinate lower_left;
	lower_left[0] = 0.0;
	lower_left[1] = 0.0;

	GeoCoordinate upper_right;
	upper_right[0] = 1.0;
	upper_right[1] = 1.0;
	
	Rectangle<GeoCoordinate> rectangle;
	rectangle.set_corners(lower_left, upper_right);
	
	std::cout << "Set span rectangle." << std::endl;
	
	int nodes = 10;
	try
	{
		if (argc > 1)
			nodes = atoi(argv[1]);
	} catch (std::exception e)
	{};
	
	std::cout << "Read #numbers: " << nodes << std::endl;
	
	Quadtree<GeoCoordinate> quadtree;
	quadtree.set_span_rectangle(rectangle);
	quadtree.set_max_depth(3);
	srand(0);
	
	std::cout << "Inited Quadtree" << std::endl;
	
	
	for (int i = 0; i < nodes; ++i)
	{
		GeoCoordinate a;
		a[0] = (static_cast<double>(rand()) / static_cast<double>(RAND_MAX));
		a[1] = (static_cast<double>(rand()) / static_cast<double>(RAND_MAX));
		quadtree.add_point(a);
	}
	
	for (int i = 0; i < nodes; i += 1)
		quadtree.move_point(i, GeoCoordinate(0.1, 0.1));
	
	quadtree.move_point(0, GeoCoordinate(0.5, 0.5));
	
	std::cout << quadtree << std::endl;
	
	std::vector<uint32_t> results;
	Rectangle<GeoCoordinate> qr;
	qr.set_corners(GeoCoordinate(0.4, 0.4), GeoCoordinate(0.6, 0.6));
	quadtree.range_query(qr, results);
	
	std::vector<uint32_t>::const_iterator iter = results.begin();
	std::vector<uint32_t>::const_iterator iter_end = results.end();
	for (; iter != iter_end; ++iter)
	{
		std::cout << *iter << ", ";
	}
	std::cout << std::endl;
/*	for (int i = 0; i < nodes; ++i)
	{
		GeoCoordinate a;
		a[0] = (static_cast<double>(rand()) / static_cast<double>(RAND_MAX));
		a[1] = (static_cast<double>(rand()) / static_cast<double>(RAND_MAX));
		quadtree.add_point(a);
	}
	std::cout << "Added nodes" << std::endl;
	
	for (int i = 100; i < 500; i += 7)
		quadtree.remove_point(i);
	
	
	for (int i = 0; i < 10000; ++i)
	{
		GeoCoordinate a;
		a[0] = (static_cast<double>(rand()) / static_cast<double>(RAND_MAX));
		a[1] = (static_cast<double>(rand()) / static_cast<double>(RAND_MAX));
		
		double some_random_value
			= static_cast<double>(rand()) / static_cast<double>(RAND_MAX);
		double sign = (some_random_value < 0.5 ? -1.0 : 1.0);
		
		GeoCoordinate b;
		b[0] = a[0] + (sign * 0.05);
		b[1] = a[1] + (sign * 0.0125);
		
		GeoCoordinate c;
		c[0] = a[0] - (sign * 0.025);
		c[1] = a[1] + (sign * 0.025);
	
		GeoCoordinate d;
		d[0] = a[0] + (sign * 0.025);
		d[1] = a[1] + (sign * 0.0375);
		
		Trapezoid<GeoCoordinate> query_trapezoid(a, b, c, d);
		
		std::vector< Quadtree<GeoCoordinate>::D_IndexType > result;
		quadtree.range_query(query_trapezoid, result);
		
//		std::cout << result.size() << "  ";
	}
	std::cout << "Made 10000 queries." << std::endl;
	
	for (int i = 0 ; i < 1000; ++i)
	{
		GeoCoordinate a;
		a[0] = (static_cast<double>(rand()) / static_cast<double>(RAND_MAX));
		a[1] = (static_cast<double>(rand()) / static_cast<double>(RAND_MAX));
		
		double some_random_value
			= static_cast<double>(rand()) / static_cast<double>(RAND_MAX);
		double sign = (some_random_value < 0.5 ? -1.0 : 1.0);
		
		GeoCoordinate b;
		b[0] = a[0] + (sign * 0.01);
		b[1] = a[1] + (sign * 0.01);
		
		Rectangle<GeoCoordinate> query_rectangle;
		query_rectangle.set_corners(a, b);
		
		std::vector< Quadtree<GeoCoordinate>::D_IndexType > result;
		quadtree.range_query(query_rectangle, result);
		
		if (result.size() > 0)
		{
			GeoCoordinate c;
			some_random_value
				= static_cast<double>(rand()) / static_cast<double>(RAND_MAX);
			if (some_random_value < 0.5 )
			{
				c[0] = a[0] + 0.001;
				c[1] = a[1] + 0.001;
			} else
			{
				c[0] = a[0] + 0.1;
				c[1] = a[1] + 0.1;
			}
				
			bool move_result = quadtree.move_point(result.front(), c);
			if (!move_result)
				std::cout << "Out of range: " << c << std::endl;
		}
	}
	
	std::cout << std::endl;
*/	
	return 0;
}

